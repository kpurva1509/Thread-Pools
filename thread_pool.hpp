#include "thread_safe_queue.hpp"
#include "join_threads.hpp"
#include "function_wrapper.hpp"

class thread_pool {

    /* boolean variable to indicate the lifetime of the thread pool */
    std::atomic_bool done;

    /* queue with pointer to fuctions submitted to the pool */
    thread_safe_queue<function_wrapper> work_queue;

    /* vector to store threads in the thread pool */
    std::vector<std::thread> threads;

    /* joiner_threads object to join the threads prior to return */
    join_threads joiner;

    /* enable worker_thread to pick up tasks from work_queue */
    void worker_thread();

    /* parameters to enable thread_pool without contention */
    thread_safe_queue<function_wrapper> global_work_queue;
    typedef std::queue<function_wrapper> local_queue_type;
    static thread_local std::unique_ptr<local_queue_type> local_work_queue;
    void worker_thread_no_contention();

public:

    thread_pool();
    ~thread_pool();

    template<typename Functiontype> 
    void submit_naive(Functiontype func);

    template<typename Functiontype>
    std::future<typename std::result_of<Functiontype()>::type> submit(Functiontype func);

    void run_pending_tasks();

    template<typename Functiontype>
    std::future<typename std::result_of<Functiontype()>::type> submit_no_contention(Functiontype func);

    void run_pending_tasks_no_contention();
};
