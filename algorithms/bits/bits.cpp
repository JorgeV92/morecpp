#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
using namespace std;

auto has_alt_bits(int n) -> bool {
    // Time O(logn)
    int prev = -1;
    while (n) {
        int cur = n & 1;
        if (prev == cur) return false;
        prev = cur;
        n >>= 1;
    }
    return true;
}

int main() {

    return 0;
}