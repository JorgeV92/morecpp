#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

template <
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>
>
class threadsafe_lookup_table {
private:
    class bucket_type {
    private:
        using bucket_value = std::pair<Key, Value>;
        using bucket_data = std::list<bucket_value>;

        using bucket_iterator = typename bucket_data::iterator;
        using const_bucket_iterator =
            typename bucket_data::const_iterator;

        bucket_data data;

        // mutable allows const functions such as value_for()
        // to lock the mutex.
        mutable boost::shared_mutex mutex;

        bucket_iterator find_entry_for(const Key& key) {
            return std::find_if(
                data.begin(),
                data.end(),
                [&](const bucket_value& item) {
                    return item.first == key;
                }
            );
        }

        const_bucket_iterator find_entry_for(const Key& key) const {
            return std::find_if(
                data.cbegin(),
                data.cend(),
                [&](const bucket_value& item) {
                    return item.first == key;
                }
            );
        }

    public:
        Value value_for(
            const Key& key,
            const Value& default_value
        ) const {
            boost::shared_lock<boost::shared_mutex> lock(mutex);

            const auto found_entry = find_entry_for(key);

            if (found_entry == data.cend()) {
                return default_value;
            }

            return found_entry->second;
        }

        void add_or_update_mapping(
            const Key& key,
            const Value& value
        ) {
            boost::unique_lock<boost::shared_mutex> lock(mutex);

            auto found_entry = find_entry_for(key);

            if (found_entry == data.end()) {
                data.emplace_back(key, value);
            } else {
                found_entry->second = value;
            }
        }

        void remove_mapping(const Key& key) {
            boost::unique_lock<boost::shared_mutex> lock(mutex);

            auto found_entry = find_entry_for(key);

            if (found_entry != data.end()) {
                data.erase(found_entry);
            }
        }
    };

    std::vector<std::unique_ptr<bucket_type>> buckets;
    Hash hasher;

    bucket_type& get_bucket(const Key& key) const {
        const std::size_t bucket_index =
            hasher(key) % buckets.size();

        return *buckets[bucket_index];
    }

public:
    using key_type = Key;
    using mapped_type = Value;
    using hash_type = Hash;

    explicit threadsafe_lookup_table(
        unsigned num_buckets = 19,
        const Hash& hasher_ = Hash()
    )
        : buckets(num_buckets),
          hasher(hasher_) {
        if (num_buckets == 0) {
            throw std::invalid_argument(
                "Number of buckets cannot be zero"
            );
        }

        for (auto& bucket : buckets) {
            bucket = std::make_unique<bucket_type>();
        }
    }

    threadsafe_lookup_table(
        const threadsafe_lookup_table&
    ) = delete;

    threadsafe_lookup_table& operator=(
        const threadsafe_lookup_table&
    ) = delete;

    Value value_for(
        const Key& key,
        const Value& default_value = Value()
    ) const {
        return get_bucket(key).value_for(key, default_value);
    }

    void add_or_update_mapping(
        const Key& key,
        const Value& value
    ) {
        get_bucket(key).add_or_update_mapping(key, value);
    }

    void remove_mapping(const Key& key) {
        get_bucket(key).remove_mapping(key);
    }
};

int main() {
    threadsafe_lookup_table<std::string, std::string> statuses;

    statuses.add_or_update_mapping("server", "starting");

    auto reader = [&statuses](int reader_id) {
        for (int i = 0; i < 5; ++i) {
            const std::string status =
                statuses.value_for("server", "unknown");

            std::cout
                << "Reader "
                << reader_id
                << " saw: "
                << status
                << '\n';
        }
    };

    auto writer = [&statuses] {
        statuses.add_or_update_mapping("server", "running");
        statuses.add_or_update_mapping("server", "healthy");
    };

    std::thread reader1(reader, 1);
    std::thread reader2(reader, 2);
    std::thread writer_thread(writer);

    reader1.join();
    reader2.join();
    writer_thread.join();

    std::cout
        << "Final status: "
        << statuses.value_for("server", "unknown")
        << '\n';
}
