#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>

class bit_vector {
public:
    bit_vector(std::size_t n) : nbits(n), v((n + 63) / 64) {}

    void set(std::size_t i) {
        v[i/64] |= 1ULL << (i%64);
    }

    void clear(std::size_t i) {
        check_index(i);
        v[i/64] &= ~(1ULL << (i%64));
    }

    bool test(std::size_t i) const {
        check_index(i);
        return (v[i/64] & (1ULL << (i%64))) != 0;
    }

    std::size_t size() const {
        return nbits;
    }
private:
    std::size_t nbits;
    std::vector<std::uint64_t> v;
    void check_index(std::size_t i) const {
        if (i >= nbits) {
            throw std::out_of_range("bit index out of range.");
        }
    }
};

int main() {
    const int MAXV = 10000000;
    bit_vector bv(MAXV+1);  
    int x;
    while (std::cin >> x) {
        if (x < 0 || x > MAXV) {
            std::cerr << "value out of range: " << x << '\n';
            continue;
        }
        bv.set(x);
    }

    for (int i = 0; i <= MAXV; i++) {
        if (bv.test(i)) {
            std::cout << i << ' ';
        }
    }

    return 0;
}