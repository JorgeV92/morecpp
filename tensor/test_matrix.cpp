#include <iostream>
#include <vector>

#include "matrix.hpp"

using namespace Matrix_impl;

void solve_random_system(size_t n) {
    Mat2d A = random_matrix(n);     // generate random Mat2d
    Vec b = random_vector(n);       // generate random Vec

    std::cout << "A = " << A << std::endl;
    std::cout << "b = " << b << std::endl;

    try {
        Vec x = classical_gaussian_elimination(A, b);
        std::cout << "classical elim solution is x = " << x << std::endl;
        Vec v = A * x;
        std::cout << "A * x = " << v << std::endl;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    // [TODO] test
    return 0;
}