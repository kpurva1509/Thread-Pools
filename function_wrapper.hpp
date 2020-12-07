#include "headers.hpp"

/* This class will take a packaged_task and
 * pass it to the work_queue
 * shall have an overloaded function call operator
 * on function invocation, will call the responsible
 * function object for implementation
 * as it receives a packaged_task, only move 
 * semantics will be permissible 
 */

class function_wrapper {

    /* Abstract base; inherited classes can 
     * implement functions with varying
     * argument types and return types
     */
    struct impl_base{
        virtual void call() = 0;
        virtual ~impl_base(){}
    };

    std::unique_ptr<impl_base> impl;

    template<typename F>
    struct impl_type : impl_base {
        F f;
        impl_type(F&& _f): f(std::move(_f)){}
        void call(){ f(); }
    };

public:

    /* implement R-value reference constructors */
    function_wrapper() = default;

    template<typename F>
    function_wrapper(F&& f):impl(new impl_type<F>(std::move(f))){}

    void operator()(){ impl->call(); }

    function_wrapper(function_wrapper&& rhs): impl(std::move(rhs.impl)){}

    function_wrapper& operator=(function_wrapper&& rhs){
        impl = std::move(rhs.impl);
        return *this;
    }

    /* delete L-value reference constructors */
    function_wrapper(function_wrapper&) = delete;
    function_wrapper(const function_wrapper& rhs) = delete;
    function_wrapper& operator=(const function_wrapper& rhs) = delete;
};