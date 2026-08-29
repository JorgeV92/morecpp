#pragma once 

#include <cstddef>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <utility>

/*
    short string
    length <= 128

    CompactString object
    ┌───────────────────────────────┐
    │ buffer                        │
    │ "hello"                       │
    │                               │
    │                               │
    ├───────────────────────────────┤
    │ length = 5                    │
    ├───────────────────────────────┤
    │ heap_ptr = nullptr            │
    └───────────────────────────────┘

    NO heap allocation


    length > 128

    CompactString object
    ┌───────────────────────────────┐
    │ buffer (unused)               │
    ├───────────────────────────────┤
    │ length = 500                  │
    ├───────────────────────────────┤
    │ heap_ptr ─────────────────────┼───► heap
    └───────────────────────────────┘   ┌──────────────┐
                                        │ 500 chars    │
                                        └──────────────┘

    If length <= BUFFER_SIZE, the characters are stored in buffer. Otherwise, they're stored in heap_ptr.
*/

constexpr size_t BUFFER_SIZE = 128;

class CompactString {
public:
    CompactString(const char* src, size_t len) : length(len), heap_ptr(nullptr) {
        if (len > 0 && src == nullptr) throw std::invalid_argument("src is null");
        if (!is_small()) { heap_ptr = new char[length]; }
        if (length > 0) std::memcpy(data(), src, length);
    }

    CompactString(const CompactString& o) : length(o.length), heap_ptr(nullptr) {
        if (!is_small()) heap_ptr = new char[length];
        if (length > 0) std::memcpy(data(), o.data(), length);
    }

    CompactString(CompactString&& o) noexcept : length(o.length), heap_ptr(nullptr) {
        if (o.is_small()) {
            if (length > 0) std::memcpy(buffer, o.buffer, length);
        } else { heap_ptr = o.heap_ptr; o.heap_ptr = nullptr; }
        o.length = 0;
    }

    CompactString& operator=(const CompactString& o) {
        if (this == &o) return *this;
        CompactString tmp(o);
        *this = std::move(tmp);
        return *this;
    }

    CompactString& operator=(CompactString&& o) noexcept {
        if (this == &o) return *this;
        delete[] heap_ptr;
        heap_ptr = nullptr;
        length = o.length;
        if (o.is_small()) { if (length>0) std::memcpy(buffer, o.buffer, length); }
        else { heap_ptr = o.heap_ptr; o.heap_ptr = nullptr; }
        o.length = 0;
        return *this;
    }

    std::size_t size() const { return length; }

    int compare(const CompactString& o) const {
        const std::size_t common = std::min(length, o.length);
        int res = std::memcmp(data(), o.data(), common);
        if (res != 0) return res;
        if (length < o.length) return -1;
        if (length > o.length) return 1;
        return 0;
    }

    CompactString concat(const CompactString& o) const {
        CompactString res(length + o.length);
        if (length >0) {
            std::memcpy(res.data(), data(), length);
        } 
        if (o.length >0) {
            std::memcpy(res.data() + length, o.data(), o.length);
        }
        return res;
    }

    CompactString substr(std::size_t pos, std::size_t count) const {
        if (pos > length || count > length - pos) throw std::out_of_range("substring");
        CompactString res(count);
        if (count > 0) { std::memcpy(res.data(), data() + pos, count); }
        return res;
    }

    ~CompactString() { delete[] heap_ptr; }
private:
    char buffer[BUFFER_SIZE];
    size_t length;
    char* heap_ptr;

    bool is_small() const { return length <= BUFFER_SIZE; }
    char* data() { return is_small() ? buffer : heap_ptr; }
    const char* data() const { return is_small() ? buffer : heap_ptr; }
    explicit CompactString(std::size_t len) : length(len), heap_ptr(nullptr) {
        if (!is_small()) heap_ptr = new char[length];
    }
};

