# c++ basics

Reference vs pointers:

A **reference** 

  - An alias for an existing object.
  - Must be initialized when declared.
  - Cannot be reseated to refer to another object later.
  - usually accessed like a normal variable.
  
```cpp
int x = 10;
int& ref = x;
ref = 20 // x becomes 20
```

A **pointer**:

  - stores a memory address.
  - can be null.
  - can be changed to point somewhere else.
  - must be dereferenced explicitly.

```cpp
int x = 10;
int* p p = &x;
*p = 20; // x becomes 20
```

A *reference* us usually a safer alias and cannot be null in normal use. A *pointer* is more flexible because it can be null, reseated, and used for dynamic memory.

Stack vs Heap allocation 

Stack 

- memory managed automatically.
- very fast allocation/deallocation.
- lifetime tied to scope.
- usually smaller in size.

```cpp
int x = 5 // stack 
std::string s // local object on stack
```

Heap

- memory allocated dynamically.
- lifetime controlled manually or with smart pointers.
- larger and more flexible.
- slower than stack allocation.

```cpp
int* p = new int(5); // heap
delete p;
```

Stack allocation is automatic and fast, but scoped-limited. Heap allocation is dynamic and flexible, but requires explicit lifetime management unless smart pointers are used.

What kinds of smart pointers exist? The main smart pointers in modern c++ are: 


`std::unique_ptr`

-  exclusive ownership
-  only one owner at a time
-  lightweight

`std::shared_ptr`

- shared ownership
- multiple owners allowed
- uses reference counting.

`weak_ptr`

- non-owning observer of a `shared_ptr` object.
- does not increase reference count
- helps break cyclic references

How is `unique_ptr` implemented? How does it enforce one owner?

`unique_ptr` stores a raw pointer internally and deletes it in its destructor.

It enforce unique ownership by:
- deleting the copy constructor 
- deleting the copy assigment operator
- allowing move constructor and move assigment 

For example 

```cpp
template<T> class UniquePtr {
    T* ptr;

public:
    explicit UniquePtr(T* p = nullptr) {}
    ~UniquePtr() { delete ptr; }
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    Unique(UniquePtr&& o) noexcept : ptr(o.ptr) {o.ptr = nullptr;}
    UniquePtr& operator=(UniquePtr&& o) noexcept {if (this != &o) {delete ptr; ptr = o.ptr; o.ptr = nullptr;} return *this;}
}
```

key point copying is forbidden and ownership can only be transferred with move semantics.

How does `shared_ptr` work? How is the reference counter synchronized?

`shared_ptr` uses a control block that typically stores:
- the raw pointer 
- the stong reference count
- the weak reference count
- deleter information 
  
When you copy a `shared_ptr`, the string count increases. When a `shared_ptr` is destroyed, the strong count decresaes. When the strong count reaches zero, the managed object is deleted.

An *atomic* operation is an operation on a shared variable that happens indivisibly, so other threads cannot observe it half-completed. In c++, `std::atomic` is used for this. It helps avoid *data races*  for simple shared state like counters and flags, though it does not automatically make larger program logic thread-safe.

The counters are typically updated using *atomic operations*, so increments/decrements are thread-safe across `shared_ptr` instances sharing the same control block.

The reference count management is thread-safe. Access to the actual ppinted-to object is not automatically thread-safe. 