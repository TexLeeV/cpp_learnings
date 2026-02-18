# Multi-Threaded Patterns Test Suite

## Overview

Comprehensive test suite covering `shared_ptr` usage in multi-threaded environments with **15 tests** demonstrating both correct patterns and anti-patterns.

**Requirements**: C++11 (uses custom latch/barrier implementations), Asio library, pthreads

---

## Test Coverage Map

### Core Patterns (Tests 1-6)

#### 1-2. **Ownership Transfer** (`asio::io_context`)
- ✅ **Safe**: `shared_from_this()` keeps object alive during async work
- ❌ **Unsafe**: Raw `this` pointer leads to use-after-free
- **Concepts**: Ownership transfer, callback lifetime, `asio::post()`

#### 3-4. **Reference Counting** (Atomic Operations)
- Atomic use_count increment across threads
- Parallel copy vs move operations
- **Concepts**: Control block atomicity, memory barriers, performance

#### 5-6. **Weak Callbacks** (Timer Expiration)
- Weak callback expires before execution
- Shared callback keeps object alive
- **Concepts**: `weak_ptr::lock()`, timer callbacks, lifetime control

### Advanced Patterns (Tests 7-10)

#### 7. **Thread-Safe Cache** (`weak_ptr` registry)
- Concurrent cache access with weak_ptr
- **Concepts**: Cache invalidation, weak observation, mutex protection

#### 8. **Producer-Consumer** (`std::queue`)
- Ownership transfer through queue
- **Concepts**: `std::condition_variable`, ownership semantics, blocking operations

#### 9. **Destruction Order** (Cross-thread destruction)
- Object destroyed in different thread than created
- **Concepts**: Thread-local destruction, RAII across threads

#### 10. **Thread Pool** (`asio::thread_pool`)
- Work distribution with ownership transfer
- **Concepts**: Thread pool vs io_context, work stealing, completion tracking

### Edge Cases & Anti-Patterns (Tests 11-15)

#### 11. **Race Conditions** (Data races)
- ❌ `shared_ptr` doesn't protect contained data
- **Concepts**: Atomicity guarantees, what shared_ptr protects vs doesn't

#### 12. **Thread-Safe Singleton** (`std::call_once`)
- Concurrent singleton initialization
- **Concepts**: `std::once_flag`, initialization safety, static storage

#### 13-14. **Circular References** (Memory leaks)
- ❌ Circular reference causes leak across threads
- ✅ `weak_ptr` breaks cycle
- **Concepts**: Reference cycles, weak observation, cleanup

#### 15. **Timer Callbacks** (Real-world async I/O)
- Cancellable timers with `asio::steady_timer`
- Weak callbacks for safe cancellation
- **Concepts**: `async_wait()`, cancellation, error handling

#### 16. **Connection Management** (Production pattern)
- Connection pool with weak_ptr registry
- Async read operations with lifetime management
- **Concepts**: Connection lifetime, manager patterns, weak registry

#### 17. **std::bind vs Lambda** (Comparison)
- Both approaches for member function binding
- **Concepts**: Capture semantics, readability, performance

---

## Key Concepts Demonstrated

### Threading Primitives

```cpp
CountDownLatch start_latch(4);       // Synchronize thread start (C++11 compatible)
Barrier sync_point(4);                // Synchronize at checkpoint (C++11 compatible)
std::mutex mutex_;                    // Protect shared state
std::condition_variable cv_;          // Wait/notify pattern
std::atomic<int> counter{0};          // Lock-free counter
```

### Asio Patterns

```cpp
// io_context: Single-threaded event loop
asio::io_context io;
asio::post(io, callback);
io.run();

// thread_pool: Multi-threaded work distribution
asio::thread_pool pool(4);
asio::post(pool, callback);
pool.join();

// steady_timer: Async timer
asio::steady_timer timer(io);
timer.expires_after(std::chrono::milliseconds(100));
timer.async_wait(callback);
```

### Ownership Patterns

```cpp
// Safe: shared_from_this() in callback
asio::post(io, [self = shared_from_this()]() {
    self->do_work();  // Object kept alive
});

// Safe: weak_ptr for cancellable callbacks
std::weak_ptr<T> weak_self = shared_from_this();
asio::post(io, [weak_self]() {
    if (auto self = weak_self.lock()) {
        self->do_work();  // Execute if still alive
    }
});

// Unsafe: Raw this pointer
asio::post(io, [this]() {
    this->do_work();  // Dangling pointer if object destroyed!
});
```

### std::bind Usage

```cpp
// Bind with shared_from_this()
asio::post(io, std::bind(&Class::method, shared_from_this(), arg1, arg2));

// Equivalent lambda
asio::post(io, [self = shared_from_this(), arg1, arg2]() {
    self->method(arg1, arg2);
});
```

---

## Question Categories

Each test includes **uncompressed Q/A/R format** with questions in these categories:

### 1. **Mechanism Questions**
- "What happens to use_count when...?"
- "Why does weak_ptr::lock() return...?"
- "What atomic operation occurs in the control block...?"

### 2. **Safety Questions**
- "What is the danger of using raw this?"
- "Why is it safe/unsafe to...?"
- "What prevents race conditions...?"

