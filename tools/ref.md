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

### How does `shared_ptr` work? How is the reference counter synchronized?

`shared_ptr` uses a control block that typically stores:
- the raw pointer 
- the stong reference count
- the weak reference count
- deleter information 
  
When you copy a `shared_ptr`, the string count increases. When a `shared_ptr` is destroyed, the strong count decresaes. When the strong count reaches zero, the managed object is deleted.

An *atomic* operation is an operation on a shared variable that happens indivisibly, so other threads cannot observe it half-completed. In c++, `std::atomic` is used for this. It helps avoid *data races*  for simple shared state like counters and flags, though it does not automatically make larger program logic thread-safe.

The counters are typically updated using *atomic operations*, so increments/decrements are thread-safe across `shared_ptr` instances sharing the same control block.

The reference count management is thread-safe. Access to the actual ppinted-to object is not automatically thread-safe. 

### Can we copy `unique_ptr` or pass it from one object to another? 

You cannot copy a `unique_ptr`.

```cpp
std::unique_ptr<int> p1 = std::make__unique<int>(5);
// std::unique_ptr<int> p2 = p1; // error
```

You can move it.

```cpp
std::unique_ptr<int> p2 = std::move(p1); // ok
```

After the move `p2` owns the object and `p1` becomes `NULL`. You can also pass it by move, if tranferring ownership and by reference, if not transferring ownership.

```cpp
void takeOwnership(std::unique_ptr<int> p);
void useWithoutOwning(const std::unique_ptr<int>& p);
```

### What are lvalues and rvalues? 

**lvalue** has a persistent location in memory and can appear on the left-hand side of assigment. 

```cpp
int x = 5; // x is an lvalue
```

**rvalue** temporary value object and usually does not have a persistent named locations. 

```cpp
int y = x + 1; // x +1 is an rvalue
```

The following are some examples:

```cpp
int a = 10;
int& lref = a       // lvalue reference
int&& rref = 20;    // rvalue reference
```

### What are `std::move` and `std::forward`?

`std::move` casts an object to an rvalue and signals that its resources may be moved from. 

```cpp
std::string a = "hello";
std::string b = std::move(a);
```

It does not move by itself. It only enables move semantics.

`std::forward` use in templates to preserve whether an argument was originally an lvalue or rvalue. 

```cpp
template<T> void wrapper(T&& arg) { foo(std::forward<T>(arg)); }
```

`std::move` unconditionally treats something as movable. `std::forward` conditionally preserves value category in templates.

# OOP 

### Ways to access private fields of class?

Normally, private members cna only be accessed by, member functions of the same class, friends, somethimes nested classes depending on access rules.

Ways: public getter/setter methods, friend functions, friend class.

```cpp
class A {
private:
    int x = 42;
    friend class B;
};
```

### Can a class inherit multiple classes? 

Yes, c++ supports multiple inheritance.

```cpp
class A{};
class B{};
class C : public A, public B {};
```

This can introduce ambiguity, especially with the diamond problem.

The diamond problem happens in multiple inheritance when a class inherits from two classes that both inherit from the same base class.

It forms a shape like this:

```bash
    A
   / \
  B   C
   \ /
    D
```

`B` inherits from `A`, `C` inherits from `A`, `D` inherits from both `B` and `C`.

### Is a static field initialized in the class constructor? 

No. A static data member belongs to class, not the each object. It is not initialized by the constructor of each instance.

```cpp
class A {
public: 
    static int count;
};

int A::count= 0;
```

Static memebers are class-level, not object-level. They are initialized separately from instance construction. 


### Can an exception be thrown in a constructor or destructor? How to prevent that? 

**Constructor** Yes, a constructor can throw. If it throws, the object is considered not fully constructed, and already-constructed subobjects are cleaned up. 

**Destructor** can throw, but it is strongly discouraged. If a destructor throws during stack unwinding from another exception, the program calls `std::terminate()`. 

Destructors should be `noexcept` and should not throw and handle errors inside the destructor. 

```cpp
~MyClass() noexcept {
    try {
        // cleanup
    } catch (...) {
        //
    }
}
```

### What are virtual methods?

A virtual method enables **runtime polymorphism**. 

```cpp
class Base {
public:
    virtual void speak() { std::cout << "Base\n"; }
};

class Derived : public Base {
public:
    void speak() override { std:: cout << "Derived\n"; }
};
```

If called through a base pointer/reference, the derived version is choosen at runtime.

```cpp
Base* b = new Derived();
b->speak(); // Derived
```

Only the base destructor may run, causing resource leaks or worse. If a class has any virtual functions, it often should have a virtual destructor. 

### Difference between abstract class and interface? 

**Abstract class** has at least one pure virtual function and may also contain data members and implemented methods. 

```cpp
class Shape {
public:
    virtual double area() const = 0;
    virtual ~Shape() = default;
};
```

**Interface**: c++ has no special `interface` keyword. We have classes that conatins only pure virtual methods, has no data members, and acts as a contract. 

```cpp
class IPrinter {
public:
    virtual void print() = 0;
    virtual ~IPrinter() = default;
};
```

In c++, an interface is usually just a pure abstract class.

### Can a constructor be virtual? 

No. Virtual dispatch needs an already-constructed object and a valid setup. Constructors build the object, so virtual behavior does not make sense there. But destructors can and often should be virtual. 

## How is `const` used for class methods? 

A `const` member function promises not to modify the observable state of the object.

```cpp
class A {
    int x;
public:
    int getX() const { return x; }
};
```

Inside a `const` method, `this` is treated as `const A*` and you cannot modify non-mutuable members. 

```cpp
const A a;
a.getX();   // okay only if getX() is const
```

### How to protect an object form copying? 

Delete the copy constructor and copy assigment operator.

```cpp
class A {
public:
    A() = default;
    A(const A&) = delete;
    A& operator=(const &A) = delete;
};
```

# STL Containers 

**Vector vs list** 

`std::vector` 
- dynamic array 
- contiguous memory 
- fast random access: `O(1)`
- fast at end insertion 
- inserting/removing in middle expensive

`std::list`
- doubly-linked list 
- non-contiguous memory 
- no fast random access
- cheap insert/erase if iterator is known
- worse cache localoty

Prefer `vector` by default and use `list` only when frequent middle insertion/erase with stable iterators is truly needed.

**Map vs unordered_map**

`std::map`
- usually implemented as a balanced bst, commonly red-black tree
- keys are ordered
- operations are `O(log n)`

`std::unordered_map`
- hash table 
- keys are not ordered
- average `O(1)` lookup/insert/erase
- worst case `O(n)`

Use `map` when you need sorted order, you need range queries, and deterministic ordering matters.
Use `unordered_map` when you want faster average lookup and ordering does not matter.


