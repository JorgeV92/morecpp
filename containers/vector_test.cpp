#include "vector.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

struct Tracked {
    inline static int alive = 0;

    explicit Tracked(int value) : value(value) {
        if (value < 0) {
            throw std::runtime_error("construction failed");
        }
        ++alive;
    }
    ~Tracked() { --alive; }
    Tracked(const Tracked&) = delete;
    Tracked& operator=(const Tracked&) = delete;

    int value;
};

template <typename Exception, typename Action>
void expect_throw(Action action) {
    bool threw = false;
    try {
        action();
    } catch (const Exception&) {
        threw = true;
    }
    assert(threw);
}

int main() {
    using IntVector = learning::Vector<int>;
    static_assert(std::is_same_v<decltype(std::declval<IntVector&>()[0]), int&>);
    static_assert(std::is_same_v<decltype(std::declval<const IntVector&>()[0]),
                                 const int&>);

    {
        learning::Vector<Tracked> items(3);
        assert(Tracked::alive == 0);  // Capacity alone constructs nothing.
        int value = 42;
        Tracked& first = items.emplace_back(value);  // Forward an lvalue.
        assert(&first == &items[0]);
        first.value = 43;

        expect_throw<std::runtime_error>([&] { items.emplace_back(-1); });
        assert(items.size() == 1 && Tracked::alive == 1);
        assert(items[0].value == 43);

        // Reuse the slot whose construction failed.
        items.emplace_back(7);
        items.emplace_back(9);
        assert(items.size() == 3 && items.capacity() == 3);
        assert(items[1].value == 7 && items[2].value == 9);
        assert(Tracked::alive == 3);

        // Full capacity must be detected before the throwing constructor runs.
        expect_throw<std::length_error>([&] { items.emplace_back(-1); });
        assert(items.size() == 3 && Tracked::alive == 3);
    }
    assert(Tracked::alive == 0);

    // Destruction also runs when an unrelated exception leaves the scope.
    expect_throw<std::runtime_error>([] {
        learning::Vector<Tracked> items(3);
        items.emplace_back(1);
        throw std::runtime_error("leave scope");
    });
    assert(Tracked::alive == 0);

    learning::Vector<Tracked> empty;
    expect_throw<std::length_error>([&] { empty.emplace_back(1); });
    assert(empty.empty() && empty.capacity() == 0 && Tracked::alive == 0);

    learning::Vector<std::unique_ptr<int>> pointers(1);
    auto owner = std::make_unique<int>(99);
    pointers.emplace_back(std::move(owner));  // Forward a move-only argument.
    assert(owner == nullptr && *pointers[0] == 99);
}
