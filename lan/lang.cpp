#include <iostream>
#include <vector>

#include "langclass.hpp"

using namespace std;


int main() {
    using namespace code;

    code::X var{7};

    auto user = [](code::X var, code::X* ptr) -> int {
        int x= var.mf(7);
        int y = ptr->mf(9);
    };

    

    return 0;
}