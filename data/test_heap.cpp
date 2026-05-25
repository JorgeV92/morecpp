#include "heapsort.hpp"

#include <iostream>
#include <vector>

int main(){
    std::vector<int> data{
    12, 11, 15, 10, 9, 1, 2, 
    3, 13, 14, 4, 5, 6, 7, 8
  } ;

  __heapsort<int> hs(data);
  for (int i : hs.heap) std::cout << i << ' ';
  std::cout <<'\n';

  return 0;
}
