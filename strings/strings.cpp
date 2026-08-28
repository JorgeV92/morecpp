#include <iostream>
#include <vector>

#include "string.hpp"

using namespace std;

int main() {
    
    sss::Assoc values;
    std::string buf;
    while (cin>>buf) ++values[buf];
    for (auto x : values.vec) {
        std::cout << '{' << x.first << ',' << x.second << "}\n";
    }

    return 0;
}