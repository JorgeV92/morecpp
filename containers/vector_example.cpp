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
}  // Both vectors release their storage automatically here.
