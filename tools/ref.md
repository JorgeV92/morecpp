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

