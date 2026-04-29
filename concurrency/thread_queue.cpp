#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex queue_mutex;
    std::queue<T> data_queue;
    std::condition_variable data_ready;
    bool closed = false;

public:
    void push(T value)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (closed)
            {
                throw std::runtime_error("cannot push to a closed queue");
            }

            data_queue.push(std::move(value));
        }

        data_ready.notify_one();
    }

    std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (data_queue.empty())
        {
            return std::nullopt;
        }

        T value = std::move(data_queue.front());
        data_queue.pop();
        return value;
    }

    std::optional<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        data_ready.wait(lock, [this] {
            return closed || !data_queue.empty();
        });

        if (data_queue.empty())
        {
            return std::nullopt;
        }

        T value = std::move(data_queue.front());
        data_queue.pop();
        return value;
    }

    void close()
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            closed = true;
        }

        data_ready.notify_all();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return data_queue.empty();
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        return data_queue.size();
    }
};

class safe_printer
{
private:
    std::mutex print_mutex;

public:
    void print(std::string const& message)
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << message << '\n';
    }
};

struct print_job
{
    int id{};
    std::string document;
    int pages{};
};

struct processed_job
{
    int job_id{};
    int worker_id{};
    int pages{};
};

void require(bool condition, std::string const& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

std::vector<print_job> build_jobs()
{
    return {
        {1, "quarterly-report.pdf", 8},
        {2, "shipping-labels.pdf", 3},
        {3, "team-roadmap.pdf", 5},
        {4, "invoice-1042.pdf", 2},
        {5, "training-handout.pdf", 6},
        {6, "release-checklist.pdf", 4}
    };
}

void example_try_pop()
{
    std::cout << "Example 1: try_pop does not block\n";

    threadsafe_queue<int> numbers;
    require(!numbers.try_pop().has_value(), "empty queue should not produce a value");

    numbers.push(10);
    numbers.push(20);

    auto first = numbers.try_pop();
    auto second = numbers.try_pop();
    auto third = numbers.try_pop();

    require(first && *first == 10, "first pop should return the first pushed value");
    require(second && *second == 20, "second pop should return the second pushed value");
    require(!third, "queue should be empty after two pops");

    std::cout << "popped values: " << *first << ", " << *second << "\n\n";
}

void worker_loop(int worker_id,
                 threadsafe_queue<print_job>& queue,
                 std::vector<processed_job>& processed,
                 std::mutex& processed_mutex,
                 safe_printer& printer)
{
    while (auto job = queue.wait_and_pop())
    {
        printer.print("worker " + std::to_string(worker_id) +
                      " printing " + job->document);

        std::this_thread::sleep_for(std::chrono::milliseconds(job->pages * 30));

        {
            std::lock_guard<std::mutex> lock(processed_mutex);
            processed.push_back({job->id, worker_id, job->pages});
        }
    }

    printer.print("worker " + std::to_string(worker_id) + " stopped");
}

void example_producer_consumer()
{
    std::cout << "Example 2: producers feed a queue and workers consume it\n";

    safe_printer printer;
    threadsafe_queue<print_job> queue;
    std::vector<processed_job> processed;
    std::mutex processed_mutex;

    constexpr int worker_count = 3;
    std::vector<std::thread> workers;

    for (int id = 1; id <= worker_count; ++id)
    {
        workers.emplace_back(worker_loop,
                             id,
                             std::ref(queue),
                             std::ref(processed),
                             std::ref(processed_mutex),
                             std::ref(printer));
    }

    std::thread producer([&] {
        for (print_job job : build_jobs())
        {
            printer.print("producer queued " + job.document);
            queue.push(std::move(job));
            std::this_thread::sleep_for(40ms);
        }

        queue.close();
    });

    producer.join();

    for (std::thread& worker : workers)
    {
        worker.join();
    }

    std::sort(processed.begin(), processed.end(), [](auto const& lhs, auto const& rhs) {
        return lhs.job_id < rhs.job_id;
    });

    const int total_pages = std::accumulate(
        processed.begin(),
        processed.end(),
        0,
        [](int total, processed_job const& job) {
            return total + job.pages;
        });

    std::cout << "\nProcessed jobs:\n";
    for (processed_job const& job : processed)
    {
        std::cout << "  job " << job.job_id << " printed by worker "
                  << job.worker_id << " (" << job.pages << " pages)\n";
    }

    require(processed.size() == build_jobs().size(), "every job should be processed");
    require(total_pages == 28, "processed jobs should include every page");
    require(queue.empty(), "queue should be empty after all workers stop");

    std::cout << "total pages printed: " << total_pages << "\n\n";
}

void example_close_wakes_waiter()
{
    std::cout << "Example 3: close wakes a waiting consumer\n";

    threadsafe_queue<int> queue;
    bool saw_closed_queue = false;

    std::thread waiter([&] {
        saw_closed_queue = !queue.wait_and_pop().has_value();
    });

    std::this_thread::sleep_for(100ms);
    queue.close();
    waiter.join();

    require(saw_closed_queue, "waiter should stop after queue is closed");

    std::cout << "waiting thread exited cleanly\n\n";
}

int main()
{
    try
    {
        example_try_pop();
        example_producer_consumer();
        example_close_wakes_waiter();
        std::cout << "All queue examples completed.\n";
    }
    catch (std::exception const& e)
    {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
