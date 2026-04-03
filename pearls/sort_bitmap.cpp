#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

constexpr uint32_t MAX_VALUE = 10'000'000;
constexpr uint32_t INPUT_COUNT = 1'000'000;

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

vector<uint32_t> read_input_file(const string& filename) {
    ifstream in(filename);
    if (!in) throw runtime_error("fail to open file");
    vector<uint32_t> data;
    data.reserve(INPUT_COUNT);
    uint32_t x;
    while (in >> x) {
        if (x >= MAX_VALUE)
            throw runtime_error("Input value out of range: " + to_string(x));
        data.push_back(x);
    }
    return data;
}

void write_output_file(const string& filename, const vector<uint32_t>& data) {
    ofstream out(filename);
    if (!out) throw runtime_error("fail to open file");
    for (uint32_t x : data) {
        out << x << '\n';
    }
}

void generate_input_file(const string& filename) {
    vector<uint32_t> all(MAX_VALUE);
    iota(all.begin(), all.end(), 0);
    mt19937 rng(42);
    shuffle(all.begin(), all.end(), rng);
    ofstream out(filename);
    if (!out) throw runtime_error("could not create file.");

    for (uint32_t i=0; i<INPUT_COUNT; i++) {
        out << all[i] << '\n';
    }
    cout << "Generated " << INPUT_COUNT << " distinct integers in " << filename << '\n';
}

vector<uint32_t> bitmap_sort(const vector<uint32_t>& input) {
    bit_vector bv(MAX_VALUE);
    for (uint32_t x : input) {
        bv.set(x);
    }
    vector<uint32_t> out;
    out.reserve(input.size());
    for (uint32_t i=0; i<MAX_VALUE; i++) {
        if (bv.test(i)) 
            out.push_back(i);
    }
    return out;
}

vector<uint32_t> std_sort_v(const vector<uint32_t>& input) {
    vector<uint32_t> out = input;
    sort(out.begin(), out.end());
    return out;
}

vector<uint32_t> my_sort(const vector<uint32_t>& input) {
    vector<uint32_t> out = bitmap_sort(input);
    return out;
}

template<class F>
pair<vector<uint32_t>,double> timed_run(F f, const vector<uint32_t>& input) {
    auto start = chrono::steady_clock::now();
    vector<uint32_t> res = f(input);
    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;
    return {std::move(res), elapsed.count()};
}

int main(int argc, char* argv[]) {

    try {
        if (argc < 3) {
            cerr << "Usage:\n";
            cerr << "  " << argv[0] << " --generate input.txt\n";
            cerr << "  " << argv[0] << " --bench input.txt\n";
            return 1;
        }

        string mode = argv[1];
        string filename = argv[2];

        if (mode == "--generate") {
            generate_input_file(filename);
            return 0;
        }

        if (mode != "--bench") {
            throw runtime_error("Unknown mode: " + mode);
        }

        vector<uint32_t> data = read_input_file(filename);
        cout << "Read " << data.size() << " integers from " << filename << "\n\n";

        auto [bitmap_result, bitmap_time] = timed_run(bitmap_sort, data);
        auto [std_result, std_time] = timed_run(std_sort_v, data);
        auto [p1_result, p1_time] = timed_run(my_sort, data);

        if (bitmap_result != std_result) {
            throw runtime_error("bitmap_sort output does not match std::sort output");
        }

        if (p1_result != std_result) {
            throw runtime_error("Problem 1 sort output does not match std::sort output");
        }

        cout << fixed << setprecision(6);
        cout << "Algorithm-only timings\n";
        cout << "----------------------\n";
        cout << "bitmap sort : " << bitmap_time << " s\n";
        cout << "std::sort   : " << std_time << " s\n";
        cout << "Problem 1   : " << p1_time << " s\n";

        write_output_file("bitmap_sorted.txt", bitmap_result);
        write_output_file("std_sorted.txt", std_result);
        write_output_file("problem1_sorted.txt", p1_result);

        cout << "\nWrote outputs to:\n";
        cout << "  bitmap_sorted.txt\n";
        cout << "  std_sorted.txt\n";
        cout << "  problem1_sorted.txt\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}