#include <cstddef>
#include <optional>

#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream>

/**

* @brief A fixed-capacity FIFO queue implemented as a circular array.
*
* A CircularBuffer stores elements in a fixed-size array and reuses array
* positions by wrapping indices back to the beginning when they reach the end.
* This allows insertion and removal to run in constant time without shifting
* any elements.
*
* The buffer maintains three pieces of state:
*
* 1. head_
*
* The index of the oldest element currently stored in the buffer.
* The next pop operation removes the element at head_.
*
* 2. tail_
*
* The index where the next pushed element will be written.
*
* 3. size_
*
* The number of elements currently stored in the buffer.
*
* Indices are advanced using circular wraparound:
*
* ```
  next_index(i) = (i + 1) % capacity
  ```
*
* For example, with a capacity of 3:
*
* ```
  0 -> 1 -> 2 -> 0 -> 1 -> ...
  ```
*
* This wraparound behavior allows the underlying array to be reused instead
* of moving elements whenever an item is removed.
*
* The class stores:
*
* ```
  std::vector<std::optional<T>> buffer_;
  ```
*
* Each std::optional<T> represents one physical slot in the circular buffer.
* An empty optional means that the slot does not currently contain an element.
* An engaged optional contains a constructed T object.
*
* Using std::optional<T> avoids requiring T to be default-constructible.
* Elements are constructed only when they are pushed and destroyed when their
* slots are reset.
*
* The following invariants are maintained:
*
* * head_ points to the oldest stored element when the buffer is not empty.
* * tail_ points to the position where the next element will be inserted.
* * size_ is always between 0 and capacity, inclusive.
* * The buffer is empty when size_ == 0.
* * The buffer is full when size_ == capacity.
* * Elements are popped in FIFO order starting from head_.
* * Advancing an index past the final slot wraps it back to index 0.
*
* head_ and tail_ may be equal in two different situations:
*
* ```
  head_ == tail_ and size_ == 0
  ```
*
* means the buffer is empty.
*
* ```
  head_ == tail_ and size_ == capacity
  ```
*
* means the buffer is full.
*
* Therefore, size_ is required to distinguish the full state from the empty
* state.
*
* Example with capacity 3:
*
* Initial state:
*
* ```
  Physical array: [ _  _  _ ]
  ```
* ```
  head_ = 0
  ```
* ```
  tail_ = 0
  ```
* ```
  size_ = 0
  ```
*
* After pushing 10, 20, and 30:
*
* ```
  Physical array: [ 10  20  30 ]
  ```
* ```
  Logical order:  [ 10  20  30 ]
  ```
* ```
  head_ = 0
  ```
* ```
  tail_ = 0
  ```
* ```
  size_ = 3
  ```
*
* The buffer is full. tail_ wrapped back to index 0.
*
* After popping 10:
*
* ```
  Physical array: [ _  20  30 ]
  ```
* ```
  Logical order:  [ 20  30 ]
  ```
* ```
  head_ = 1
  ```
* ```
  tail_ = 0
  ```
* ```
  size_ = 2
  ```
*
* After pushing 40:
*
* ```
  Physical array: [ 40  20  30 ]
  ```
* ```
  Logical order:  [ 20  30  40 ]
  ```
* ```
  head_ = 1
  ```
* ```
  tail_ = 1
  ```
* ```
  size_ = 3
  ```
*
* The physical order in the array is not necessarily the same as the logical
* FIFO order. The logical order always begins at head_ and continues with
* circular wraparound.
*
* push(value):
*
* * Returns false if the buffer is already full.
* * Stores the new value in the optional at tail_.
* * Advances tail_ using circular wraparound.
* * Increments size_.
* * Returns true when insertion succeeds.
*
* push_overwrite(value):
*
* * Always inserts the new value.
* * Stores the value at tail_.
* * If the buffer is not full, size_ is incremented.
* * If the buffer is full, the value at tail_ is the oldest element and is
* overwritten.
* * When an old element is overwritten, head_ is advanced so that it points
* to the next-oldest element.
* * tail_ is then advanced to the next insertion position.
* * size_ remains unchanged when the buffer was already full.
*
* For example, if the buffer contains:
*
* ```
  [ 10, 20, 30 ]
  ```
*
* and both head_ and tail_ are at index 0, calling:
*
* ```
  push_overwrite(40)
  ```
*
* overwrites 10 and produces the logical order:
*
* ```
  [ 20, 30, 40 ]
  ```
*
* Both head_ and tail_ advance from index 0 to index 1, while size_ remains 3.
*
* pop():
*
* * Returns std::nullopt if the buffer is empty.
* * Moves the oldest element out of buffer_[head_].
* * Resets the optional at head_, destroying the contained T.
* * Advances head_ using circular wraparound.
* * Decrements size_.
* * Returns the removed element wrapped in std::optional<T>.
*
* front():
*
* * Returns a reference to the oldest element without removing it.
* * Dereferences buffer_[head_] to access the T stored inside the optional.
* * Throws if the buffer is empty.
*
* empty():
*
* * Returns true when size_ == 0.
*
* full():
*
* * Returns true when size_ == capacity.
*
* size():
*
* * Returns the current number of stored elements.
*
* capacity():
*
* * Returns the maximum number of elements that can be stored.
*
* Average time complexity:
*
* * push:           O(1)
* * push_overwrite: O(1)
* * pop:            O(1)
* * front:          O(1)
* * empty:          O(1)
* * full:           O(1)
* * size:           O(1)
*
* Space complexity:
*
* * O(capacity)
*
* Example:
*
* ```
  CircularBuffer<int> buffer(3);
  ```
*
* ```
  buffer.push_overwrite(10);
  ```
* ```
  buffer.push_overwrite(20);
  ```
* ```
  buffer.push_overwrite(30);
  ```
*
* ```
  // Logical order: 10, 20, 30
  ```
*
* ```
  buffer.push_overwrite(40);
  ```
*
* ```
  // 10 is overwritten.
  ```
* ```
  // Logical order: 20, 30, 40
  ```
*
* ```
  auto value = buffer.pop();
  ```
*
* ```
  if (value) {
  ```
* ```
      std::cout << *value; // Prints 20
  ```
* ```
  }
  ```
*
* This data structure is useful when:
*
* * Memory usage must remain bounded.
* * Old storage positions should be reused.
* * Constant-time FIFO insertion and removal are required.
* * The newest data should replace the oldest data when capacity is reached.
* * Streaming data, logs, audio samples, network packets, or event updates
* must be stored in a fixed-size queue.
*
* @tparam T
* ```
  The type of element stored in the buffer. T should support the
  ```
* ```
  construction, assignment, and move operations used by the buffer.
  ```
*
* @note
* This implementation is not thread-safe. External synchronization is required
* if multiple threads access the same buffer and at least one thread modifies
* it.
  */

