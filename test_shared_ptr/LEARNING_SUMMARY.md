# Smart Pointer Mastery - Compressed Reference

## Assessment Results

### Aliasing & Weak Pointers: 10/10 ✅
- Perfect understanding of aliasing constructor (shares ownership, stores different pointer)
- Control block lifetime mechanics (use_count vs weak_count)
- Weak pointer observation without ownership
- lock() promotion and safety guarantees

### Ownership Patterns: Complete ✅
All concepts demonstrated with answers in test files.

### Reference Counting: Complete ✅
All concepts demonstrated with answers in test files.

### Allocation Patterns: Complete ✅
All concepts demonstrated with answers in test files.

### Structural Patterns: Complete ✅
All concepts demonstrated with answers in test files.

### Deallocation & Lifetime: Complete ✅
All concepts demonstrated with answers in test files.

---

## Quick Reference: Key Mechanisms

### Control Block Structure
```
Control Block:
├── use_count (strong refs) → Object destroyed when 0
├── weak_count (weak refs)  → Control block destroyed when both 0
├── deleter (if custom)
└── allocator (if custom)
```

### Aliasing Constructor
```cpp
std::shared_ptr<Container> owner = std::make_shared<Container>();
std::shared_ptr<Member> alias(owner, &owner->member);
// alias.get() → &owner->member (stored pointer)
// Deleter runs on Container (owned object)
// use_count shared between owner and alias
```

### enable_shared_from_this
```cpp
class Widget : public std::enable_shared_from_this<Widget> {
    std::shared_ptr<Widget> get_shared() { return shared_from_this(); }
};
// Stores weak_ptr internally
// shared_from_this() calls weak_ptr::lock()
// Throws std::bad_weak_ptr if called before shared_ptr owns object
```

### weak_ptr Mechanics
```cpp
std::weak_ptr<T> weak = shared;  // weak_count++, use_count unchanged
weak.expired();                   // true if use_count == 0
std::shared_ptr<T> locked = weak.lock();  // atomic: check + increment
// Returns nullptr if expired
```

### Move vs Copy Performance
```cpp
// Copy: 2 pointer copies + atomic increment (~10-20 cycles)
std::shared_ptr<T> p2 = p1;

// Move: 2 pointer copies + nullptr set (~3-5 cycles)
std::shared_ptr<T> p2 = std::move(p1);  // p1 becomes nullptr
```

### Parameter Passing
```cpp
void f(std::shared_ptr<T> p);           // Copy: atomic ops
void f(const std::shared_ptr<T>& p);    // No copy, can't modify ptr
void f(std::shared_ptr<T>& p);          // No copy, can modify ptr
// Prefer const& for read-only, & for modification, value for ownership transfer
```

### make_shared vs new
```cpp
// make_shared: 1 allocation (object + control block contiguous)
auto p1 = std::make_shared<T>();

// new: 2 allocations (object, then control block)
std::shared_ptr<T> p2(new T());

// Trade-off: make_shared can't free object memory until weak_ptrs die
```

### Custom Deleters
```cpp
std::shared_ptr<T> p(new T(), [](T* ptr) { 
    cleanup(ptr); 
    delete ptr; 
});
// Required for: arrays (delete[]), file handles, custom cleanup
// Cannot use make_shared with custom deleter
```

### Circular Reference Breaking
```cpp
class Parent {
    std::vector<std::shared_ptr<Child>> children;  // Parent owns children
};
class Child {
    std::weak_ptr<Parent> parent;  // Child observes parent
};
// Rule: Strong ownership down hierarchy, weak observation up
```

---

## Common Pitfalls

### ❌ Duplicate Control Blocks
```cpp
T* raw = new T();
std::shared_ptr<T> p1(raw);  // Control block 1
std::shared_ptr<T> p2(raw);  // Control block 2 → double delete!
```

### ❌ shared_from_this Before Ownership
```cpp
class Widget : public std::enable_shared_from_this<Widget> {
    Widget() {
        auto self = shared_from_this();  // Throws std::bad_weak_ptr!
    }
};
```

### ❌ Circular References
```cpp
struct Node {
    std::shared_ptr<Node> next;  // Cycle → memory leak
};
// Fix: Use weak_ptr for back-references
```

### ❌ Wrong Array Deleter
```cpp
std::shared_ptr<T> p(new T[10]);  // Uses delete, not delete[]!
// Fix: std::shared_ptr<T> p(new T[10], std::default_delete<T[]>());
```

---

## Performance Guidelines

### When to Use make_shared
- ✅ Default choice (faster, single allocation)
- ✅ No custom deleter needed
- ✅ Object size is small-medium
- ❌ Large objects with long-lived weak_ptrs (delays memory release)

### When to Use new + shared_ptr
- ✅ Custom deleter required
- ✅ Large objects with weak_ptrs (allows early memory release)
- ✅ Private constructor (factory pattern)

### Passing Strategies
- **By value**: Ownership transfer, async operations
- **By const&**: Read-only access, no ownership change (fastest)
- **By &**: Need to modify the shared_ptr itself
- **Never**: Raw pointer from .get() for long-term storage

---

## Observable Signals for Debugging

### EventLog Patterns
```cpp
// Expected: ctor, dtor (object created and destroyed)
// Leak: ctor only (destructor never called)
// Double delete: ctor, dtor, dtor (destructor called twice)
```

### Valgrind Detection
```bash
valgrind --leak-check=full --show-leak-kinds=all ./program
# Look for: allocations from shared_ptr/weak_ptr constructors
# Control block leaks: ~16-32 bytes "still reachable"
```

### use_count Debugging
```cpp
std::cout << "use_count: " << ptr.use_count() << "\n";
// Unexpected count → look for hidden copies, circular refs
// Count 0 → moved-from state (ptr is nullptr)
```

---

## Teaching Readiness

### Can Explain to Senior Engineers
- Control block implementation details
- Atomic operation costs and cache coherency
- Memory layout differences (make_shared vs new)
- Race conditions prevented by lock()

### Can Explain to Junior Engineers
- Ownership vs observation (shared_ptr vs weak_ptr)
- When objects are destroyed (use_count → 0)
- Why circular references leak
- Basic performance guidelines

### Can Identify in Code Review
- Unnecessary copies in hot paths
- Missing weak_ptr for circular structures
- Incorrect array deleters
- shared_from_this misuse

---

## Next Steps

All core concepts mastered. Ready for:
1. Production code review and mentoring
2. Teaching smart pointer patterns to team
3. Debugging complex ownership issues
4. Architecting systems with clear ownership semantics

**Status**: Production-ready for smart pointer usage and teaching. ✅
