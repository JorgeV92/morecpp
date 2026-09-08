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
- **Full capacity:** insertion now grows automatically, including from zero
  capacity. The growth implementation and its element-type requirements are
  explained below.
- **Reference access:** `operator[]` returns `T&` for a mutable vector and
  `const T&` for a const vector, avoiding a copy. It is unchecked: the caller
  must supply `index < size()`. Accessing unused capacity is invalid.
- **Destruction before deallocation:** the destructor walks the live elements
  in reverse insertion order using `allocator_traits::destroy`, then frees the
  storage. This also releases resources owned by elements, such as a
  `unique_ptr`. Element destructors must not throw.

Insertion with spare capacity is O(1) in vector bookkeeping, plus the cost of
constructing `T`. Access is O(1); destruction calls one destructor per live
element. Insertion with spare capacity preserves existing element references.
Removing an element, destroying the vector, or growing capacity invalidates
references to the affected elements.

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
  it grows or the vector is destroyed. Retaining capacity avoids repeated
  allocations when filling and clearing a vector, but keeps that memory reserved.
- **Reference validity:** `pop_back` invalidates references to the removed
  element; references to surviving elements stay valid. `clear` invalidates all
  element references. Retained storage does not keep removed objects alive.

`pop_back` takes O(1) bookkeeping plus one element's destruction cost. `clear`
performs one destruction per live element, so it is O(size) for constant-time
element destructors. Neither operation allocates or deallocates vector storage.

## Reserving more capacity

`reserve(n)` ensures room for at least `n` elements without changing size. This
implementation allocates exactly `n` slots when growing. If `n <= capacity()`,
it does nothing, including for `reserve(0)`; it never shrinks the vector.

Read the implementation in three steps:

1. **Allocate:** check the allocator's maximum element count, then allocate the
   new storage. An oversized request throws `std::length_error`; allocation
   itself may throw. Both leave the existing vector unchanged.
2. **Construct:** relocate each live element into the new allocation, tracking
   how many constructions succeeded. The original allocation remains owned by
   the vector during this work.
3. **Commit:** only after every relocation succeeds, destroy the old elements,
   free their storage, and install the new pointer and capacity. `clear()` sets
   size to zero, so restore it from the successful construction count.

```text
before reserve(8): [ Item(99) ][ storage x 3 ]   size = 1, capacity = 4
new allocation:   [ Item(99) ][ storage x 7 ]   size = 1, capacity = 8
                  destroy old element and release old allocation after success
```

### Why move or copy?

`std::move_if_noexcept` selects the argument passed to the element constructor:

| Element type | Relocation | Reason |
| --- | --- | --- |
| Has a `noexcept` move constructor | Move | Transferring resources cannot fail |
| Move may throw or is unavailable, copy available | Copy | Keep original values available if a later copy fails |
| Move may throw, no copy available | Move | This is the available way to relocate the elements |

Calling `reserve` requires `T` to be move-constructible or copy-constructible.
It constructs new elements; it does not use assignment or require a default
constructor. Moving an element is separate from moving the vector itself,
whose move operations are still disabled.

### What if relocation throws?

The catch block destroys only the successfully constructed new elements, frees
the new allocation, and rethrows the original exception. It never destroys the
slot whose construction failed. The vector keeps its original allocation, size,
and capacity.

When copying is selected, failure preserves the original elements' values,
assuming the copy constructor does not modify its source. This gives the
**strong exception guarantee** for the vector's state. As with `emplace_back`,
external side effects performed by an element constructor cannot be rolled back.
When moving cannot throw, relocation completes after allocation succeeds.

For a move-only type whose move can throw, earlier moves may have changed source
elements; even the move that throws can modify its source. Cleanup preserves
the vector's storage and object lifetimes, but cannot promise the old values.
`vector_reserve_test.cpp` demonstrates this limit with a deliberately throwing
move constructor. Element destructors must still not throw.

A successful growth invalidates all element pointers and references because the
old objects and allocation are gone. A no-op preserves them. A failed relocation
retains the old objects, with the value caveat above. For a vector of
`unique_ptr`, relocating the pointers does not relocate the objects they own.

Growth performs O(size) element constructions and destructions, plus allocation
cost; a no-op is O(1). Both allocations coexist temporarily, so peak storage
includes the old and new capacities. Use `reserve` when you know the upcoming
element count and want to avoid reallocating during those insertions.

## Appending and automatic growth

`push_back(const T&)` copies an existing value. `push_back(T&&)` forwards an
rvalue so the element constructor can move from it. Both delegate to
`emplace_back`, which returns a reference to the appended element; `push_back`
returns nothing. `emplace_back` also accepts constructor arguments directly.

When full, this vector starts with capacity one and then doubles:

```text
append count:  0  1  2  3  4  5
capacity:      0  1  2  4  4  8
```

