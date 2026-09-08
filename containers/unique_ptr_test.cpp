#include "unique_ptr.hpp"

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <utility>

struct Tracked {
    inline static int alive = 0;
    inline static int destroyed = 0;

    explicit Tracked(int value) : value(value) { ++alive; }
    ~Tracked() {
        --alive;
        ++destroyed;
    }

    int value;
};

int main() {
    using Owner = learning::UniquePtr<Tracked>;
    // Ownership cannot be duplicated, transferred, or acquired implicitly yet.
    static_assert(!std::is_copy_constructible_v<Owner>);
    static_assert(!std::is_copy_assignable_v<Owner>);
    static_assert(!std::is_move_constructible_v<Owner>);
    static_assert(!std::is_move_assignable_v<Owner>);
    static_assert(!std::is_convertible_v<Tracked*, Owner>);
    static_assert(!std::is_convertible_v<Owner, bool>);
    static_assert(std::is_same_v<decltype(*std::declval<const Owner&>()), Tracked&>);
    static_assert(std::is_same_v<
                  decltype(*std::declval<learning::UniquePtr<const Tracked>&>()),
                  const Tracked&>);

    {
        Owner empty;
        Owner null(nullptr);
        assert(!empty && empty.get() == nullptr);
        assert(!null && null.get() == nullptr);
    }
    assert(Tracked::alive == 0 && Tracked::destroyed == 0);

    {
        const Owner owner(new Tracked(42));
        assert(owner && Tracked::alive == 1);
        assert(&*owner == owner.get());
        (*owner).value = 43;  // A const owner still provides mutable access to T.
        Tracked* borrowed = owner.get();
        assert(borrowed->value == 43 && owner->value == 43);
        assert(Tracked::destroyed == 0);  // Borrowing did not release ownership.
    }
    assert(Tracked::alive == 0 && Tracked::destroyed == 1);

    try {
        Owner owner(new Tracked(7));
        throw std::runtime_error("leave the owner's scope");
    } catch (const std::runtime_error&) {
        assert(Tracked::alive == 0 && Tracked::destroyed == 2);
    }

    {
        learning::UniquePtr<const Tracked> owner(new const Tracked(99));
        assert(owner->value == 99 && Tracked::alive == 1);
    }
    assert(Tracked::alive == 0 && Tracked::destroyed == 3);
}
