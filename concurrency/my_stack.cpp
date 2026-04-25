#include <iostream>
#include <vector>
#include <algorithm>
#include <deque>
#include <exception>
#include <mutex>
#include <memory>
#include <numeric>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

template<typename T, typename Container = std::deque<T>> 
class stack {
public:
    explicit stack(const Container&);
    explicit stack(Container&& = Container());
    template<class Alloc> explicit stack(const Alloc&);
    template<class Alloc> stack(const Container&, const Alloc&);
    template<class Alloc> stack(Container&&, const Alloc&);
    template<class Alloc> stack(stack&&, const Alloc&);

    bool empty() const;
    size_t size() const;
    T& top();
    T const& top() const;
    void push(T const&);
    void push(T&&); 
    void pop();
    void swap(stack&&);
};

struct empty_stack : std::exception {
    const char* what() const noexcept override {
        return "ThreadSafe_Stack is empty";
    }
};


template<typename T> 
class ThreadSafe_Stack {
public:
    ThreadSafe_Stack() {}
    ThreadSafe_Stack(const ThreadSafe_Stack& o) {
        std::lock_guard<std::mutex> lock(o.m);
        data = o.data;
    }
    ThreadSafe_Stack& operator=(const ThreadSafe_Stack&)=delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    void pop(T& val) {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        val = data.top();
        data.pop();
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
private:
    std::stack<T> data;
    mutable std::mutex m;
};

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void example_basic_usage() {
    std::cout << "Example 1: basic LIFO usage\n";

    ThreadSafe_Stack<int> numbers;
    numbers.push(10);
    numbers.push(20);
    numbers.push(30);

    std::shared_ptr<int> latest = numbers.pop();
    std::cout << "pop() returned shared_ptr holding: " << *latest << '\n';

    int next_value = 0;
    numbers.pop(next_value);
    std::cout << "pop(value) copied out: " << next_value << '\n';

    int last_value = 0;
    numbers.pop(last_value);
    std::cout << "remaining value: " << last_value << "\n\n";
}

void example_exception_handling() {
    std::cout << "Example 2: handling an empty stack\n";

    ThreadSafe_Stack<std::string> messages;
    try {
        messages.pop();
    } catch (const empty_stack& e) {
        std::cout << "caught empty_stack: " << e.what() << "\n\n";
    }
}

void example_multiple_producers() {
    std::cout << "Example 3: multiple threads pushing work items\n";

    ThreadSafe_Stack<int> values;
    constexpr int producer_count = 4;
    constexpr int values_per_producer = 25;
    std::vector<std::thread> producers;

    for (int producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([producer, &values]() {
            const int start = producer * values_per_producer + 1;
            for (int value = 0; value < values_per_producer; ++value) {
                values.push(start + value);
            }
        });
    }

    for (std::thread& producer : producers) {
        producer.join();
    }

    int count = 0;
    int total = 0;
    while (!values.empty()) {
        total += *values.pop();
        ++count;
    }

    std::cout << "items pushed: " << count << '\n';
    std::cout << "sum of pushed values: " << total << "\n\n";
}

void test_pop_returns_lifo_order() {
    ThreadSafe_Stack<int> values;
    values.push(1);
    values.push(2);
    values.push(3);

    require(*values.pop() == 3, "pop() should remove the most recent value");

    int next = 0;
    values.pop(next);
    require(next == 2, "pop(T&) should return the next most recent value");

    int final_value = 0;
    values.pop(final_value);
    require(final_value == 1, "final value should be the oldest pushed item");
    require(values.empty(), "stack should be empty after removing every item");
}

void test_pop_throws_on_empty() {
    ThreadSafe_Stack<int> values;
    bool threw = false;

    try {
        values.pop();
    } catch (const empty_stack&) {
        threw = true;
    }

    require(threw, "pop() should throw empty_stack when the stack is empty");
}

void test_multiple_producers_keep_all_values() {
    ThreadSafe_Stack<int> values;
    constexpr int producer_count = 4;
    constexpr int values_per_producer = 100;
    std::vector<std::thread> producers;

    for (int producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([producer, &values]() {
            const int start = producer * values_per_producer + 1;
            for (int value = 0; value < values_per_producer; ++value) {
                values.push(start + value);
            }
        });
    }

    for (std::thread& producer : producers) {
        producer.join();
    }

    int count = 0;
    int total = 0;
    while (!values.empty()) {
        total += *values.pop();
        ++count;
    }

    const int expected_count = producer_count * values_per_producer;
    const int expected_total = expected_count * (expected_count + 1) / 2;

    require(count == expected_count, "concurrent pushes should keep every item");
    require(total == expected_total, "concurrent pushes should preserve all values");
}

void run_tests() {
    test_pop_returns_lifo_order();
    test_pop_throws_on_empty();
    test_multiple_producers_keep_all_values();
    std::cout << "Tests passed: 3/3\n";
}

int main() {
    try {
        example_basic_usage();
        example_exception_handling();
        example_multiple_producers();
        run_tests();
    } catch (const std::exception& e) {
        std::cerr << "Failure: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
