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