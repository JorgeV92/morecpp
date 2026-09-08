#pragma once

#include <cstddef>
#include <memory>

namespace learning {

template <typename T>
class Vector {
public:
    Vector() noexcept = default;
    explicit Vector(std::size_t initial_capacity) : capacity_(initial_capacity) {
        if (capacity_ != 0) {
            data_ = allocator_.allocate(capacity_);
        }
    }
    ~Vector() {
        // No elements exist yet, so only the storage needs releasing.
        if (data_ != nullptr) {
            allocator_.deallocate(data_, capacity_);
        }
    }

    // A raw pointer must not accidentally give two vectors the same ownership.
    Vector(const Vector&) = delete;
    Vector& operator=(const Vector&) = delete;
    Vector(Vector&&) = delete;
    Vector& operator=(Vector&&) = delete;

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

private:
    std::allocator<T> allocator_;
    T* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

}  // namespace learning
