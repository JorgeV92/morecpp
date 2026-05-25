# pragma once 

#include <algorithm>
#include <iterator>
#include <iostream>
#include <vector>

template<typename RAI>
void heap_sort(RAI begin, RAI end) {
  std::make_heap(begin, end);
  std::sort_heap(begin, end);
}

template<typename T> 
struct __heapsort {
  std::vector<T> heap;
  __heapsort(std::vector<T>& a) { heap = a; __heap_sort_(); }
  
  void __shift_down_(int i, int mx) {
    int i_big{}, c1{}, c2{};
    while (i < mx) {
      i_big = i;
      c1 = (2*i) + 1;
      c2 = c1 + 1;
      if (c1 < mx && heap[c1] > heap[i_big]) 
        i_big = c1;
      if (c2 < mx && heap[c2]>heap[i_big])
          i_big = c2;
      if (i_big == i) return;
      std::swap(heap[i], heap[i_big]);
      i = i_big;
    }
  }

  void __to_heap_() {
    int i = (heap.size()/2) - 1;
    while (i >= 0) {
      __shift_down_(i, heap.size());
      --i;
    }
  }
  
  void __heap_sort_() {
    __to_heap_();
    int end = heap.size() - 1;
    while (end > 0) {
      std::swap(heap[0], heap[end]);
      __shift_down_(0, end);
      --end;
    }
  }
};
