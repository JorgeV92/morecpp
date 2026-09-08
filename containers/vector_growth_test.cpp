#include "vector.hpp"

#include <cassert>
#include <string>
#include <utility>

// Count moves to distinguish total linear relocation from quadratic growth.
struct Counted {
    inline static int moves = 0;

    explicit Counted(int value) : value(value) {}
    Counted(const Counted&) = delete;
    Counted(Counted&& other) noexcept : value(std::exchange(other.value, -1)) {
        ++moves;
    }

    int value;
};

int main() {
    learning::Vector<int> numbers;
    numbers.push_back(10);
    assert(numbers.size() == 1 && numbers.capacity() == 1);
    numbers.emplace_back(20);
    assert(numbers.size() == 2 && numbers.capacity() == 2);
    numbers.push_back(30);
    assert(numbers.size() == 3 && numbers.capacity() == 4);
    int* first = &numbers[0];
    numbers.push_back(40);  // Available capacity keeps existing references valid.
    assert(&numbers[0] == first && numbers.capacity() == 4);
    int& last = numbers.emplace_back(50);
    assert(&last == &numbers[4]);
    assert(numbers.size() == 5 && numbers.capacity() == 8);
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        assert(numbers[i] == static_cast<int>((i + 1) * 10));
    }

    // A non-power-of-two initial capacity also grows geometrically.
    learning::Vector<int> odd_capacity(3);
    for (int i = 0; i < 4; ++i) {
        odd_capacity.push_back(i);
    }
    assert(odd_capacity.size() == 4 && odd_capacity.capacity() == 6);

    const std::string text(100, 'x');
    learning::Vector<std::string> words(1);
    words.push_back(text);  // Copy an lvalue.
    words.push_back(words[0]);  // The source is in the full allocation.
    assert(words[0] == text && words[1] == text);
    std::string& appended = words.emplace_back(words[0]);
    assert(&appended == &words[2] && appended == text);
    assert(words[0] == text && words[1] == text);
    words.push_back(words[1]);  // Aliasing without reallocation works too.
    assert(words.size() == 4 && words[3] == text);

    // Constructor arguments can also refer to members of an existing element.
    learning::Vector<std::pair<std::string, int>> pairs(1);
    pairs.emplace_back(text, 7);
    pairs.emplace_back(pairs[0].first, pairs[0].second);
    assert(pairs[0].first == text && pairs[1].first == text);
    assert(pairs[0].second == 7 && pairs[1].second == 7);

    learning::Vector<Counted> counted;
    constexpr int count = 100;
    for (int i = 0; i < count; ++i) {
        counted.emplace_back(i);
    }
    assert(counted.size() == count);
    assert(Counted::moves < 2 * count);
    for (std::size_t i = 0; i < counted.size(); ++i) {
        assert(counted[i].value == static_cast<int>(i));
    }
}
