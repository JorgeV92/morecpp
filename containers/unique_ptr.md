# Building a unique pointer

Read `unique_ptr.hpp`, then `unique_ptr_example.cpp`. This C++17 exercise starts
with **stage 1: owning one object**. Review this stage before adding ownership
transfer. The repo also has a fuller [SimpleUniquePtr example](../impl/simple_unique_ptr.cpp).

## Ownership and lifetime

```text
UniquePtr<Item> owner  --->  Item(42), allocated with new
owner leaves scope    --->  delete calls ~Item() and frees its storage
```

The invariant is simple: `ptr_` is null, or this wrapper is the sole owner of an
object that can be safely destroyed with `delete ptr_`.

- **RAII:** the raw-pointer constructor takes ownership of an already constructed
  object. The destructor deletes it on normal scope exit and exception unwinding.
  The wrapper itself does not allocate; `new Item(42)` runs before its constructor.
- **Empty ownership:** default construction sets `ptr_` to null. `delete nullptr`
  does nothing, so the destructor needs no branch.
- **Single objects:** adopt a pointer from a matching single-object `new`
  expression exactly once. Stack objects, `new[]` arrays, and pointers already
  owned elsewhere do not meet this contract. The class rejects array and
  non-object template types; its destructor requires `T` to be complete.
- **No copying:** a shallow copy would make two wrappers delete the same object.
  Copy construction and assignment stay disabled for exclusive ownership.
  Moving is temporarily disabled too: a defaulted move would also copy the raw
  pointer. A later stage will transfer it and empty the source.
- **`explicit` and `noexcept`:** the constructor requires deliberate adoption,
  such as `UniquePtr<Item> owner(new Item(42))`. Storing a pointer cannot throw,
  even though the preceding `new` expression can. The object's destructor must
  not throw; an exception escaping the wrapper's `noexcept` destructor terminates
  the program.

This pointer owns an already live object. `delete` both destroys that object
and deallocates its storage. The vector owns allocator storage and manages
construction, destruction, and storage separately.

## Access and borrowing

| Operation | Meaning |
| --- | --- |
| `if (owner)` | Test whether an object is owned; explicit conversion also allows `!owner` |
| `owner.get()` | Borrow the raw pointer without giving up ownership |
| `*owner` | Access the object by reference |
| `owner->member` | Access a member through the stored pointer |

Dereferencing requires a non-null pointer; access is unchecked. A borrowed
pointer does not extend the object's lifetime. Do not delete it or create a
second owner from it. Once the owner destroys the object, the borrowed pointer
is dangling.

`const UniquePtr<Item>` makes the owner const, while the `Item` remains mutable.
`UniquePtr<const Item>` provides read-only access to the object. This is why the
const accessors return `T*` and `T&`, preserving the const qualification of `T`.

Construction and access take O(1). Cleanup performs one `delete`, including the
cost of the object's destructor and deallocation.

## Run it

From the repository root:

```sh
mkdir -p build
c++ -std=c++17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined containers/unique_ptr_example.cpp -o build/unique_ptr_example
./build/unique_ptr_example
```

Expected output:

```text
empty owns an object: false
Item constructed: 42
owned value: 42
Item destroyed: 43
owner scope finished
```

Run the ownership checks with assertions enabled:

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined containers/unique_ptr_test.cpp -o build/unique_ptr_test
./build/unique_ptr_test
```

The checks cover null ownership, borrowed access, const behavior, exactly one
destruction, cleanup during exception unwinding, and the disabled copy/move
operations. Successful execution is silent.

## Review

1. Why does `get()` leave deletion to the owner?
2. Why would defaulted copying or moving cause a double deletion?
3. Why can `const UniquePtr<Item>` modify an `Item`, while `UniquePtr<const Item>` cannot?
4. Why does this destructor use `delete`, while the vector uses allocator operations?

## Next small stages

| Stage | Addition | C++ topic |
| --- | --- | --- |
| 2 | Move construction and assignment | Ownership transfer, `noexcept`, self-move, cleaning up the destination |
| 3 | `release`, `reset`, and `swap` | Giving up ownership versus destroying an object |
| 4 | `make_unique` helper | Perfect forwarding and construction inside a factory |
| 5 | Use with the learning vector | Moving ownership during element relocation |

Array ownership and custom deleters are future extensions to review separately.
