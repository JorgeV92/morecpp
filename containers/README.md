# Building a vector

Read `vector.hpp`, then `vector_example.cpp`.

`size` counts constructed elements. `capacity` counts how many elements fit in
the allocated storage. An empty vector can therefore own memory:

```text
Vector<Item>(4):  [ storage ][ storage ][ storage ][ storage ]
                 size = 0, capacity = 4; no Item objects constructed
```

invariants (conditions every operation must preserve) are:

- `size_ == 0`, because element construction has not been added.
- `size_ <= capacity_`.
- `data_ == nullptr` exactly when `capacity_ == 0`.
- Each allocation has one owner and is deallocated once.

### Why these implementation choices?

- **RAII:** construction acquires storage; destruction releases it when the
  vector leaves scope, including during exception unwinding. Callers do not
  manually free it.
- **`std::allocator<T>`:** allocates storage with the size and alignment needed
  for `T`, without calling element constructors. `new T[capacity]` would also
  default-initialize every slot, mixing capacity with element lifetime and
  requiring a default constructor. The example's `Item` has none.
- **Matching allocation and deallocation:** the destructor passes the original
  pointer and capacity to `deallocate`. It skips the null pointer. Do not use
  `delete[]` on this allocation. Once we construct elements, we must destroy
  them before deallocating their storage.
- **No copying or moving yet:** copying the pointer would create two owners and
  a double deallocation. A defaulted move would also just copy this raw pointer.
  We explicitly disable all four operations until we implement ownership rules.
- **A template in a header:** callers need the definitions available when the
  compiler instantiates `Vector<int>` or `Vector<Item>`.
- **`explicit`, `const`, and `noexcept`:** a number cannot implicitly become a
  vector; queries cannot change it and do not throw. Allocation can throw, so
  the capacity constructor is not `noexcept`. If allocation fails, construction
  fails without acquiring storage that needs cleanup.

### Run it

From the repository root:

```sh
mkdir -p build
c++ -std=c++17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined containers/vector_example.cpp -o build/vector_example
./build/vector_example
```

Expected output:

```text
default: size=0, capacity=0, empty=true
reserved: size=0, capacity=4, empty=true
```

AddressSanitizer and UndefinedBehaviorSanitizer help catch memory errors as we
add operations. They do not prove the implementation correct.

### Review b

1. Why is `reserved.empty()` true even though it owns storage?
2. Why can we allocate space for `Item` without a default constructor?
3. What would happen if two vectors owned the same `data_` pointer?

## Next 

| Stage | Addition | C++ topic |
| --- | --- | --- |
| 2 | `emplace_back` within capacity, element access, destruction | Object lifetime, perfect forwarding; reject insertion when full |
| 3 | `pop_back` and `clear` | Destroying elements while retaining storage |
| 4 | `reserve` | Relocation, move vs. copy, cleanup when construction throws |
| 5 | `push_back` and automatic growth | Geometric capacity, amortized cost, inserting an existing element |
| 6 | Move construction and assignment | Ownership transfer, valid moved-from state, `noexcept` |
| 7 | Copy construction and assignment | Deep copies, Rule of Five, exception guarantees |
| 8 | Iteration and a final behavior review | Const correctness, iterator invalidation, contiguous storage |
