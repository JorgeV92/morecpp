#pragma once

#include <type_traits>

namespace learning {

// Stage 1: exclusive ownership of one object; ownership transfer comes later.
template <typename T>
class UniquePtr {
    static_assert(std::is_object_v<T> && !std::is_array_v<T>,
                  "UniquePtr<T> supports single object types only");

public:
    UniquePtr() noexcept = default;
    explicit UniquePtr(T* ptr) noexcept : ptr_(ptr) {}

    ~UniquePtr() noexcept {
        static_assert(sizeof(T) > 0, "T must be complete when deleting it");
        delete ptr_;  // Deleting nullptr is safe.
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
    // Defaulted moves would copy the raw pointer, creating two owners.
    UniquePtr(UniquePtr&&) = delete;
    UniquePtr& operator=(UniquePtr&&) = delete;

    [[nodiscard]] T* get() const noexcept { return ptr_; }  // Borrowed access.
    explicit operator bool() const noexcept { return ptr_ != nullptr; }

    // Access requires a non-null pointer. Const ownership does not make T const.
    T& operator*() const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }

private:
    T* ptr_ = nullptr;
};

}  // namespace learning
