#include <atomic>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

template <class T>
class lock_free_stack {
private:
    using count_type = std::ptrdiff_t;

    struct node;

    struct counted_node_ptr {
        count_type external_count{};
        node* ptr{};
    };

    // std::atomic<T> requires an appropriate trivially copyable value type.
    static_assert(std::is_trivially_copyable_v<counted_node_ptr>);

    struct node {
        std::shared_ptr<T> data;
        std::atomic<count_type> internal_count{0};
        counted_node_ptr next{};

        template <class... Args>
            requires std::constructible_from<T, Args...>
        explicit node(Args&&... args)
            : data(std::make_shared<T>(
                  std::forward<Args>(args)...)) {}
    };

    std::atomic<counted_node_ptr> head_{
        counted_node_ptr{0, nullptr}
    };

    // Reserve an external reference to the current head.
    // Returns false if an empty stack was observed.
    [[nodiscard]]
    bool increase_head_count(counted_node_ptr& old_head) noexcept
    {
        counted_node_ptr new_head;

        for (;;) {
            if (old_head.ptr == nullptr) {
                return false;
            }

            new_head = old_head;
            ++new_head.external_count;

            if (head_.compare_exchange_strong(
                    old_head,
                    new_head,
                    std::memory_order_acquire,
                    std::memory_order_relaxed)) {
                old_head.external_count =
                    new_head.external_count;

                return true;
            }

            // On failure, old_head has automatically been
            // replaced with the current value of head_.
        }
    }

public:
    lock_free_stack() noexcept = default;

    lock_free_stack(const lock_free_stack&) = delete;
    lock_free_stack& operator=(const lock_free_stack&) = delete;
    lock_free_stack(lock_free_stack&&) = delete;
    lock_free_stack& operator=(lock_free_stack&&) = delete;

    ~lock_free_stack()
    {
        // No other thread may access the stack during destruction.
        while (pop()) {
        }
    }

    /*
     * A two-word atomic is not guaranteed to be implemented
     * without an internal lock.
     */
    [[nodiscard]]
    bool is_lock_free() const noexcept
    {
        return head_.is_lock_free();
    }

    template <class... Args>
        requires std::constructible_from<T, Args...>
    void emplace(Args&&... args)
    {
        counted_node_ptr new_head{
            1,
            new node(std::forward<Args>(args)...)
        };

        new_head.ptr->next =
            head_.load(std::memory_order_relaxed);

        while (!head_.compare_exchange_weak(
            new_head.ptr->next,
            new_head,
            std::memory_order_release,
            std::memory_order_relaxed)) {
            // On failure, new_head.ptr->next is updated
            // with the latest head value.
        }
    }

    void push(const T& value)
    {
        emplace(value);
    }

    void push(T&& value)
    {
        emplace(std::move(value));
    }

    [[nodiscard]]
    std::shared_ptr<T> pop() noexcept
    {
        counted_node_ptr old_head =
            head_.load(std::memory_order_relaxed);

        for (;;) {
            /*
             * Do not dereference old_head.ptr until we have
             * successfully reserved an external reference.
             */
            if (!increase_head_count(old_head)) {
                return {};
            }

            node* const ptr = old_head.ptr;

            if (head_.compare_exchange_strong(
                    old_head,
                    ptr->next,
                    std::memory_order_relaxed,
                    std::memory_order_relaxed)) {
                /*
                 * We successfully removed ptr from the stack.
                 * Move the shared ownership of T into result.
                 */
                std::shared_ptr<T> result;
                result.swap(ptr->data);

                /*
                 * Remove:
                 *   1 reference owned by head_
                 *   1 reference owned by this pop operation
                 *
                 * Transfer all other external references to
                 * internal_count.
                 */
                const count_type count_increase =
                    old_head.external_count - 2;

                if (ptr->internal_count.fetch_add(
                        count_increase,
                        std::memory_order_release)
                    == -count_increase) {
                    delete ptr;
                }

                return result;
            }

            /*
             * Another thread changed head_.
             *
             * We cannot decrement the old node's external count
             * through head_, so record the released reference in
             * internal_count instead.
             */
            if (ptr->internal_count.fetch_sub(
                    1,
                    std::memory_order_relaxed) == 1) {
                /*
                 * We changed the count from 1 to 0 and are the
                 * final thread using this node.
                 */
                ptr->internal_count.load(
                    std::memory_order_acquire);

                delete ptr;
            }

            /*
             * The failed CAS has already updated old_head with
             * the latest value of head_, so retry.
             */
        }
    }
};
