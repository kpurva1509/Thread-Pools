#include "headers.hpp"

template<typename T>
class thread_safe_queue {
    
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::unique_ptr<node> head;
    node* tail;
    std::mutex head_mutex;
    std::mutex tail_mutex;
    std::condition_variable data_condition;

    /* Copy C'tor and assign private - deleted */
    thread_safe_queue(const thread_safe_queue& rhs) {}
    thread_safe_queue& operator=(const thread_safe_queue& rhs) {}

    /* Private helpers */
    node* get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    std::unique_ptr<node> pop_head() {
        std::unique_ptr<node> old_head = std::move(head);
        head=std::move(old_head->next);
        return old_head;
    }

    std::unique_lock<std::mutex> wait_for_data(){
        std::unique_lock<std::mutex> head_lock(head_mutex);
        data_condition.wait(head_lock,[&](){return head.get() != get_tail();});
        return std::move(head_lock);
    }

    std::unique_ptr<node> wait_pop_head() {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        return pop_head();
    }

    std::unique_ptr<node> wait_pop_head(T& value)
    {
        std::unique_lock<std::mutex> head_lock(wait_for_data());
        value=std::move(*head->data);
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head() {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get()==get_tail())
        {
            return std::unique_ptr<node>();
        }
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head(T& value) {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if(head.get()==get_tail())
        {
            return std::unique_ptr<node>();
        }
        value=std::move(*head->data);
        return pop_head();
    }


public:

    thread_safe_queue();
    // thread_safe_queue(const thread_safe_queue& rhs) = delete;
    // thread_safe_queue& operator=(const thread_safe_queue& rhs) = delete;

    std::shared_ptr<T> try_pop();
    bool try_pop(T&);
    std::shared_ptr<T> wait_and_pop();
    void wait_and_pop(T&);
    void push(const T&);
    void empty();

};