template<typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(std::size_t capacity) 
      : buffer_(capacity) {
    if (capacity == 0)
        throw std::invalid_argument("capacity must be greater than 0");
  }

  bool push(T value) {
    if (full()) 
        return false;
    buffer_[tail_] = std::move(value);
    tail_ = get_next_idx(tail_);
    ++size_;
    return true;
  }

  void push_overwrite(T value) {
    buffer_[tail_] = std::move(value);
    if (full()) {
      head_ = get_next_idx(head_);
    } else {
      ++size_;
    }
    tail_ = get_next_idx(tail_);
  }

  std::optional<T> pop() {
    if (empty())
      return std::nullopt;
    auto val = std::move(buffer_[head_]);
    buffer_[head_].reset();
    head_ = get_next_idx(head_);
    --size_;
    return val;
  }

  const T& front() const {
    if (empty())
      throw std::out_of_range("CB is empty.");
    return *buffer_[head_];
  }

  bool full() const {
    return size_ == buffer_.size();
  }

  bool empty() const {
    return size_ == 0;
  }

  std::size_t size() const {
    return size_;
  }

  std::size_t capacity() const {
    return buffer_.size();
  }

  void print_state() const {
        std::cout << "Physical array: [ ";

        for (const auto& slot : buffer_) {
            if (slot) {
                std::cout << *slot << ' ';
            } else {
                std::cout << "_ ";
            }
        }

        std::cout << "]\n";

        std::cout << "Logical order: [ ";

        std::size_t index = head_;

        for (std::size_t count = 0; count < size_; ++count) {
            std::cout << *buffer_[index] << ' ';
            index = get_next_idx(index);
        }

        std::cout << "]\n";

        std::cout << "head = " << head_
                  << ", tail = " << tail_
                  << ", size = " << size_
                  << "\n\n";
    }

private:
    std::size_t get_next_idx(std::size_t index) const {
    return (index + 1) % buffer_.size();
  }

  std::vector<std::optional<T>> buffer_;
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t size_ = 0;
};

int main() {
  CircularBuffer<int> buf(3);
  buf.push(10);
  buf.push(20);
  buf.push(30);

  std::cout << buf.front() << '\n';

  if (!buf.push(40)) {
    std::cout << "Buffer is full\n";
  }

  std::cout << *buf.pop() << '\n';

  buf.push(40);

  while (!buf.empty()) {
    std::cout << *buf.pop() << ' ';
  }


  CircularBuffer<int> buffer(3);

  std::cout << "Initial state\n";
  buffer.print_state();

  std::cout << "Push 10\n";
  buffer.push_overwrite(10);
  buffer.print_state();

  std::cout << "Push 20\n";
  buffer.push_overwrite(20);
  buffer.print_state();

  std::cout << "Push 30\n";
  buffer.push_overwrite(30);
  buffer.print_state();

  // The buffer is full here.
  // Pushing 40 overwrites the oldest value, which is 10.
  std::cout << "Push 40: overwrites 10\n";
  buffer.push_overwrite(40);
  buffer.print_state();

  // The oldest value is now 20.
  std::cout << "Push 50: overwrites 20\n";
  buffer.push_overwrite(50);
  buffer.print_state();

  std::cout << "Pop all values\n";

  while (auto value = buffer.pop()) {
      std::cout << "Popped: " << *value << '\n';
  }

  return 0;
}


