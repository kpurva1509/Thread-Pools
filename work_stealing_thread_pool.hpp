#include "headers.hpp"
#include "function_wrapper.hpp"
#include "join_threads.hpp"
#include "thread_safe_queue.hpp"
#include "work_stealing_queue.h"

class work_stealing_thread_pool {

    std::atomic_bool done;
    typedef function_wrapper data_type;
    thread_safe_queue<data_type> pool_work_queue;
    std::vector<std::unique_ptr<work_stealing_queue> > pthread_local_queue;
    std::vector<std::thread> threads;
    join_threads joiner;

    static thread_local work_stealing_queue* thread_local_queue;
    static thread_local unsigned my_index;

    void worker_thread(unsigned _my_index);
    bool pop_self_local(data_type& res);
    bool pop_other_local(data_type& res);
    bool pop_task_global(data_type& res);

public:

    work_stealing_thread_pool();

    template<typename Functiontype>
    std::future<typename std::result_of<Functiontype()>::type> submit(Functiontype func);

    void run_pending_tasks();

    ~work_stealing_thread_pool();
};