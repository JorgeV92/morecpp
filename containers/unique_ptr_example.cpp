#include "unique_ptr.hpp"

#include <iostream>

struct Item {
    explicit Item(int value) : value(value) {
        std::cout << "Item constructed: " << value << '\n';
    }
    ~Item() { std::cout << "Item destroyed: " << value << '\n'; }

    int value;
};

int main() {
    learning::UniquePtr<Item> empty;
    std::cout << std::boolalpha
              << "empty owns an object: " << static_cast<bool>(empty) << '\n';

    {
        learning::UniquePtr<Item> owner(new Item(42));
        if (owner) {
            std::cout << "owned value: " << owner->value << '\n';
            (*owner).value = 43;
        }
    }  // The owner's destructor deletes its Item here.

    std::cout << "owner scope finished\n";
}
