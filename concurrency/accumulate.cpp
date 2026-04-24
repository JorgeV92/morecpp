#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <string>
#include <thread>

template<typename Iterator, typename T>
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T& result) {
        result = std::accumulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);
    if (!length)
        return init;

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(
        hardware_threads != 0 ? hardware_threads : 2,
        max_threads
    );
    unsigned long const block_size = length / num_threads;

    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);

    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); i++) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(
            accumulate_block<Iterator, T>(),
            block_start, block_end, std::ref(results[i])
        );
        block_start = block_end;
    }
    accumulate_block<Iterator, T>() (
        block_start, last, results[num_threads - 1]
    );

    std::for_each(
        threads.begin(),
        threads.end(),
        std::mem_fn(&std::thread::join)
    );

    return std::accumulate(results.begin(), results.end(), init);
}

template<typename T>
void print_example(const std::string& label, const std::vector<T>& values, T init) {
    const T parallel_total = parallel_accumulate(values.begin(), values.end(), init);
    const T sequential_total = std::accumulate(values.begin(), values.end(), init);

    std::cout << label << '\n'
              << "parallel_accumulate: " << parallel_total << '\n'
              << "std::accumulate:     " << sequential_total << "\n\n";
}

int main() {
    std::vector<int> numbers(100);
    std::iota(numbers.begin(), numbers.end(), 1);

    // With 100 values and min_per_thread set to 25, the work can be split
    // into up to 4 blocks, limited by the number of hardware threads.
    print_example("Example 1: sum 1..100 with init = 0", numbers, 0);
    print_example("Example 2: sum 1..100 with init = 100", numbers, 100);

    std::vector<std::string> words{"parallel", " ", "accumulate", " ", "works"};
    print_example(
        "Example 3: concatenate strings with init = \"\"",
        words,
        std::string{}
    );

    return 0;
}
