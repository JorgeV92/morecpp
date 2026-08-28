#include <iostream>

#include <vector>

using namespace std;

auto main() -> int {

  auto fun = [](int x) -> int {
    return x * x;
  };
 
  cout << fun(4) << '\n';
  

  return 0;
}
