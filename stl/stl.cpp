#include <iostream>
#include <vector>
#include <algorithm>        // General algorithsm
#include <cstdlib>          // bsearch(), qsort()
using namespace std;

namespace code {

template<typename T>
class initializer_list {
public:
    using value_type = T;
    using reference = const T&;
    using const_reference = const T&;
    using size_type = size_t;
    using const_iterator = const T*;

    initializer_list() noexcept;

    size_t size() const noexcept; 
    const T* begin() const noexcept;
    const T* end() const noexcept;
};

template<typename T>
    const T* begin(initializer_list<T> lst) noexcept { return lst.begin(); }
template<typename T>
    const T* end(initializer_list<T> lst) noexcept {. return lst.end(); }


/**
 * my vec
 * 
 * A is defualted to std::allocator<T> for operator new() and operator delete()
 * when acquiring memory.
 * 
 */
template<typename T, typename A = allocator<T>>
class my_vec {

};

} // namespace code;

int main() {

    int arr[]{8, 2, 42, 67, 7, 6};
    size_t n = sizeof(arr) / sizeof(arr[0]);

    auto com = [](const void* a, const void* b) -> int {
        int x = *(const int*)a;
        int y = *(const int*)b;
        if (x < y) return -1;
        if (x > y) return 1;
        return 0;
    };

    qsort(arr, n, sizeof(arr[0]), com);

    for (int x : arr) {
        cout << x << " ";
    }
    cout << '\n';



    return 0;
}