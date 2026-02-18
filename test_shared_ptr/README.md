# `shared_ptr` Learning Test Suite

## Purpose
Teach `shared_ptr` mechanics through hands-on fill-in exercises. Focus: reference counting, ownership, lifetime, aliasing, custom deleters, `weak_ptr`, cycles, anti-patterns.

## Learning Method
1. Read test to understand scenario
2. Study helper classes (fully implemented)
3. Fill in TODOs with smart pointer operations
4. Run test to validate against assertions
5. Debug failures by reasoning about ownership/lifetime
6. Query for guidance when stuck (Socratic questions, not answers)

Tests provide: complete helpers, skeleton logic, clear TODOs, assertions, dummy returns.

## Instrumentation
Logs track: constructors, destructors, copies, moves, custom deleters, event sequence. Use `EventLog::instance().dump()` to inspect.

## Recommended Path

### 0. Fill-In Exercises (`tests/test_exercises_fill_in.cpp`)
7 guided exercises: `enable_shared_from_this`, `weak_ptr` cache, observer pattern, custom deleters, aliasing, copy-on-write, breaking cycles.

### 1. Reference Counting (`tests/test_reference_counting.cpp`)
Control block, allocation patterns (`new` vs `make_shared`), copy vs move, aliasing, self-assignment safety.

**Key Insight:** Control block is invisible complexity. Lifetime determined by global reference counting state.

### 2. Ownership Patterns (`tests/test_ownership_patterns.cpp`)
`enable_shared_from_this`, factory pattern, parameter passing, return by value, `weak_ptr` mechanics, move semantics.

**Key Insight:** Control block has dual lifetime—object (use_count) and control block (use_count + weak_count).

### 3. Allocation Patterns (`tests/test_allocation_patterns.cpp`)
`make_shared` vs `new`, custom deleters, arrays, deleter type erasure, aliasing with deleters.

### 4. Deallocation & Lifetime (`tests/test_deallocation_lifetime.cpp`)
Destruction tracking, custom deleters, shared ownership timing, aliasing lifetime, multiple resets.

### 5. Structural Patterns (`tests/test_structural_patterns.cpp`)
Circular references (leaks), breaking cycles with `weak_ptr`, parent-child, graphs, self-references.

### 6. Aliasing & `weak_ptr` (`tests/test_aliasing_weak.cpp`)
Aliasing constructor, `weak_ptr` creation/assignment, `lock()`/`expired()`, aliasing with `weak_ptr`, multiple weak refs.

### 7. Collection Patterns (`tests/test_collection_patterns.cpp`)
`weak_ptr` cache, observer pattern, registry pattern (classes provided, implement test logic).

### 8. Scope & Lifetime (`tests/test_scope_lifetime_patterns.cpp`)
`weak_ptr::lock()`, lambda capture (`weak_ptr` vs `shared_ptr`), lifetime extension, conditional locking, mutable captures.

### 9. Self-Reference (`tests/test_self_reference_patterns.cpp`)
Async operations, cancellable callbacks, timers, event emitters (classes provided, implement test logic).

### 10. Polymorphism (`tests/test_polymorphism_patterns.cpp`)
`dynamic_pointer_cast`, `static_pointer_cast`, `const_pointer_cast`, Pimpl idiom, polymorphic containers, downcasting.

### 11. Singleton & Registry (`tests/test_singleton_registry.cpp`)
Meyers singleton, thread-safe singleton, global registry (classes provided, implement test logic).

### 12. Interop (`tests/test_interop_patterns.cpp`)
Custom deleters for C resources, safe `get()` usage, RAII wrappers, bridging C/C++ ownership, array wrapping.

### 13. Conditional Lifetime (`tests/test_conditional_lifetime.cpp`)
Lazy initialization, copy-on-write, deferred construction, resource pool, optional resource (classes provided, implement test logic).

### 14. Anti-Patterns (`tests/test_anti_patterns.cpp`)
Globals, unnecessary pass-by-value, multiple control blocks from raw pointer, self-reference without `enable_shared_from_this`, cycles without `weak_ptr`.

### 15. Smart Pointer Contrast (`tests/test_smart_pointer_contrast.cpp`)
`unique_ptr` vs `shared_ptr`, move semantics, custom deleters, overhead, aliasing.

### 16. Compile-Fail Tests (`compile_fail_tests/`)
See `compile_fail_tests/README.md`. Focus: copying `unique_ptr`, multiple `shared_ptr` from raw pointer, stack objects, direct `weak_ptr` dereference, non-copyable types.

## Building & Running
```bash
mkdir build && cd build
cmake ..
make
ctest --verbose                  # All tests
./test_reference_counting        # Individual suite
./test_reference_counting --gtest_filter=ReferenceCountingTest.BasicCreationAndDestruction  # Specific test
```

## Instrumentation Format
```
[index] EventType(name)::operation [id=N]
```
Example: `[0] Tracked(A)::ctor [id=1]`

Use to trace: lifetime, ownership, destruction order.

## Key Questions
As you implement TODOs, reason about:
- **Ownership**: Who owns the object?
- **Reference count**: What should `use_count()` be?
- **Lifetime**: When will object be destroyed?
- **Deleter**: Which deleter, when invoked?
- **Aliasing**: Does it extend lifetime?
- **Cycles**: Are there cyclic references? Will they leak?
- **Weak pointers**: When use `weak_ptr` vs `shared_ptr`?

## Socratic Learning
- **Do**: Ask about mechanics ("What happens to use_count when...?")
- **Do**: Ask for hints when stuck ("Which smart pointer type?")
- **Do**: Request clarification ("Why does this use weak_ptr?")
- **Don't**: Ask for complete solutions
- **Don't**: Skip reasoning

Goal: build intuition through guided discovery.

## Exclusions
Does NOT cover: multithreading, thread safety, performance benchmarking, `std::atomic<shared_ptr>`, lock-free patterns.

Focus: core ownership and lifetime semantics in single-threaded contexts.

## Final Notes
- **Implement, don't copy**: Build muscle memory
- **Test incrementally**: One TODO at a time
- **Instrumentation is truth**: Trust logs over intuition
- **Failure is learning**: Failing tests reveal mental model gaps
- **Ask precise questions**: Formulate specific, testable hypotheses
