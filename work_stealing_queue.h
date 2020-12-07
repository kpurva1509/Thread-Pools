#include "headers.hpp"
#include "function_wrapper.hpp"

class work_stealing_queue{

    /* wrapper around packaged tasks */
    typedef function_wrapper data_type;

    /* using double ended queue to avoid contention on the data */
    std::deque<data_type> the_queue;

    /* mutex to provide thread safe access to functions */
    mutable std::mutex the_mutex;

public:

    work_stealing_queue() = default;
    work_stealing_queue(const work_stealing_queue& rhs) = delete;
    work_stealing_queue& operator=(const work_stealing_queue& rhs) = delete;

    void push(data_type data);
    bool empty();
    bool try_pop(data_type& res);
    bool try_steal(data_type& res);

};