### 3. **Lifetime Questions**
- "When is the object destroyed?"
- "What keeps the object alive during...?"
- "In which thread does destruction occur?"

### 4. **Design Questions**
- "Why use weak_ptr instead of shared_ptr?"
- "What is the advantage of...?"
- "Why is this pattern ideal for...?"

### 5. **Observable Signals**
- "What does ThreadSafeEventLog record?"
- "How many times is X logged?"
- "What is the final use_count?"

---

## ThreadSafeEventLog Infrastructure

Thread-safe wrapper around EventLog for debugging:

```cpp
ThreadSafeEventLog::instance().record("Event message");
ThreadSafeEventLog::instance().clear();
auto events = ThreadSafeEventLog::instance().events();
size_t count = ThreadSafeEventLog::instance().count("pattern");
```

**Use cases**:
- Verify callback execution order
- Detect race conditions
- Track object lifetime across threads
- Debug async operation flow

---

## Building Concepts Progressively

### Foundation (Tests 1-4)
Basic multi-threading with shared_ptr:
- Ownership transfer
- Atomic reference counting
- Safe vs unsafe patterns

### Intermediate (Tests 5-10)
Practical async patterns:
- Weak callbacks
- Caches and registries
- Producer-consumer
- Thread pools

### Advanced (Tests 11-15)
Production patterns and pitfalls:
- Race condition detection
- Singleton initialization
- Circular references
- Real-world async I/O
- Connection management

### Mastery (Tests 16-17)
Complete patterns:
- Connection pools
- Timer-based systems
- std::bind vs lambda trade-offs

---

## Common Pitfalls Demonstrated

### ❌ Anti-Pattern 1: Raw `this` in Callbacks
```cpp
asio::post(io, [this]() {
    this->method();  // DANGER: Object might be destroyed!
});
```

### ❌ Anti-Pattern 2: Circular References
```cpp
node1->set_next(node2);  // shared_ptr
node2->set_next(node1);  // shared_ptr → Memory leak!
```

### ❌ Anti-Pattern 3: Assuming Data Race Protection
```cpp
std::shared_ptr<Data> shared = ...;
// Multiple threads:
shared->counter++;  // RACE CONDITION! shared_ptr doesn't protect this!
```

### ✅ Correct Pattern 1: shared_from_this()
```cpp
asio::post(io, [self = shared_from_this()]() {
    self->method();  // Safe: Object kept alive
});
```

### ✅ Correct Pattern 2: Weak Callbacks
```cpp
std::weak_ptr<T> weak_self = shared_from_this();
asio::post(io, [weak_self]() {
    if (auto self = weak_self.lock()) {
        self->method();  // Execute only if alive
    }
});
```

### ✅ Correct Pattern 3: Break Cycles with weak_ptr
```cpp
node1->set_next(node2);       // shared_ptr (ownership)
node2->set_weak_next(node1);  // weak_ptr (observation)
```

---

## Performance Considerations

### Copy vs Move in Multi-threaded Context

**Copy** (atomic increment):
```cpp
std::shared_ptr<T> copy = shared;  // Atomic increment (~10-20 cycles)
```

**Move** (no atomic ops):
```cpp
std::shared_ptr<T> moved = std::move(temp);  // Just pointer copy (~3-5 cycles)
```

**Impact**: Move is ~2-3x faster due to avoiding atomic operations and memory barriers.

### Cache Coherency

When multiple threads access the same control block:
- Atomic operations require cache line synchronization
- Frequent copy/destroy causes cache thrashing
- Prefer move semantics in hot paths

---

## Real-World Applications

### 1. **Async Server** (Connection Management)
- Connection pool with weak_ptr registry
- Async read/write operations
- Automatic cleanup when connections close

### 2. **Timer Service** (Cancellable Callbacks)
- Weak callbacks for timers
- Safe cancellation without races
- Automatic cleanup on expiration

### 3. **Work Queue** (Producer-Consumer)
- Thread-safe queue with shared_ptr
- Ownership transfer between threads
- Clear lifetime semantics

### 4. **Resource Cache** (Weak Registry)
- Cache with automatic expiration
- No reference cycles
- Thread-safe access

---

## Testing Strategy

### Deterministic Synchronization
```cpp
CountDownLatch start_latch(N);   // All threads start together (C++11 compatible)
Barrier sync_point(N);            // Checkpoint synchronization (C++11 compatible)
```

### Observable Signals
```cpp
ThreadSafeEventLog::instance().record("Event");
EXPECT_EQ(count("pattern"), expected);
```

### Use Count Verification
```cpp
std::atomic<long> use_count_in_callback{0};
// In callback:
use_count_in_callback = ptr.use_count();
// After join:
EXPECT_EQ(use_count_in_callback, expected);
```

---

## Next Steps

After completing this test suite, you will have mastered:

1. ✅ Safe async callback patterns
2. ✅ Weak pointer usage in multi-threaded context
3. ✅ Thread-safe caches and registries
4. ✅ Producer-consumer with ownership transfer
5. ✅ Destruction order across threads
6. ✅ Race condition detection
7. ✅ Singleton thread-safety
8. ✅ Circular reference prevention
9. ✅ Real-world async I/O patterns
10. ✅ std::bind vs lambda trade-offs

**Status**: Ready for production multi-threaded C++ development with shared_ptr! 🚀
