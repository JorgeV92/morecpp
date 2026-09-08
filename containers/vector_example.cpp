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

    reserved.pop_back();  // Destroys Item(7); the first element stays alive.
    std::cout << "after pop_back: size=" << view.size()
              << ", capacity=" << view.capacity()
              << ", first=" << first.value << '\n';

    reserved.clear();  // Destroys Item(43); do not use first after this point.
    std::cout << "after clear: size=" << view.size()
              << ", capacity=" << view.capacity()
              << ", empty=" << view.empty() << '\n';

    reserved.emplace_back(99);  // Reuses the storage retained by clear().
    std::cout << "after reuse: size=" << view.size()
              << ", capacity=" << view.capacity()
              << ", first=" << view[0].value << '\n';

    reserved.reserve(8);  // Relocates Item(99); size stays at one.
    std::cout << "after reserve: size=" << view.size()
              << ", capacity=" << view.capacity()
              << ", first=" << view[0].value << '\n';

    reserved.reserve(2);  // A smaller request does not shrink the allocation.
    reserved.emplace_back(100);
    std::cout << "after another insertion: size=" << view.size()
              << ", capacity=" << view.capacity()
              << ", values=" << view[0].value << ", " << view[1].value << '\n';

    learning::Vector<Item> growing;
    growing.emplace_back(5);  // First insertion allocates capacity one.
    growing.push_back(growing[0]);  // Copy an existing element while growing.
    growing.push_back(Item(7));
    std::cout << "automatic growth: size=" << growing.size()
              << ", capacity=" << growing.capacity()
              << ", values=" << growing[0].value << ", " << growing[1].value
              << ", " << growing[2].value << '\n';
}  // Destroy the remaining Items, then release the storage.
