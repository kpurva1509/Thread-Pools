#include "thread_pool.hpp"

// method implementations

/* private method implementation */

void thread_pool::worker_thread() {
    /* will iterate till done is true */
    while(!done) {
        function_wrapper task;
        if(work_queue.try_pop(task)){
            task();
        } else {
            std::this_thread::yield();
        }
    }
}

/* public method implementation */

thread_pool::thread_pool() {

    done = false;
    joiner{threads};

    /* query the hardware_concurrency, spawn worker_threads */
    /* worker_threads will pick up tasks from the work_queue */
    const int max_threads = std::thread::hardware_concurrency();
    try {
        for(int i=0; i < max_threads; i++) {
            threads.push_back(std::thread(&thread_pool::worker_thread), this);
        }      
    } catch(...){
        done = true;
        throw;
    }
}

thread_pool::~thread_pool() {
    done = true;
}

/* This is a naive implementation of the submit function
 * It does not return the status of the task passed to the work_queue
 * Once the function is submitted to the work_queue, the worker_threads
 * pick it up
 * We need to record the outcome of the function implementation to
 * ensure it was executed successfully
 */
template<typename Functiontype> 
void thread_pool::submit_naive(Functiontype& func) {
    /* submit task functions to the queue */
    work_queue.push(std::function<void()>(func));
}

/* Pass packaged_tasks to the work_queue instead of raw function pointer
 * These packaged_tasks are threads launched asynchronously
 * These threads can be MOVEd from the task to the worker_thread
 * Passing packaged_tasks can help us wait on the spawned threads
 * WE can get the future associated with the task and ensure all
 * tasks are completed sucessfully 
 */
template<typename Functiontype>
std::future<typename std::result_of<Functiontype()>::type> submit(Functiontype func) {
    /* Capture return datatype of the task function 
    * std::result_of<Functiontype()>::type 
    */

    typedef typename std::result_of<Functiontype()>::type result_type;
    std::packaged_task<result_type()> task(std::move(func));
    std::future<result_type> res(task.get_future());

    /* work_queue to be refactored as it no longer supports pointer to functions
     * work_queue to accept packaged_task MOVE instead 
     */
    work_queue.push(std::move(task));
    return res;
}

/* allow worker_threads to complete pending tasks to avoid deadlock 
 * multiple independant threads can therefore run in parallel
 */
void thread_pool::run_pending_tasks() {

    function_wrapper task;
    if(work_queue.try_pop(task)) {
        task();
    } else {
        std::this_thread::yield();
    }
}

/* Implementation of contention-free member methods */
void thread_pool::worker_thread_no_contention() {
    local_work_queue.reset(new local_queue_type);
    while(!done){
        run_pending_tasks_no_contention();
    }
}

template<typename Functiontype>
void thread_pool::submit_no_contention(Functiontype func) {
    typedef typename std::result_of<Functiontype()>::type result_type;
    std::packaged_task<result_type()> task(f);
    std::future<result_type> res(task.get_future());
    if(local_work_queue)
    {
        local_work_queue->push(std::move(task));
    }
    else {
        pool_work_queue.push(std::move(task));
    }
    return res;
}

void thread_pool::run_pending_tasks_no_contention() {
    function_wrapper task;
    if(local_work_queue && !local_work_queue->empty()){
        task=std::move(local_work_queue->front());
        local_work_queue->pop();
        task();
    }
    else if(pool_work_queue.try_pop(task)){ 
        task(); 
    }
    else {
        std::this_thread::yield();
    }
}