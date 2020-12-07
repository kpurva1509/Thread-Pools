#include "work_stealing_thread_pool.hpp"

/* Private function implementations */

void work_stealing_thread_pool::worker_thread(unsigned _my_index) {
    my_index = _my_index;
    thread_local_queue = pthread_local_queue[my_index].get();
    while(!done){
        run_pending_tasks();
    }
}

bool work_stealing_thread_pool::pop_self_local(data_type& res) {
    return thread_local_queue && thread_local_queue->try_pop(res);
}

bool work_stealing_thread_pool::pop_other_local(data_type& res) {

    for(unsigned i = 0; i < pthread_local_queue.size(); i++) {
         unsigned const index=(my_index+i+1)%pthread_local_queue.size();
            if(pthread_local_queue[index]->try_steal(task))
            {
                return true;
            }
    }
    return false;
}

bool work_stealing_thread_pool::pop_task_global(data_type& res) {
    return pool_work_queue.try_pop(res);
}


/* Public function implementations */

work_stealing_thread_pool::work_stealing_thread_pool(): done(false), joiner(threads) {

    unsigned const max_threads = std::thread::hardware_concurrency();
    
    try{ 
        for(int i=0; i<max_threads; i++){
            pthread_local_queue.push_back(
                            std::unique_ptr<work_stealing_queue>(new work_stealing_queue));
            threads.push_back(std::thread(
                            &work_stealing_thread_pool::worker_thread, this, i));
        }

    } catch(...) {
        done = true;
        throw;
    }
    
}

template<typename Functiontype>
std::future<typename std::result_of<Functiontype()>::type> work_stealing_thread_pool::submit(Functiontype func) {

    typedef typename std::result_of<Functiontype()>::type result_type;
    std::packaged_task<result_type()> task(func);
    if(thread_local_queue) {
        thread_local_queue->push(std::move(task));
    } else {
        pool_work_queue.push(std::move(task));
    } 
}

void work_stealing_thread_pool::run_pending_tasks() {

    data_type task;
    if(pop_self_local(task) ||
       pop_other_local(task) ||
       pop_task_global(task)) {
           task();
    } else {
        std::this_thread::yield();
    }
}

work_stealing_thread_pool::~work_stealing_thread_pool() {
    done = true;
}