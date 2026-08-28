#include <cstddef>
#include <iostream>
#include <string>
#include <utility>


/**

* @brief A simplified exclusive-ownership smart pointer similar to
* ```
     std::unique_ptr<T>.
  ```
*
* SimpleUniquePtr manages the lifetime of one dynamically allocated object.
* It stores a raw pointer internally and automatically deletes the pointed-to
* object when the SimpleUniquePtr is destroyed.
*
* The class follows the RAII principle:
*
* ```
  Resource Acquisition Is Initialization
  ```
*
* The dynamically allocated object is acquired when the SimpleUniquePtr takes
* ownership of a raw pointer. The object is automatically released by the
* SimpleUniquePtr destructor.
*
* Basic ownership example:
*
* ```
  SimpleUniquePtr<Widget> ptr(new Widget());
  ```
*
* Memory representation:
*
* ```
  Stack                          Heap
  ```
*
* ```
  +------------------+           +------------------+
  ```
* ```
  | ptr              |           | Widget           |
  ```
* ```
  |                  |           |                  |
  ```
* ```
  | ptr_ ------------+---------->| dynamically      |
  ```
* ```
  +------------------+           | allocated object |
  ```
* ```
                                 +------------------+
  ```
*
* ptr is the exclusive owner of the heap-allocated Widget. When ptr leaves
* scope, its destructor executes:
*
* ```
  delete ptr_;
  ```
*
* This destroys the Widget and releases its memory.
*
* ---
* EXCLUSIVE OWNERSHIP
* ---
*
* At most one SimpleUniquePtr should own a given dynamically allocated object.
*
* The following ownership state is valid:
*
* ```
  +-------------+
  ```
* ```
  | first       |
  ```
* ```
  | ptr_ -------+----------+
  ```
* ```
  +-------------+          |
  ```
* ```
                           v
  ```
* ```
                     +------------+
  ```
* ```
                     | Widget     |
  ```
* ```
                     +------------+
  ```
*
* The following state must not be allowed:
*
* ```
  +-------------+
  ```
* ```
  | first       |
  ```
* ```
  | ptr_ -------+----------+
  ```
* ```
  +-------------+          |
  ```
* ```
                           v
  ```
* ```
                     +------------+
  ```
* ```
                     | Widget     |
  ```
* ```
                           ^
  ```
* ```
  +-------------+          |
  ```
* ```
  | second      |          |
  ```
* ```
  | ptr_ -------+----------+
  ```
* ```
  +-------------+
  ```
*
* If first and second both believed that they owned the same Widget, both
* destructors would execute:
*
* ```
  delete ptr_;
  ```
*
* This would attempt to delete the same object twice, resulting in undefined
* behavior.
*
* Copy construction and copy assignment are therefore disabled:
*
* ```
  SimpleUniquePtr(const SimpleUniquePtr&) = delete;
  ```
* ```
  SimpleUniquePtr& operator=(const SimpleUniquePtr&) = delete;
  ```
*
* The following code does not compile:
*
* ```
  SimpleUniquePtr<Widget> first(new Widget());
  ```
* ```
  SimpleUniquePtr<Widget> second = first;
  ```
*
* ---
* MOVE SEMANTICS
* ---
*
* Although ownership cannot be copied, it can be transferred using move
* semantics:
*
* ```
  SimpleUniquePtr<Widget> second = std::move(first);
  ```
*
* Before the move:
*
* ```
  first.ptr_
  ```
* ```
      |
  ```
* ```
      v
  ```
* ```
  +----------+
  ```
* ```
  | Widget   |
  ```
* ```
  +----------+
  ```
*
* ```
  second does not yet own an object.
  ```
*
* After the move:
*
* ```
  first.ptr_  ----> nullptr
  ```
*
* ```
  second.ptr_
  ```
* ```
      |
  ```
* ```
      v
  ```
* ```
  +----------+
  ```
* ```
  | Widget   |
  ```
* ```
  +----------+
  ```
*
* The object itself is not moved. Only the raw pointer and its ownership are
* transferred.
*
* The source pointer is set to nullptr so that its destructor does not delete
* the transferred object.
*
* ---
* INTERNAL REPRESENTATION
* ---
*
* The class contains one data member:
*
* ```
  T* ptr_ = nullptr;
  ```
*
* ptr_ has two possible states:
*
* 1. Empty state:
*
* ```
     ptr_ == nullptr
  ```
*
* ```
     +------------------+
  ```
* ```
     | SimpleUniquePtr  |
  ```
* ```
     | ptr_ = nullptr   |
  ```
* ```
     +------------------+
  ```
*
* 2. Owning state:
*
* ```
     ptr_ != nullptr
  ```
*
* ```
     +------------------+          +------------+
  ```
* ```
     | SimpleUniquePtr  |          | T object   |
  ```
* ```
     | ptr_ ------------+--------->|            |
  ```
* ```
     +------------------+          +------------+
  ```
*
* ---
* CONSTRUCTOR
* ---
*
* The default constructor creates an empty smart pointer:
*
* ```
  SimpleUniquePtr() noexcept = default;
  ```
*
* Result:
*
* ```
  ptr_ == nullptr
  ```
*
* The raw-pointer constructor takes ownership of an existing dynamically
* allocated object:
*
* ```
  explicit SimpleUniquePtr(T* ptr) noexcept
  ```
* ```
      : ptr_(ptr) {}
  ```
*
* Example:
*
* ```
  SimpleUniquePtr<Widget> ptr(new Widget());
  ```
*
* Once ownership has been transferred to SimpleUniquePtr, the raw pointer
* should not be deleted manually.
*
* Incorrect:
*
* ```
  Widget* raw = new Widget();
  ```
* ```
  SimpleUniquePtr<Widget> ptr(raw);
  ```
*
* ```
  delete raw; // Incorrect: ptr still believes it owns the object.
  ```
*
* When ptr is later destroyed, it would try to delete the object again.
*
* ---
* DESTRUCTOR
* ---
*
* The destructor releases the owned object:
*
* ```
  ~SimpleUniquePtr() {
  ```
* ```
      delete ptr_;
  ```
* ```
  }
  ```
*
* Calling delete on nullptr is safe, so no explicit null check is necessary:
*
* ```
  delete nullptr; // Valid and has no effect.
  ```
*
* Therefore, the same destructor handles both an empty pointer and an owning
* pointer.
*
* Scope example:
*
* ```
  {
  ```
* ```
      SimpleUniquePtr<Widget> ptr(new Widget());
  ```
*
* ```
      // Widget exists here.
  ```
* ```
  }
  ```
*
* ```
  // ptr is destroyed here.
  ```
* ```
  // Widget is automatically deleted here.
  ```
*
* This automatic cleanup also occurs during early returns and exception stack
* unwinding.
*
* ---
* MOVE CONSTRUCTOR
* ---
*
* The move constructor transfers ownership from another SimpleUniquePtr:
*
* ```
  SimpleUniquePtr(SimpleUniquePtr&& other) noexcept
  ```
* ```
      : ptr_(other.release()) {}
  ```
*
* other.release():
*
* * Returns other's raw pointer.
* * Sets other.ptr_ to nullptr.
* * Does not delete the object.
*
* Before:
*
* ```
  other.ptr_
  ```
* ```
      |
  ```
* ```
      v
  ```
* ```
  +----------+
  ```
* ```
  | T object |
  ```
* ```
  +----------+
  ```
*
* After:
*
* ```
  other.ptr_ ----> nullptr
  ```
*
* ```
  this->ptr_
  ```
* ```
      |
  ```
* ```
      v
  ```
* ```
  +----------+
  ```
* ```
  | T object |
  ```
* ```
  +----------+
  ```
*
* Ownership has changed, but the T object remains at the same memory address.
*
* ---
* MOVE ASSIGNMENT
* ---
*
* Move assignment transfers ownership into an already existing
* SimpleUniquePtr:
*
* ```
  SimpleUniquePtr& operator=(SimpleUniquePtr&& other) noexcept {
  ```
* ```
      if (this != &other) {
  ```
* ```
          reset(other.release());
  ```
* ```
      }
  ```
*
* ```
      return *this;
  ```
* ```
  }
  ```
*
* Example:
*
* ```
  auto first  = make_simple_unique<Widget>("first");
  ```
* ```
  auto second = make_simple_unique<Widget>("second");
  ```
*
* Before:
*
* ```
  first.ptr_                           second.ptr_
  ```
* ```
      |                                    |
  ```
* ```
      v                                    v
  ```
* ```
  +----------------+                 +----------------+
  ```
* ```
  | Widget "first" |                 | Widget "second"|
  ```
* ```
  +----------------+                 +----------------+
  ```
*
* Operation:
*
* ```
  second = std::move(first);
  ```
*
* Steps:
*
* 1. first.release() returns the pointer to Widget "first".
* 2. first.ptr_ becomes nullptr.
* 3. second.reset(...) deletes Widget "second".
* 4. second.ptr_ takes ownership of Widget "first".
*
* After:
*
* ```
  first.ptr_ ----> nullptr
  ```
*
* ```
  second.ptr_
  ```
* ```
      |
  ```
* ```
      v
  ```
* ```
  +----------------+
  ```
* ```
  | Widget "first" |
  ```
* ```
  +----------------+
  ```
*
* Widget "second" has been destroyed.
*
* The self-assignment check:
*
* ```
  if (this != &other)
  ```
*
* protects against an unusual operation such as:
*
* ```
  ptr = std::move(ptr);
  ```
*
* ---
* get()
* ---
*
* get() returns the stored raw pointer without changing ownership:
*
* ```
  T* get() const noexcept {
  ```
* ```
      return ptr_;
  ```
* ```
  }
  ```
*
* Example:
*
* ```
  Widget* raw = ptr.get();
  ```
*
* Ownership remains:
*
* ```
  ptr
  ```
* ```
   |
  ```
* ```
   v
  ```
* ```
  Widget
  ```
*
* raw is only a non-owning observer:
*
* ```
  raw
  ```
* ```
   |
  ```
* ```
   +----------> same Widget
  ```
*
* The caller must not normally delete the pointer returned by get().
*
* Incorrect:
*
* ```
  delete ptr.get();
  ```
*
* ptr would still contain the now-invalid pointer and would later attempt a
* second deletion.
*
* get() is useful when passing the object to an API that accepts a raw pointer
* but does not take ownership:
*
* ```
  void inspect(const Widget* widget);
  ```
*
* ```
  inspect(ptr.get());
  ```
*
* ---
* operator*()
* ---
*
* operator* returns a reference to the owned object:
*
* ```
  T& operator*() const {
  ```
* ```
      return *ptr_;
  ```
* ```
  }
  ```
*
* Example:
*
* ```
  SimpleUniquePtr<int> ptr(new int(42));
  ```
*
* ```
  std::cout << *ptr; // Prints 42.
  ```
*
* Relationship:
*
* ```
  ptr       is a SimpleUniquePtr<T>
  ```
* ```
  ptr.get() is a T*
  ```
* ```
  *ptr      is a T&
  ```
*
* Dereferencing an empty SimpleUniquePtr is undefined behavior:
*
* ```
  SimpleUniquePtr<int> ptr;
  ```
* ```
  std::cout << *ptr; // Invalid.
  ```
*
* The caller must ensure that the pointer contains an object.
*
* ---
* operator->()
* ---
*
* operator-> returns the stored pointer so that members of T can be accessed:
*
* ```
  T* operator->() const noexcept {
  ```
* ```
      return ptr_;
  ```
* ```
  }
  ```
*
* Example:
*
* ```
  SimpleUniquePtr<Widget> ptr(new Widget());
  ```
*
* ```
  ptr->run();
  ```
*
* This is conceptually equivalent to:
*
* ```
  ptr.get()->run();
  ```
*
* or:
*
* ```
  (*ptr).run();
  ```
*
* ---
* BOOLEAN CONVERSION
* ---
*
* The explicit boolean conversion reports whether an object is owned:
*
* ```
  explicit operator bool() const noexcept {
  ```
* ```
      return ptr_ != nullptr;
  ```
* ```
  }
  ```
*
* This allows:
*
* ```
  if (ptr) {
  ```
* ```
      ptr->run();
  ```
* ```
  }
  ```
*
* and:
*
* ```
  if (!ptr) {
  ```
* ```
      // ptr is empty.
  ```
* ```
  }
  ```
*
* The conversion is explicit to prevent unintended conversions to integers or
* other unrelated types.
*
* ---
* release()
* ---
*
* release() gives up ownership without deleting the object:
*
* ```
  T* release() noexcept {
  ```
* ```
      return std::exchange(ptr_, nullptr);
  ```
* ```
  }
  ```
*
* std::exchange(ptr_, nullptr) conceptually performs:
*
* ```
  T* old_pointer = ptr_;
  ```
* ```
  ptr_ = nullptr;
  ```
* ```
  return old_pointer;
  ```
*
* Before:
*
* ```
  ptr.ptr_
  ```
* ```
      |
  ```
* ```
      v
  ```
* ```
  +----------+
  ```
* ```
  | T object |
  ```
* ```
  +----------+
  ```
*
* Operation:
*
* ```
  T* raw = ptr.release();
  ```
*
* After:
*
* ```
  ptr.ptr_ ----> nullptr
  ```
*
* ```
  raw
  ```
* ```
   |
  ```
* ```
   v
  ```
* ```
  +----------+
  ```
* ```
  | T object |
  ```
* ```
  +----------+
  ```
*
* The T object is still alive, but SimpleUniquePtr no longer owns it.
*
* The caller is now responsible for either:
*
* * Deleting the raw pointer:
*
* ```
    delete raw;
  ```
*
* * Transferring it to another owner:
*
* ```
    SimpleUniquePtr<T> another(raw);
  ```
*
* Failure to do either causes a memory leak.
*
* release() and get() are different:
*
* ```
  get():
  ```
* ```
      Returns the pointer but keeps ownership.
  ```
*
* ```
  release():
  ```
* ```
      Returns the pointer and gives up ownership.
  ```
*
* ---
* reset()
* ---
*
* reset() deletes the currently owned object and optionally takes ownership of
* a new raw pointer:
*
* ```
  void reset(T* new_ptr = nullptr) noexcept {
  ```
* ```
      if (ptr_ == new_ptr) {
  ```
* ```
          return;
  ```
* ```
      }
  ```
*
* ```
      delete ptr_;
  ```
* ```
      ptr_ = new_ptr;
  ```
* ```
  }
  ```
*
* Example:
*
* ```
  SimpleUniquePtr<Widget> ptr(new Widget("old"));
  ```
*
* Current ownership:
*
* ```
  ptr
  ```
* ```
   |
  ```
* ```
   v
  ```
* ```
  +--------------+
  ```
* ```
  | Widget "old" |
  ```
* ```
  +--------------+
  ```
*
* Operation:
*
* ```
  ptr.reset(new Widget("new"));
  ```
*
* Result:
*
* * Widget "old" is destroyed.
* * ptr takes ownership of Widget "new".
*
* ```
  ptr
  ```
* ```
   |
  ```
* ```
   v
  ```
* ```
  +--------------+
  ```
* ```
  | Widget "new" |
  ```
* ```
  +--------------+
  ```
*
* Calling reset with no argument empties the pointer:
*
* ```
  ptr.reset();
  ```
*
* Result:
*
* ```
  ptr.ptr_ == nullptr
  ```
*
* The currently owned object is destroyed.
*
* The equality check:
*
* ```
  if (ptr_ == new_ptr)
  ```
*
* prevents accidentally deleting an object and then storing the same now-
* dangling pointer again.
*
* ---
* swap()
* ---
*
* swap() exchanges ownership between two SimpleUniquePtr objects:
*
* ```
  void swap(SimpleUniquePtr& other) noexcept {
  ```
* ```
      std::swap(ptr_, other.ptr_);
  ```
* ```
  }
  ```
*
* Before:
*
* ```
  first.ptr_                         second.ptr_
  ```
* ```
      |                                  |
  ```
* ```
      v                                  v
  ```
* ```
  +----------+                       +----------+
  ```
* ```
  | Object A |                       | Object B |
  ```
* ```
  +----------+                       +----------+
  ```
*
* After:
*
* ```
  first.ptr_                         second.ptr_
  ```
* ```
      |                                  |
  ```
* ```
      v                                  v
  ```
* ```
  +----------+                       +----------+
  ```
* ```
  | Object B |                       | Object A |
  ```
* ```
  +----------+                       +----------+
  ```
*
* The objects themselves are not copied or moved. Only their pointer ownership
* is exchanged.
*
* ---
* make_simple_unique()
* ---
*
* A helper function can construct an object and immediately place it under
* smart-pointer ownership:
*
* ```
  template <typename T, typename... Args>
  ```
* ```
  SimpleUniquePtr<T> make_simple_unique(Args&&... args) {
  ```
* ```
      return SimpleUniquePtr<T>(
  ```
* ```
          new T(std::forward<Args>(args)...)
  ```
* ```
      );
  ```
* ```
  }
  ```
*
* Example:
*
* ```
  auto ptr = make_simple_unique<Widget>("example", 42);
  ```
*
* This is preferable to exposing a raw new expression:
*
* ```
  SimpleUniquePtr<Widget> ptr(new Widget("example", 42));
  ```
*
* std::forward preserves the value category of each constructor argument so
* that lvalues can be copied and rvalues can be moved when appropriate.
*
* ---
* CLASS INVARIANTS
* ---
*
* The following conditions should always hold:
*
* * ptr_ is either nullptr or points to one dynamically allocated T object.
* * If ptr_ is not nullptr, this SimpleUniquePtr is its exclusive owner.
* * Two SimpleUniquePtr objects must not independently own the same pointer.
* * A moved-from SimpleUniquePtr is empty and safe to destroy.
* * Destroying an owning SimpleUniquePtr destroys the managed object exactly
* once.
* * Calling get() does not change ownership.
* * Calling release() removes ownership without deleting the object.
* * Calling reset() deletes the currently owned object before adopting another.
*
* ---
* TIME AND SPACE COMPLEXITY
* ---
*
* * Default construction: O(1)
* * Raw-pointer construction: O(1)
* * Move construction: O(1)
* * Move assignment: O(1), plus destruction of the previously owned object
* * get(): O(1)
* * operator*(): O(1)
* * operator->(): O(1)
* * release(): O(1)
* * reset(): O(1), plus destruction of the previously owned object
* * swap(): O(1)
* * Destruction: O(1), plus the cost of T's destructor
*
* The SimpleUniquePtr itself uses O(1) additional storage because it contains
* only one raw pointer.
*
*
* @tparam T
* ```
  The type of object owned by the smart pointer.
  ```
*
* @warning
* The raw pointer passed to this class must refer to an object that may safely
* be destroyed using:
*
* ```
  delete ptr;
  ```
*
* A pointer to an array allocated with new[] must not be managed by this
* simplified implementation because arrays require delete[].
*
* @note
* A moved-from SimpleUniquePtr remains valid but empty. It may be destroyed,
* assigned a new pointer, reset, or moved into again.
  */

