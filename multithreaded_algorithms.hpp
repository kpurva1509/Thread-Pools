#include "thread_pool.hpp"
#include <list>

// 1. Parallel Accumulate
// 2. Matrix Multiplication
// 3. Quick Sort
// 4. Parallel for_each

/* 1. Parallel Accumulate using thread pool - implementation */

template<typename iterator, typename T>
T accumulate_block(iterator start, iterator end){
    T result = std::accumulate(start, end,0);
    return result;
}

template<typename iterator, typename T>
T parallel_accumulate(iterator start, iterator end, T& init){
    /* We do not query the hardware concurrency of the machine
     * we just submit tasks to the thread pool to be executed
    */

   unsigned long const length = std::distance(start, end);
   unsigned int block_size = 25;
   unsigned long const block_count = (length + block_size -1)/block_size;
   std::vector<std::future<T> > futures(block_count);
   thread_pool pool;

    iterator block_start = start;
    for(int i = 0; i < block_count - 1; i++) {
        iterator block_end = block_start;
        std::advance(block_end, block_size);
        /* Submit function submits a function pointer, and
         * pushes a corresponding packaged_task into the queue */
        futures[i] = pool.submit(accumulate_block<iterator, T>(block_start, block_end));
        // futures[i] = pool.submit(std::bind(accumulate_block<iterator,T>, block_start, block_end));
        block_start = block_end;
    }

    T last_result = accumulate_block<iterator, T>(block_start, end);
    T result = init;

    for(int i = 0; i < block_count - 1; i++) {
        result += futures[i].get();
    }

    result += last_result;

    return result;
}

/* 2. Strassen's Matrix Multiplication using thread pool - implementation */


/* 3. Parallel Quick sort using thread pool - implementation */

/* sorted class is a wrapper around the thread pool 
 * performs the heavy lifting of sorting the input array 
 */
template<typename T> 
struct sorter {
    thread_pool pool;
    std::list<T> do_sort(std::list<T>& chunk_of_data) {
        
        /* preliminary check before recursive call */
        if(chunk_of_data.empty()){
            return chunk_of_data;
        }

        /* choosing a pivot element for recursive call 
         * pick the first element of the original list,
         * copy it to the result list, and set it as pivot 
         */
        std::list<T> result;
        result.splice(result.begin(), chunk_of_data, chunk_of_data.begin());
        const T& pivot_value = *(result.begin());

        /* pivot_iterator is basically iterator to std::next(chunk_of_data.bein()) */
        typename std::list<T>::iterator pivot_iterator = 
                std::partition(chunk_of_data.begin(), chunk_of_data.end(),
                [&](const T& value){
                    return value < pivot_value;
                }
        );

        /* lower chunk */
        std::list<T> new_lower_dummy;
        new_lower_dummy.splice(new_lower_dummy.end(), chunk_of_data, chunk_of_data.begin(),
                pivot_iterator);
        std::future<std::list<T> > new_lower_chunk =  
                pool.submit(std::bind(&sorter::do_sort, this, std::move(new_lower_dummy)));

        /* higher chunk */
        std::list<T> new_higher_dummy(do_sort(chunk_of_data));
        result.splice(result.end(), new_higher_dummy);
        while(new_lower_chunk.wait_for(
                std::chrono::seconds(0)) == std::future_status::timeout) {
            pool.run_pending_tasks();
        }        

        result.splice(result.begin(), new_lower_chunk.get());
        return result;
    }
};

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input){
    if(input.empty()) return input;
    sorter<T> sort;
    return sort.do_sort(input);
}

/* 4. Parallel for_each using thread pool - implementation */

