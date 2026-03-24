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