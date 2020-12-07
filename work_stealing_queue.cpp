#include "work_stealing_queue.h"

/* function implementations */

void work_stealing_queue::push(data_type data) {

    std::lock_guard<std::mutex> lg(the_mutex);
    
    /* move semantics as the queue is a function wrapper is a wrapper around
     * packaged tasks, which are only movable and not copyable 
     */
    the_queue.push_front(std::move(data));
}

bool work_stealing_queue::empty() {
    std::lock_guard<std::mutex> lg(the_mutex);
    return the_queue.empty();
}

bool work_stealing_queue::try_pop(data_type& res) {
    std::lock_guard<std::mutex> lg(the_mutex);
    if(empty()){
        return false;
    }
    res = std::move(the_queue.front());
    the_queue.pop_front();
    return true;
}

bool work_stealing_queue::try_steal(data_type& res) {
    std::lock_guard<std::mutex> lg(the_mutex);
    if(empty()) {
        return false;
    }
    res = std::move(the_queue.back());
    the_queue.pop_back();
    return true;
}