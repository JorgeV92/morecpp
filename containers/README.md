# Building a vector


Read `vector.hpp`, then `vector_example.cpp`.

`size` counts constructed elements. `capacity` counts how many elements fit in
the allocated storage. An empty vector can therefore own memory:

```text
Vector<Item>(4):  [ storage ][ storage ][ storage ][ storage ]
                 size = 0, capacity = 4; no Item objects constructed
```

The current invariants (conditions every operation must preserve) are:

- `size_ <= capacity_`.
- Exactly the first `size_` slots contain live elements; the rest are storage.
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
  `delete[]` on this allocation. Stage 2 destroys constructed elements before
  deallocating their storage.
- **No copying or moving yet:** copying the pointer would create two owners and
  a double deallocation. A defaulted move would also just copy this raw pointer.
  We explicitly disable all four operations until we implement ownership rules.
- **A template in a header:** callers need the definitions available when the
  compiler instantiates `Vector<int>` or `Vector<Item>`.
- **`explicit`, `const`, and `noexcept`:** a number cannot implicitly become a
  vector; queries cannot change it and do not throw. Allocation can throw, so
  the capacity constructor is not `noexcept`. If allocation fails, construction
  fails without acquiring storage that needs cleanup.

## constructing, accessing, and destroying elements

`emplace_back(42)` constructs a `T` directly in the next unused slot and returns
a reference to it. After two insertions, the storage looks like this:

```text
Vector<Item>(4):  [ Item(43) ][ Item(7) ][ storage ][ storage ]
                 size = 2, capacity = 4
```

- **Construction starts an object's lifetime:** `allocator_traits::construct`
  constructs the element in already allocated storage. Assigning to an unused
  slot would require an object to exist there already.
- **Perfect forwarding:** `Args&&...` accepts constructor arguments as a parameter
  pack. `std::forward<Args>(args)...` preserves whether each argument was an
  lvalue or rvalue, allowing constructors to accept references or move-only
  values. Unconditionally using `std::move` could move from a caller's lvalue.
- **Update size after success:** if an element constructor throws, `size_` stays
  unchanged. That slot can be tried again, and the vector must not destroy an
  element whose construction failed. Side effects performed by the constructor
  itself, such as modifying a referenced argument, are not rolled back.
- **Full capacity:** insertion throws `std::length_error` before attempting
  construction. A default-constructed vector has zero capacity, so insertion
  also throws there. Automatic growth comes in stage 5.
- **Reference access:** `operator[]` returns `T&` for a mutable vector and
  `const T&` for a const vector, avoiding a copy. It is unchecked: the caller
  must supply `index < size()`. Accessing unused capacity is invalid.
- **Destruction before deallocation:** the destructor walks the live elements
  in reverse insertion order using `allocator_traits::destroy`, then frees the
  storage. This also releases resources owned by elements, such as a
  `unique_ptr`. Element destructors must not throw.

Each insertion is O(1) in vector bookkeeping, plus the cost of constructing `T`.
Access is O(1); destruction calls one destructor per live element. Successful
insertions at this stage do not relocate existing elements, so their references
remain valid until those elements are removed or the vector is destroyed.

## Removing elements while keeping storage

`pop_back()` destroys the last element. `clear()` destroys all elements. Both
leave capacity unchanged, so later insertions reuse the same allocation:

```text
after insertion:  [ Item(43) ][ Item(7) ][ storage ][ storage ]  size = 2
after pop_back:   [ Item(43) ][ storage ][ storage ][ storage ]  size = 1
after clear:      [ storage  ][ storage ][ storage ][ storage ]  size = 0
after reuse:      [ Item(99) ][ storage ][ storage ][ storage ]  size = 1
                  capacity = 4 throughout
```

- **Destroy, then reuse:** `pop_back` decrements `size_`, making it the index of
  the old last element, and calls `allocator_traits::destroy` on that slot.
  Updating the count alone would leave an object alive and could leak resources
  it owns. A subsequent insertion constructs a new object in the unused slot.
- **Empty removal:** our `pop_back` throws `std::out_of_range` before decrementing
  an empty vector's size. This prevents unsigned underflow and invalid access.
  This is a learning API choice: C++17 `std::vector::pop_back` requires a
  nonempty vector and does not provide this exception check.
- **One destruction path:** `clear` calls `pop_back` until empty, in reverse
  insertion order. The vector destructor now calls `clear` before deallocating,
  sharing the same logic. Calling `clear` on an empty vector does nothing.
- **`noexcept`:** `clear` never calls `pop_back` on an empty vector, so that
  exception cannot occur inside its loop. As before, element destructors must
  not throw. A throwing destructor would terminate the program during `clear`.
- **Object resources versus vector storage:** destroying a `unique_ptr` element
  releases its owned object immediately. The vector's allocation remains until
  the vector is destroyed. Retaining capacity avoids repeated allocations when
  filling and clearing a vector, but keeps that memory reserved.
- **Reference validity:** `pop_back` invalidates references to the removed
  element; references to surviving elements stay valid. `clear` invalidates all
  element references. Retained storage does not keep removed objects alive.

`pop_back` takes O(1) bookkeeping plus one element's destruction cost. `clear`
performs one destruction per live element, so it is O(size) for constant-time
element destructors. Neither operation allocates or deallocates vector storage.

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
after insertion: size=2, capacity=4, values=43, 7
after pop_back: size=1, capacity=4, first=43
after clear: size=0, capacity=4, empty=true
after reuse: size=1, capacity=4, first=99
```

AddressSanitizer and UndefinedBehaviorSanitizer help catch memory errors as we
add operations. They do not prove the implementation correct.

Run the focused lifetime and failure checks (keep assertions enabled):

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined containers/vector_test.cpp -o build/vector_test
./build/vector_test
```

The checks cover throwing construction, reusing a failed slot, full and zero
capacity, destruction during exception unwinding, const access, and forwarding
a move-only value. Removal checks cover the destroyed element's identity,
surviving references, empty removal, repeated clearing, storage reuse, and
releasing resources owned by elements. Successful execution produces no output.

### Review 

1. Why is `reserved.empty()` true before insertion even though it owns storage?
2. Why can we allocate space for `Item` without a default constructor?
3. What would happen if two vectors owned the same `data_` pointer?
4. Why must `++size_` come after element construction?
5. Why is `std::forward` used instead of always calling `std::move`?
6. Why is accessing index 2 invalid when size is 2 and capacity is 4?
7. Why must `pop_back` call a destructor instead of only reducing `size_`?
8. What memory does `clear` release for a vector of `unique_ptr` elements, and
   what memory does it keep?
9. Why can you keep a reference to the first element after removing the second,
   but must stop using it after `clear`?

## Next

| Stage | Addition | C++ topic |
| --- | --- | --- |
| 4 | `reserve` | Relocation, move vs. copy, cleanup when construction throws |
| 5 | `push_back` and automatic growth | Geometric capacity, amortized cost, inserting an existing element |
| 6 | Move construction and assignment | Ownership transfer, valid moved-from state, `noexcept` |
| 7 | Copy construction and assignment | Deep copies, Rule of Five, exception guarantees |
| 8 | Iteration and a final behavior review | Const correctness, iterator invalidation, contiguous storage |
