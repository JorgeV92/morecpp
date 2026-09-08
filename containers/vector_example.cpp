#include "vector.hpp"

#include <iostream>

struct Item {
    explicit Item(int value) : value(value) {}
    int value;
};

int main() {
    learning::Vector<int> empty;
    // Item has no default constructor. Allocating its storage still works.
    learning::Vector<Item> reserved(4);

    std::cout << std::boolalpha;
    std::cout << "default: size=" << empty.size()
              << ", capacity=" << empty.capacity()
              << ", empty=" << empty.empty() << '\n';
    std::cout << "reserved: size=" << reserved.size()
              << ", capacity=" << reserved.capacity()
              << ", empty=" << reserved.empty() << '\n';

    Item& first = reserved.emplace_back(42);  // Constructs Item(42) in its slot.
    first.value = 43;  // The returned reference refers to the stored element.
    reserved.emplace_back(7);

    const auto& view = reserved;
    std::cout << "after insertion: size=" << view.size()
              << ", capacity=" << view.capacity()
              << ", values=" << view[0].value << ", " << view[1].value << '\n';
}  // Destroy the two Items, then release the storage.
