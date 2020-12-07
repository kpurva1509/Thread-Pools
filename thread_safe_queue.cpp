#include "thread_safe_queue.hpp"

/* Public method implementations */

template<typename T>
thread_safe_queue<T>::thread_safe_queue(){
    head = new node;
    tail = head.get();
}

template<typename T>
std::shared_ptr<T> thread_safe_queue<T>::try_pop() {
    std::unique_ptr<node> old_head=try_pop_head();
    return old_head?old_head->data:std::shared_ptr<T>();
}

template<typename T>
bool thread_safe_queue<T>::try_pop(T& _value) {
    std::unique_ptr<node> const old_head=try_pop_head(_value);
    return old_head;
}

template<typename T>
std::shared_ptr<T> thread_safe_queue<T>::wait_and_pop() {
    std::unique_ptr<node> const old_head = wait_pop_head();
    return old_head->data;
}

template<typename T>
void thread_safe_queue<T>::wait_and_pop(T& _value) {
    std::unique_ptr<node> const old_head = wait_pop_head(_value);
}

template<typename T>
void thread_safe_queue<T>::push(const T& _data) {
    std::shared_ptr<T> new_data(std::make_shared<T>(std::move(_data)));
    std::unique_ptr<node> p(new node); /* new dummy node for tail */
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
    data_condition.notify_all();
}

template<typename T>
void thread_safe_queue<T>::empty() {
    std::lock_guard<std::mutex> head_lock(head_mutex);
    return (head.get()==get_tail());
}