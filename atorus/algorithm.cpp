#include <iostream>
#include <vector>
#include <string>
using namespace std;

template<typename C, typename V>
vector<typename C::iterator> find_all(C& c, V v) {
    vector<typename C::iterator> res;
    for (auto p = c.begin(); p != c.end(); p++) {
        if (*p == v)
            res.push_back(p);
    }
    return res;
}

int main() {

    return 0;
}