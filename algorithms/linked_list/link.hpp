#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace ds {

template <class T, class Allocator = std::allocator<T>>
class List {
private:
    struct NodeBase {
        NodeBase* prev;
        NodeBase* next;

        NodeBase() noexcept : prev(this), next(this) {}
    };

    struct Node final : NodeBase {
        template <class... Args>
        explicit Node(Args&&... args)
            : NodeBase{}, value(std::forward<Args>(args)...) {}

        T value;
    };

    using NodeAllocator =
        typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    using NodeTraits = std::allocator_traits<NodeAllocator>;

public:
    using value_type      = T;
    using allocator_type  = Allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;

    template <bool IsConst>
    class BasicIterator {
        using base_pointer =
            std::conditional_t<IsConst, const NodeBase*, NodeBase*>;

        using node_pointer =
            std::conditional_t<IsConst, const Node*, Node*>;

        base_pointer node_ = nullptr;

        friend class List;

        template <bool>
        friend class BasicIterator;

        explicit BasicIterator(base_pointer node) noexcept : node_(node) {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept  = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using reference         = std::conditional_t<IsConst, const T&, T&>;
        using pointer           = std::conditional_t<IsConst, const T*, T*>;

        BasicIterator() noexcept = default;
        BasicIterator(const BasicIterator&) noexcept = default;
        BasicIterator& operator=(const BasicIterator&) noexcept = default;

        BasicIterator(const BasicIterator<false>& other) noexcept requires IsConst
            : node_(other.node_) {}

        reference operator*() const noexcept {
            return static_cast<node_pointer>(node_)->value;
        }

        pointer operator->() const noexcept {
            return std::addressof(operator*());
        }

        BasicIterator& operator++() noexcept {
            node_ = node_->next;
            return *this;
        }

        BasicIterator operator++(int) noexcept {
            BasicIterator copy = *this;
            ++(*this);
            return copy;
        }

        BasicIterator& operator--() noexcept {
            node_ = node_->prev;
            return *this;
        }

        BasicIterator operator--(int) noexcept {
            BasicIterator copy = *this;
            --(*this);
            return copy;
        }

        friend bool operator==(const BasicIterator& a,
                               const BasicIterator& b) noexcept {
            return a.node_ == b.node_;
        }
    };

    using iterator               = BasicIterator<false>;
    using const_iterator         = BasicIterator<true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List() noexcept(std::is_nothrow_default_constructible_v<NodeAllocator>)
        : sentinel_{}, size_{0}, alloc_{} {}

    explicit List(const Allocator& allocator)
        : sentinel_{}, size_{0}, alloc_{allocator} {}