An initial capacity of three grows to six. Near the allocator's limit, growth
caps the new capacity at that limit. The comparison happens before multiplying,
so doubling cannot overflow. A full vector already at the limit throws
`std::length_error` before allocating or constructing anything.

### Construct the appended element first

Consider `v.push_back(v[0])` when the vector is full. Its argument refers to an
object in the old allocation. Calling `reserve` first would invalidate that
reference before the appended copy could use it.

The private `grow_and_emplace` function keeps the old allocation alive and
constructs the appended element in its final slot **before** relocating the
existing elements with `std::move_if_noexcept`:

```text
old allocation:    [ A ][ B ]
new, append first: [ storage ][ storage ][ copy of A ][ storage ]
new, relocate:     [    A    ][    B    ][ copy of A ][ storage ]
                   release old allocation only after all construction succeeds
```

The same ordering supports `v.emplace_back(v[0])` and constructor arguments
that refer to an existing element's members. If an element stores borrowed
pointers into the old allocation, growth still invalidates those pointers.

On failure, cleanup must account for two parts: the successfully relocated
prefix and the separately appended element. `relocated` counts the prefix;
`appended` becomes true only after the new element's constructor succeeds.
Destroy those objects, free the new allocation, and rethrow. Commit the new
pointer, size, and capacity only after everything succeeds.

This keeps size, capacity, and the old allocation unchanged when construction
fails. Original values also remain unchanged when their relocation uses copies
that preserve their sources. A throwing move-only element can still change
source values, as described for `reserve`. Side effects from constructor
arguments are not rolled back either, including explicitly moving from `v[i]`.
For example, `v.push_back(std::move(v[0]))` transfers its value to the appended
element and leaves the original element moved from on success.

### Type requirements and cost

Growth requires a move-constructible or copy-constructible element type. Earlier
examples used an immovable `Tracked` type, so `if constexpr` keeps the growth
code from being instantiated for that type. Such elements can still be emplaced
within preallocated capacity; insertion when full throws `std::length_error`.

A growing insertion performs O(size) relocations. Across N appends starting
empty, doubling relocates fewer than 2N existing elements, so appending is
**amortized O(1)** when element construction and relocation have constant cost.
This describes total work across a sequence; an individual growth can still be
expensive. Reserving capacity in advance can avoid those pauses. Explicitly
calling `reserve(size() + 1)` before each append would discard this doubling
policy and can cause quadratic relocation work.

Successful growth invalidates all element references and pointers. An insertion
that fits existing capacity preserves references to existing elements. These
rules apply equally to `push_back` and `emplace_back`.

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
after reserve: size=1, capacity=8, first=99
after another insertion: size=2, capacity=8, values=99, 100
automatic growth: size=3, capacity=4, values=5, 5, 7
```

AddressSanitizer and UndefinedBehaviorSanitizer help catch memory errors as we
add operations. They do not prove the implementation correct.

Run the focused lifetime and failure checks (keep assertions enabled):

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined containers/vector_test.cpp -o build/vector_test
./build/vector_test
```

The checks cover throwing construction, reusing a failed slot, immovable types
at full and zero capacity, destruction during exception unwinding, const access,
and moving from an existing element during growth. Removal checks cover the
destroyed element's identity, surviving references, empty removal, repeated
clearing, storage reuse, and releasing resources owned by elements. Successful
execution produces no output.

Run the reserve and relocation failure checks:

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined containers/vector_reserve_test.cpp -o build/vector_reserve_test
./build/vector_reserve_test
```

These cover empty and nonempty growth, no-op requests, allocator limits, copy
failure at each element, successful copying, moving unique ownership, and the
failure limits of a throwing move-only type. Growing insertion checks also fail
the appended construction and each relocation to verify cleanup of both parts.
Successful execution is silent.

Run the automatic growth checks:

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined containers/vector_growth_test.cpp -o build/vector_growth_test
./build/vector_growth_test
```

These cover growth from empty, doubling, reference stability without growth,
insertion from existing elements and their members, and a linear bound on total
relocations over repeated appends. Successful execution is silent.

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
10. Why must the old allocation remain alive until relocation finishes?
11. Why might copying provide better exception safety than moving?
12. What can `reserve` preserve if a move-only element's move constructor throws?
13. Why must growing insertion construct the appended element before relocation?
14. Why does cleanup need both a relocation count and an appended-element flag?
15. How can appending be amortized O(1) when a single insertion can cost O(size)?

## Next

| Stage | Addition | C++ topic |
| --- | --- | --- |
| 6 | Move construction and assignment | Ownership transfer, valid moved-from state, `noexcept` |
| 7 | Copy construction and assignment | Deep copies, Rule of Five, exception guarantees |
| 8 | Iteration and a final behavior review | Const correctness, iterator invalidation, contiguous storage |

The companion [unique pointer exercise](unique_ptr.md) starts with ownership of
a single object and follows its own small stages.
