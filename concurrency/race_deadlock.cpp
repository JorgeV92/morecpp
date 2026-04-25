#include <iostream>
#include <list>
#include <vector>
#include <mutex>
#include <algorithm>
#include <thread>
#include <cassert>
std::list<int> lst;
std::mutex mtx;

void add_to_list(int new_v) {
    std::lock_guard<std::mutex> guard(mtx);
    lst.push_back(new_v);
}

bool list_contains(int v) {
    std::lock_guard<std::mutex> guard(mtx);
    return std::find(lst.begin(), lst.end(), v) != lst.end();
}

int main() {
    add_to_list(10);
    add_to_list(20);
    add_to_list(30);

    assert(list_contains(10));
    assert(list_contains(20));
    assert(list_contains(30));
    assert(!list_contains(40));

    std::cout << "Basic tests passed.\n";

    return 0;
}