    List(std::initializer_list<T> values,
         const Allocator& allocator = Allocator{})
        : List(allocator) {
        try {
            for (const auto& value : values) {
                push_back(value);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(const List& other)
        : sentinel_{},
          size_{0},
          alloc_{NodeTraits::select_on_container_copy_construction(
              other.alloc_)} {
        try {
            for (const auto& value : other) {
                push_back(value);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    List(List&& other) noexcept(std::is_nothrow_move_constructible_v<NodeAllocator>)
        : sentinel_{}, size_{0}, alloc_{std::move(other.alloc_)} {
        steal_nodes_from(other);
    }

    ~List() noexcept {
        clear();
    }

    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }

        NodeAllocator target_alloc = alloc_;

        if constexpr (NodeTraits::propagate_on_container_copy_assignment::value) {
            target_alloc = other.alloc_;
        }

        List temp{target_alloc};

        for (const auto& value : other) {
            temp.push_back(value);
        }

        clear();

        if constexpr (NodeTraits::propagate_on_container_copy_assignment::value) {
            alloc_ = other.alloc_;
        }

        steal_nodes_from(temp);
        return *this;
    }

    List& operator=(List&& other) noexcept(
        NodeTraits::propagate_on_container_move_assignment::value ||
        NodeTraits::is_always_equal::value) {
        if (this == &other) {
            return *this;
        }

        clear();

        if constexpr (NodeTraits::propagate_on_container_move_assignment::value) {
            alloc_ = std::move(other.alloc_);
            steal_nodes_from(other);
        } else if constexpr (NodeTraits::is_always_equal::value) {
            steal_nodes_from(other);
        } else {
            if (alloc_ == other.alloc_) {
                steal_nodes_from(other);
            } else {
                for (auto& value : other) {
                    push_back(std::move(value));
                }
                other.clear();
            }
        }

        return *this;
    }

    allocator_type get_allocator() const {
        return allocator_type{alloc_};
    }

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }

    reference front() {
        throw_if_empty("front");
        return *begin();
    }

    const_reference front() const {
        throw_if_empty("front");
        return *begin();
    }

    reference back() {
        throw_if_empty("back");
        return *std::prev(end());
    }

    const_reference back() const {
        throw_if_empty("back");
        return *std::prev(end());
    }

    iterator begin() noexcept {
        return iterator{sentinel_.next};
    }

    const_iterator begin() const noexcept {
        return const_iterator{sentinel_.next};
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    iterator end() noexcept {
        return iterator{&sentinel_};
    }

    const_iterator end() const noexcept {
        return const_iterator{&sentinel_};
    }

    const_iterator cend() const noexcept {
        return end();
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator{end()};
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator{end()};
    }

    const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator{begin()};
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator{begin()};
    }

    const_reverse_iterator crend() const noexcept {
        return rend();
    }

    void push_front(const T& value) {
        insert(begin(), value);
    }

    void push_front(T&& value) {
        insert(begin(), std::move(value));
    }

    void push_back(const T& value) {
        insert(end(), value);
    }

    void push_back(T&& value) {
        insert(end(), std::move(value));
    }

    template <class... Args>
    reference emplace_front(Args&&... args) {
        return *emplace(begin(), std::forward<Args>(args)...);
    }

    template <class... Args>
    reference emplace_back(Args&&... args) {
        return *emplace(end(), std::forward<Args>(args)...);
    }

    iterator insert(const_iterator position, const T& value) {
        return emplace(position, value);
    }

    iterator insert(const_iterator position, T&& value) {
        return emplace(position, std::move(value));
    }

    template <class... Args>
    iterator emplace(const_iterator position, Args&&... args) {
        Node* node = create_node(std::forward<Args>(args)...);

        link_before(const_cast<NodeBase*>(position.node_), node);

        return iterator{node};
    }

    iterator erase(const_iterator position) {
        NodeBase* current = const_cast<NodeBase*>(position.node_);

        if (current == &sentinel_) {
            throw std::out_of_range("List::erase cannot erase end()");
        }

        NodeBase* next = current->next;

        unlink(current);
        destroy_node(static_cast<Node*>(current));
        --size_;

        return iterator{next};
    }

    iterator erase(const_iterator first, const_iterator last) {
        while (first != last) {
            first = erase(first);
        }

        return iterator{const_cast<NodeBase*>(last.node_)};
    }

    void pop_front() {
        throw_if_empty("pop_front");
        erase(begin());
    }

    void pop_back() {
        throw_if_empty("pop_back");
        erase(std::prev(end()));
    }

    void clear() noexcept {
        NodeBase* current = sentinel_.next;

        while (current != &sentinel_) {
            NodeBase* next = current->next;
            destroy_node(static_cast<Node*>(current));
            current = next;
        }

        reset_empty();
        size_ = 0;
    }

    void swap(List& other) noexcept(
        NodeTraits::propagate_on_container_swap::value ||
        NodeTraits::is_always_equal::value) {
        if (this == &other) {
            return;
        }

        if constexpr (NodeTraits::propagate_on_container_swap::value) {
            using std::swap;
            swap(alloc_, other.alloc_);
        } else {
            assert(alloc_ == other.alloc_ || NodeTraits::is_always_equal::value);
        }

        using std::swap;

        swap(sentinel_.next, other.sentinel_.next);
        swap(sentinel_.prev, other.sentinel_.prev);
        swap(size_, other.size_);

        repair_sentinel_links();
        other.repair_sentinel_links();
    }

    friend void swap(List& a, List& b) noexcept(noexcept(a.swap(b))) {
        a.swap(b);
    }

    friend bool operator==(const List& a, const List& b) {
        return a.size_ == b.size_ &&
               std::equal(a.begin(), a.end(), b.begin());
    }

private:
    NodeBase sentinel_;
    size_type size_;
    [[no_unique_address]] NodeAllocator alloc_;

    explicit List(const NodeAllocator& allocator)
        : sentinel_{}, size_{0}, alloc_{allocator} {}

    void reset_empty() noexcept {
        sentinel_.next = &sentinel_;
        sentinel_.prev = &sentinel_;
    }

    void repair_sentinel_links() noexcept {
        if (size_ == 0) {
            reset_empty();
            return;
        }

        sentinel_.next->prev = &sentinel_;
        sentinel_.prev->next = &sentinel_;
    }

    template <class... Args>
    Node* create_node(Args&&... args) {
        Node* node = NodeTraits::allocate(alloc_, 1);

        try {
            NodeTraits::construct(alloc_, node, std::forward<Args>(args)...);
        } catch (...) {
            NodeTraits::deallocate(alloc_, node, 1);
            throw;
        }

        return node;
    }

    void destroy_node(Node* node) noexcept {
        NodeTraits::destroy(alloc_, node);
        NodeTraits::deallocate(alloc_, node, 1);
    }

    void link_before(NodeBase* position, NodeBase* node) noexcept {
        node->next = position;
        node->prev = position->prev;

        position->prev->next = node;
        position->prev = node;

        ++size_;
    }

    static void unlink(NodeBase* node) noexcept {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    void steal_nodes_from(List& other) noexcept {
        if (other.empty()) {
            reset_empty();
            size_ = 0;
            return;
        }

        sentinel_.next = other.sentinel_.next;
        sentinel_.prev = other.sentinel_.prev;
        size_ = other.size_;

        sentinel_.next->prev = &sentinel_;
        sentinel_.prev->next = &sentinel_;

        other.reset_empty();
        other.size_ = 0;
    }

    void throw_if_empty(const char* operation) const {
        if (empty()) {
            throw std::out_of_range(
                std::string("List::") + operation + " on empty list"
            );
        }
    }
};

} // namespace ds