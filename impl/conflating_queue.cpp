#include <cstddef>
#include <iostream>
#include <list>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>


/**

* @brief A fixed-capacity FIFO queue that conflates pending values by key.
*
* A ConflatingQueue stores key-value pairs in insertion order, similar to a
* normal FIFO queue. The difference is that the queue stores at most one
* pending entry for each key.
*
* If a value is pushed with a key that already exists in the queue, the
* existing value is replaced instead of inserting another entry. This process
* is called conflation.
*
* For example, suppose the queue contains:
*
* ```
  (A, 10), (B, 20), (C, 30)
  ```
*
* Pushing:
*
* ```
  (B, 200)
  ```
*
* produces:
*
* ```
  (A, 10), (B, 200), (C, 30)
  ```
*
* The queue still contains only one entry for key B. The position of B does
* not change; only its pending value is updated.
*
* This implementation is bounded by a fixed capacity. If a new key is pushed
* while the queue is full, the oldest entry is removed before the new entry
* is inserted.
*
* For example, if the capacity is 3 and the queue contains:
*
* ```
  (A, 10), (B, 20), (C, 30)
  ```
*
* pushing:
*
* ```
  (D, 40)
  ```
*
* removes the oldest entry A and produces:
*
* ```
  (B, 20), (C, 30), (D, 40)
  ```
*
* Internally, the class uses two data structures:
*
* 1. std::list<std::pair<Key, Value>>
*
* The list stores entries in FIFO order. The first element is the oldest
* pending entry, and the last element is the newest pending entry.
*
* std::list is used because inserting or removing one element does not
* invalidate iterators that point to the other elements.
*
* 2. std::unordered_map<Key, Iterator>
*
* The hash map stores an iterator to the list entry associated with each
* key. This allows the queue to determine in average O(1) time whether a
* key is already pending and, if so, update its value directly.
*
* The following invariants are maintained:
*
* * Every key appears at most once in the list.
* * Every key in the list has exactly one corresponding entry in the map.
* * Every iterator stored in the map points to the list entry with the same key.
* * The front of the list is the oldest pending entry.
* * The back of the list is the newest pending entry.
* * The queue size never exceeds its configured capacity.
*
* push_overwrite(key, value):
*
* * If the key already exists, its stored value is replaced.
* * The queue size and key position remain unchanged.
* * If the key does not exist and space is available, the entry is appended.
* * If the key does not exist and the queue is full, the oldest entry is
* removed before the new entry is appended.
*
* pop():
*
* * Removes and returns the oldest key-value pair.
* * Removes the corresponding key from the lookup map.
* * Returns std::nullopt if the queue is empty.
*
* empty():
*
* * Returns true when the queue contains no pending entries.
*
* full():
*
* * Returns true when the number of pending entries equals the capacity.
*
* size():
*
* * Returns the current number of unique pending keys.
*
* capacity():
*
* * Returns the maximum number of unique keys that may be stored.
*
* Average time complexity:
*
* * Insert a new key:        O(1)
* * Update an existing key:  O(1)
* * Remove the oldest entry: O(1)
* * Check whether full:      O(1)
* * Check whether empty:     O(1)
*
* Space complexity:
*
* * O(capacity)
*
* This data structure is useful when only the most recent pending value for
* each key matters. Examples include:
*
* * Market-price updates keyed by stock symbol.
* * UI refresh events keyed by component identifier.
* * Sensor readings keyed by sensor identifier.
* * Configuration updates keyed by setting name.
* * Network messages where older unsent updates become obsolete.
*
* Example:
*
* ```
  ConflatingQueue<std::string, int> queue(3);
  ```
*
* ```
  queue.push_overwrite("A", 10);
  ```
* ```
  queue.push_overwrite("B", 20);
  ```
* ```
  queue.push_overwrite("C", 30);
  ```
*
* ```
  queue.push_overwrite("B", 200);
  ```
*
* ```
  // Queue order:
  ```
* ```
  // (A, 10), (B, 200), (C, 30)
  ```
*
* ```
  queue.push_overwrite("D", 40);
  ```
*
* ```
  // A is removed because the queue was full.
  ```
* ```
  // Queue order:
  ```
* ```
  // (B, 200), (C, 30), (D, 40)
  ```
*
* @tparam Key
* ```
  The type used to identify entries. Key must be hashable and comparable
  ```
* ```
  for equality because it is stored in std::unordered_map.
  ```
*
* @tparam Value
* ```
  The type of value associated with each key. Value should support the
  ```
* ```
  construction and assignment operations used by push_overwrite().
  ```
*
* @note
* This implementation is not thread-safe. External synchronization is required
* if multiple threads access the same queue and at least one thread modifies it.
  */