template<typename T>
class SimpleUniquePtr {
public:
  SimpleUniquePtr() noexcept = default;

  explicit SimpleUniquePtr(T* ptr) noexcept 
    : ptr_(ptr) {}

  ~SimpleUniquePtr() {
    delete ptr_;
  }

  SimpleUniquePtr(const SimpleUniquePtr&) = delete;

  SimpleUniquePtr& operator=(const SimpleUniquePtr&) = delete;

  SimpleUniquePtr(SimpleUniquePtr&& o) noexcept 
    : ptr_(o.release()) {}

  SimpleUniquePtr& operator=(SimpleUniquePtr&& o) noexcept {
    if (this != o) {
      reset(o.release());
    }
    return *this;
  }

  T* get() const noexcept {
    return ptr_;
  }

  T& operator*() const {
    return *ptr_;
  }

  T* operator->() const noexcept {
    return ptr_;
  }

  explicit operator bool() const noexcept {
    return ptr_ != nullptr;
  }

  T* release() noexcept {
    return std::exchange(ptr_, nullptr);
  }

  void reset(T* new_ptr = nullptr) noexcept {
    if (ptr_ == new_ptr) return;
    delete ptr_;
    ptr_ = new_ptr;
  }

  void swap(SimpleUniquePtr& o) noexcept {
    std::swap(ptr_, o.ptr_);
  }

private:
  T* ptr_ = nullptr;
};

