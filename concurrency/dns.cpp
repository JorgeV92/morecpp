#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

class dns_entry
{
public:
    std::string ip_address;
    int ttl_seconds{};
};

std::ostream& operator<<(std::ostream& os, dns_entry const& entry)
{
    return os << entry.ip_address << " (ttl " << entry.ttl_seconds << "s)";
}

class dns_cache
{
private:
    std::map<std::string, dns_entry> entries;
    mutable std::shared_mutex entry_mutex;

public:
    std::optional<dns_entry> find_entry(std::string const& domain) const
    {
        std::shared_lock<std::shared_mutex> lock(entry_mutex);

        auto it = entries.find(domain);

        if (it == entries.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    void update_or_add_entry(std::string const& domain,
                             dns_entry const& dns_details)
    {
        std::unique_lock<std::shared_mutex> lock(entry_mutex);

        entries[domain] = dns_details;
    }

    std::vector<std::pair<std::string, dns_entry>> snapshot() const
    {
        std::shared_lock<std::shared_mutex> lock(entry_mutex);

        return {entries.begin(), entries.end()};
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

dns_entry fake_dns_lookup(std::string const& domain)
{
    std::this_thread::sleep_for(80ms);

    if (domain == "example.com")
    {
        return {"93.184.216.34", 300};
    }
    if (domain == "cppreference.com")
    {
        return {"151.101.193.69", 180};
    }
    if (domain == "openai.com")
    {
        return {"172.64.154.211", 120};
    }
    if (domain == "ietf.org")
    {
        return {"104.16.45.99", 240};
    }

    return {"203.0.113." + std::to_string(domain.size()), 60};
}

void print_cache(dns_cache const& cache, safe_printer& printer)
{
    printer.print("Cache snapshot:");

    for (auto const& [domain, entry] : cache.snapshot())
    {
        std::ostringstream line;
        line << "  " << domain << " -> " << entry;
        printer.print(line.str());
    }
}

void example_shared_mutex()
{
    safe_printer printer;
    dns_cache cache;
    cache.update_or_add_entry("example.com", {"93.184.216.34", 300});
    cache.update_or_add_entry("cppreference.com", {"151.101.193.69", 180});

    printer.print("\nExample 1: shared_mutex allows many readers or one writer");

    std::vector<std::string> domains{
        "example.com",
        "cppreference.com",
        "openai.com"
    };

    auto reader = [&](int id) {
        for (int round = 0; round < 5; ++round)
        {
            std::string const& domain = domains[(id + round) % domains.size()];
            auto result = cache.find_entry(domain);

            std::ostringstream line;
            line << "reader " << id << " looked up " << domain << ": ";
            if (result)
            {
                line << *result;
            }
            else
            {
                line << "not cached";
            }
            printer.print(line.str());

            std::this_thread::sleep_for(40ms);
        }
    };

    std::thread writer([&] {
        std::this_thread::sleep_for(70ms);
        printer.print("writer refreshing openai.com");
        cache.update_or_add_entry("openai.com", fake_dns_lookup("openai.com"));

        std::this_thread::sleep_for(120ms);
        printer.print("writer changing example.com ttl");
        cache.update_or_add_entry("example.com", {"93.184.216.34", 60});
    });

    std::vector<std::thread> readers;
    for (int id = 0; id < 3; ++id)
    {
        readers.emplace_back(reader, id);
    }

    for (auto& thread : readers)
    {
        thread.join();
    }
    writer.join();

    print_cache(cache, printer);
}

void example_futures()
{
    safe_printer printer;
    dns_cache cache;
    std::vector<std::string> domains{
        "example.com",
        "cppreference.com",
        "openai.com",
        "ietf.org"
    };

    printer.print("\nExample 2: futures collect results from background work");

    std::vector<std::future<std::pair<std::string, dns_entry>>> lookups;
    for (auto const& domain : domains)
    {
        lookups.push_back(std::async(std::launch::async, [domain] {
            return std::make_pair(domain, fake_dns_lookup(domain));
        }));
    }

    for (auto& lookup : lookups)
    {
        auto [domain, entry] = lookup.get();
        cache.update_or_add_entry(domain, entry);

        std::ostringstream line;
        line << "cached " << domain << " -> " << entry;
        printer.print(line.str());
    }

    print_cache(cache, printer);
}

class dns_work_queue
{
private:
    std::queue<std::string> domains;
    std::mutex queue_mutex;
    std::condition_variable data_ready;
    bool stopped = false;

public:
    void push(std::string domain)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            domains.push(std::move(domain));
        }

        data_ready.notify_one();
    }

    std::optional<std::string> pop()
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        data_ready.wait(lock, [&] {
            return stopped || !domains.empty();
        });

        if (domains.empty())
        {
            return std::nullopt;
        }

        std::string domain = std::move(domains.front());
        domains.pop();
        return domain;
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stopped = true;
        }

        data_ready.notify_all();
    }
};

void example_condition_variable()
{
    safe_printer printer;
    dns_cache cache;
    dns_work_queue queue;

    printer.print("\nExample 3: condition_variable wakes worker threads");

    auto worker = [&](int id) {
        while (auto domain = queue.pop())
        {
            dns_entry entry = fake_dns_lookup(*domain);
            cache.update_or_add_entry(*domain, entry);

            std::ostringstream line;
            line << "worker " << id << " resolved " << *domain
                 << " -> " << entry;
            printer.print(line.str());
        }

        std::ostringstream line;
        line << "worker " << id << " stopping";
        printer.print(line.str());
    };

    std::thread worker_a(worker, 1);
    std::thread worker_b(worker, 2);

    for (std::string domain : {
             "example.com",
             "cppreference.com",
             "openai.com",
             "ietf.org",
             "learning-concurrency.local"
         })
    {
        queue.push(std::move(domain));
    }

    std::this_thread::sleep_for(500ms);
    queue.stop();

    worker_a.join();
    worker_b.join();

    print_cache(cache, printer);
}

int main()
{
    example_shared_mutex();
    example_futures();
    example_condition_variable();

    return 0;
}