template<typename Key, typename Value>
class ConflatingQueue {
private:
  using Entry = std::pair<Key, Value>;
  using Iterator = typename std::list<Entry>::iterator;

public:
    explicit ConflatingQueue(std::size_t capacity)   
      : capacity_(capacity) {
      if (capacity == 0) {
        throw std::invalid_argument("capacity cant be zero.");
      }
    }

  void push_overwrite(Key key, Value value) {
    auto found = pending_.find(key);
    if (found != pending_.end()) {
      found->second->second = std::move(value);
      return;
    } 

    if (full()) {
      auto oldest = queue_.begin();
      pending_.erase(oldest->first);
      queue_.erase(oldest);
    }

    queue_.emplace_back(std::move(key), std::move(value));
    auto inserted = std::prev(queue_.end());
    pending_.emplace(inserted->first, inserted);
  }

  std::optional<Entry> pop() {
    if (empty()) 
        return std::nullopt;

    auto oldest = queue_.begin();
    pending_.erase(oldest->first);
    Entry result = std::move(*oldest);
    queue_.erase(oldest);
    return result;
  }

  bool empty() const {
    return queue_.empty();
  }

  bool full() const {
    return queue_.size() == capacity_;
  }

  std::size_t size() const {
    return queue_.size();
  }

  std::size_t capacity() const {
    return capacity_;
  }

  void print_state() const {
    std::cout << "Queue: [ ";

    for (const auto& [key, value] : queue_) {
        std::cout << '(' << key << ", " << value << ") ";
    }

    std::cout << "]\n";
    std::cout << "size = " << size()
              << ", capacity = " << capacity_
              << "\n\n";
  }

private:
  std::size_t capacity_;
  std::list<Entry> queue_;
  std::unordered_map<Key, Iterator> pending_;
};

int main() {
    ConflatingQueue<std::string, int> queue(3);

    std::cout << "Initial state\n";
    queue.print_state();

    std::cout << "Push (A, 10)\n";
    queue.push_overwrite("A", 10);
    queue.print_state();

    std::cout << "Push (B, 20)\n";
    queue.push_overwrite("B", 20);
    queue.print_state();

    std::cout << "Push (C, 30)\n";
    queue.push_overwrite("C", 30);
    queue.print_state();

    std::cout << "Push (B, 200)\n";
    std::cout << "B already exists, so 20 is replaced by 200\n";
    queue.push_overwrite("B", 200);
    queue.print_state();

    std::cout << "Push (D, 40)\n";
    std::cout << "D is a new key and the queue is full,\n"
                 "so the oldest entry, A, is removed\n";
    queue.push_overwrite("D", 40);
    queue.print_state();

    std::cout << "Push (C, 300)\n";
    std::cout << "C already exists, so 30 is replaced by 300\n";
    queue.push_overwrite("C", 300);
    queue.print_state();

    std::cout << "Pop all entries\n";

    while (auto entry = queue.pop()) {
        const auto& [key, value] = *entry;

        std::cout << "Popped: ("
                  << key << ", "
                  << value << ")\n";
    }
}
