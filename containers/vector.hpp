#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace learning {

template <typename T>
class Vector {
    using Allocator = std::allocator<T>;
    using Traits = std::allocator_traits<Allocator>;

public:
    Vector() noexcept = default;
    explicit Vector(std::size_t initial_capacity) : capacity_(initial_capacity) {
        if (capacity_ != 0) {
            data_ = allocator_.allocate(capacity_);
        }
    }
    ~Vector() {
        // End element lifetimes before releasing their storage.
        clear();
        if (data_ != nullptr) {
            allocator_.deallocate(data_, capacity_);
        }
    }

    // A raw pointer must not accidentally give two vectors the same ownership.
    Vector(const Vector&) = delete;
    Vector& operator=(const Vector&) = delete;
    Vector(Vector&&) = delete;
    Vector& operator=(Vector&&) = delete;

    void reserve(std::size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        if (new_capacity > Traits::max_size(allocator_)) {
            throw std::length_error("Vector capacity exceeds allocator limit");
        }

        T* new_data = allocator_.allocate(new_capacity);
        std::size_t constructed = 0;
        try {
            for (; constructed < size_; ++constructed) {
                Traits::construct(allocator_, new_data + constructed,
                                  std::move_if_noexcept(data_[constructed]));
            }
        } catch (...) {
            // Only successfully constructed elements need destruction.
            while (constructed != 0) {
                --constructed;
                Traits::destroy(allocator_, new_data + constructed);
            }
            allocator_.deallocate(new_data, new_capacity);
            throw;
        }

        // All relocation succeeded. Replace the old allocation now.
        clear();
        if (data_ != nullptr) {
            allocator_.deallocate(data_, capacity_);
        }
        data_ = new_data;
        size_ = constructed;  // clear() reset size_, but the element count stays.
        capacity_ = new_capacity;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            if constexpr (std::is_move_constructible_v<T> ||
                          std::is_copy_constructible_v<T>) {
                return grow_and_emplace(std::forward<Args>(args)...);
            } else {
                throw std::length_error("Vector growth requires movable or copyable elements");
            }
        }
        Traits::construct(allocator_, data_ + size_, std::forward<Args>(args)...);
        // A throwing constructor must not count as a live element.
        ++size_;
        return data_[size_ - 1];
    }

    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }

    void pop_back() {
        if (empty()) {
            throw std::out_of_range("Cannot pop_back an empty Vector");
        }
        --size_;
        Traits::destroy(allocator_, data_ + size_);
    }

    void clear() noexcept {
        // Keep the allocation; only end the live elements' lifetimes.
        while (!empty()) {
            pop_back();
        }
    }

    // Unchecked access: index must be less than size(), not just capacity().
    T& operator[](std::size_t index) noexcept { return data_[index]; }
    const T& operator[](std::size_t index) const noexcept { return data_[index]; }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

private:
    // Called only when full. Keep the old allocation alive until commit.
    template <typename... Args>
    T& grow_and_emplace(Args&&... args) {
        const std::size_t limit = Traits::max_size(allocator_);
        if (capacity_ == limit) {
            throw std::length_error("Vector capacity exceeds allocator limit");
        }
        // Start at one; cap doubling at the limit before multiplication overflows.
        const std::size_t new_capacity = capacity_ == 0 ? 1 :
            (capacity_ > limit / 2 ? limit : capacity_ * 2);
        T* new_data = allocator_.allocate(new_capacity);
        std::size_t relocated = 0;
        bool appended = false;
        try {
            // Arguments may refer to an old element: consume them before moving it.
            Traits::construct(allocator_, new_data + size_, std::forward<Args>(args)...);
            appended = true;
            for (; relocated < size_; ++relocated) {
                Traits::construct(allocator_, new_data + relocated,
                                  std::move_if_noexcept(data_[relocated]));
            }
        } catch (...) {
            while (relocated != 0) {
                --relocated;
                Traits::destroy(allocator_, new_data + relocated);
            }
            if (appended) {
                Traits::destroy(allocator_, new_data + size_);
            }
            allocator_.deallocate(new_data, new_capacity);
            throw;
        }

        const std::size_t new_size = size_ + 1;
        clear();
        if (data_ != nullptr) {
            allocator_.deallocate(data_, capacity_);
        }
        data_ = new_data;
        size_ = new_size;
        capacity_ = new_capacity;
        return data_[size_ - 1];
    }

    Allocator allocator_;
    T* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

}  // namespace learning
