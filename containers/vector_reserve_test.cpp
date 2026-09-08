#include "vector.hpp"

#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

// Copying can fail after a chosen number of successful constructions.
struct Copyable {
    inline static int alive = 0;
    inline static int copies_before_throw = -1;

    explicit Copyable(int value) : value(value) { ++alive; }
    Copyable(const Copyable& other) : value(other.value) {
        if (copies_before_throw == 0) {
            throw std::runtime_error("copy failed");
        }
        if (copies_before_throw > 0) {
            --copies_before_throw;
        }
        ++alive;
    }
    Copyable(Copyable&&) noexcept(false) {
        throw std::runtime_error("reserve should prefer copying this type");
    }
    ~Copyable() { --alive; }

    int value;
};

// Even the move that throws can have already changed its source.
struct ThrowingMoveOnly {
    inline static int alive = 0;
    inline static int moves_before_throw = -1;

    explicit ThrowingMoveOnly(int value) : value(value) { ++alive; }
    ThrowingMoveOnly(const ThrowingMoveOnly&) = delete;
    ThrowingMoveOnly(ThrowingMoveOnly&& other)
        : value(std::exchange(other.value, -1)) {
        if (moves_before_throw == 0) {
            throw std::runtime_error("move failed");
        }
        if (moves_before_throw > 0) {
            --moves_before_throw;
        }
        ++alive;
    }
    ~ThrowingMoveOnly() { --alive; }

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
    learning::Vector<int> empty;
    empty.reserve(0);
    assert(empty.empty() && empty.capacity() == 0);
    empty.reserve(2);
    assert(empty.empty() && empty.capacity() == 2);
    empty.emplace_back(42);
    assert(empty[0] == 42);
    empty.clear();
    empty.reserve(4);  // Grow allocated storage with no live elements.
    assert(empty.empty() && empty.capacity() == 4);
    empty.emplace_back(99);
    assert(empty[0] == 99);

    {
        learning::Vector<Copyable> items(3);
        items.emplace_back(10);
        items.emplace_back(20);
        items.emplace_back(30);
        Copyable* first = &items[0];

        Copyable::copies_before_throw = 0;
        items.reserve(0);
        items.reserve(2);
        items.reserve(3);
        assert(items.size() == 3 && items.capacity() == 3);
        assert(&items[0] == first && Copyable::alive == 3);

        // Failure at any copy position must leave the old elements intact.
        for (int successful_copies = 0; successful_copies < 3; ++successful_copies) {
            Copyable::copies_before_throw = successful_copies;
            expect_throw<std::runtime_error>([&] { items.reserve(6); });
            assert(items.size() == 3 && items.capacity() == 3);
            assert(&items[0] == first && Copyable::alive == 3);
            assert(items[0].value == 10 && items[1].value == 20);
            assert(items[2].value == 30);
        }

        expect_throw<std::length_error>([&] {
            items.reserve(std::numeric_limits<std::size_t>::max());
        });
        assert(items.size() == 3 && items.capacity() == 3);
        assert(&items[0] == first && Copyable::alive == 3);

        // Growing insertion constructs the appended copy before relocating three
        // old elements. Fail at each of those four construction positions.
        for (int successful_copies = 0; successful_copies < 4; ++successful_copies) {
            Copyable::copies_before_throw = successful_copies;
            expect_throw<std::runtime_error>([&] { items.push_back(items[0]); });
            assert(items.size() == 3 && items.capacity() == 3);
            assert(&items[0] == first && Copyable::alive == 3);
            assert(items[0].value == 10 && items[1].value == 20);
            assert(items[2].value == 30);
        }

        Copyable::copies_before_throw = -1;
        items.reserve(6);
        // first is now invalid; access the relocated elements through items.
        assert(items.size() == 3 && items.capacity() == 6);
        assert(Copyable::alive == 3);
        assert(items[0].value == 10 && items[1].value == 20);
        assert(items[2].value == 30);
        items.emplace_back(40);
        assert(items.size() == 4 && items[3].value == 40);
    }
    assert(Copyable::alive == 0);

    {
        learning::Vector<Copyable> items;
        Copyable source(5);
        Copyable::copies_before_throw = 0;
        expect_throw<std::runtime_error>([&] { items.push_back(source); });
        assert(items.empty() && items.capacity() == 0 && Copyable::alive == 1);
        Copyable::copies_before_throw = -1;
        items.push_back(source);
        items.push_back(items[0]);
        assert(items.size() == 2 && items.capacity() == 2);
        assert(items[0].value == 5 && items[1].value == 5 && Copyable::alive == 3);
    }
    assert(Copyable::alive == 0);

    {
        learning::Vector<std::unique_ptr<Copyable>> owners(1);
        owners.emplace_back(std::make_unique<Copyable>(77));
        Copyable* pointee = owners[0].get();
        owners.reserve(4);  // unique_ptr has a noexcept move and no copy.
        assert(owners.size() == 1 && owners.capacity() == 4);
        assert(owners[0].get() == pointee && owners[0]->value == 77);
        assert(Copyable::alive == 1);  // Moving ownership does not copy its object.
    }
    assert(Copyable::alive == 0);

    {
        learning::Vector<ThrowingMoveOnly> items(3);
        items.emplace_back(10);
        items.emplace_back(20);
        items.emplace_back(30);
        ThrowingMoveOnly::moves_before_throw = 1;
        expect_throw<std::runtime_error>([&] { items.reserve(6); });
        assert(items.size() == 3 && items.capacity() == 3);
        assert(ThrowingMoveOnly::alive == 3);
        assert(items[0].value == -1 && items[1].value == -1);
        assert(items[2].value == 30);

        items.clear();  // Changed source values still belong to live objects.
        assert(ThrowingMoveOnly::alive == 0);

        items.emplace_back(10);
        items.emplace_back(20);
        items.emplace_back(30);
        ThrowingMoveOnly::moves_before_throw = 0;
        expect_throw<std::runtime_error>([&] { items.push_back(std::move(items[0])); });
        assert(items.size() == 3 && items.capacity() == 3);
        assert(items[0].value == -1 && items[1].value == 20);
        assert(items[2].value == 30 && ThrowingMoveOnly::alive == 3);

        ThrowingMoveOnly::moves_before_throw = 1;
        expect_throw<std::runtime_error>([&] { items.emplace_back(40); });
        assert(items.size() == 3 && items.capacity() == 3);
        assert(items[0].value == -1 && items[1].value == -1);
        assert(items[2].value == 30 && ThrowingMoveOnly::alive == 3);
        items.clear();  // Also verifies cleanup of the separately appended object.
        assert(ThrowingMoveOnly::alive == 0);

        items.emplace_back(50);
        ThrowingMoveOnly::moves_before_throw = -1;
        items.reserve(6);
        assert(items.size() == 1 && items.capacity() == 6);
        assert(items[0].value == 50 && ThrowingMoveOnly::alive == 1);
    }
    assert(ThrowingMoveOnly::alive == 0);
}