template<typename T, typename... Args>
SimpleUniquePtr<T> make_simple_unique(Args&&... args) {
  return SimpleUniquePtr<T>(
    new T(std::forward<Args>(args)...)
  );
}

class Widget {
public:
    explicit Widget(std::string name) 
      : name_(std::move(name)) {
    std::cout << "Constructing " << name_ << '\n';
  }

  ~Widget() {
    std::cout << "Destroying " << name_ << '\n';
  }

  void greet() const {
    std::cout << "Hello from " << name_ << '\n';
  }

private:
  std::string name_;
};

int main() {
    std::cout << "Create first pointer\n";

    auto first = make_simple_unique<Widget>("first");

    first->greet();

    std::cout << "\nMove first into second\n";

    SimpleUniquePtr<Widget> second = std::move(first);

    std::cout << "first owns an object: "
              << std::boolalpha
              << static_cast<bool>(first)
              << '\n';

    std::cout << "second owns an object: "
              << static_cast<bool>(second)
              << '\n';

    second->greet();

    std::cout << "\nReset second with a new object\n";

    second.reset(new Widget("replacement"));

    second->greet();

    std::cout << "\nRelease ownership\n";

    Widget* raw = second.release();

    std::cout << "second owns an object: "
              << static_cast<bool>(second)
              << '\n';

    // release() does not delete the object.
    // We are now responsible for deleting it.
    delete raw;

    std::cout << "\nEnd of main\n";
}
