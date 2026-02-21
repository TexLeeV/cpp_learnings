# Move Semantics & Perfect Forwarding Test Suite

## Overview

This test suite provides a comprehensive, hands-on learning path for C++ move semantics and perfect forwarding using the **Socratic method**. Each test file contains:

- **Questions (Q:)** that probe your understanding
- **Answer spaces (A:)** for you to fill in
- **Response/feedback (R:)** provided after you complete the exercise
- **TODO markers** for code you need to implement

## Module Structure

### 1. Rvalue References (`test_rvalue_references.cpp`)
**Topics Covered:**
- Lvalue vs rvalue distinction
- Rvalue references (`T&&`) syntax
- `std::move` basics
- Temporary object lifetime
- Value categories in expressions
- Move semantics in containers

**Key Questions:**
- What is the difference between an lvalue and an rvalue?
- When does the compiler automatically treat an object as an rvalue?
- How does `std::move` enable move semantics?

**Estimated Time:** 4 hours

### 2. Move Assignment & Rule of Five (`test_move_assignment.cpp`)
**Topics Covered:**
- Move assignment operator implementation
- Rule of Five (destructor, copy ctor, move ctor, copy assign, move assign)
- Self-move assignment safety
- Move semantics with raw pointers
- Moved-from state guarantees
- Exception safety with `noexcept`

**Key Questions:**
- What's the difference between move constructor and move assignment?
- Why should move operations be marked `noexcept`?
- What happens to an object after it's been moved from?

**Estimated Time:** 3 hours

### 3. std::move vs std::forward (`test_std_move.cpp`)
**Topics Covered:**
- `std::move` as an unconditional cast to rvalue
- `std::forward` for preserving value categories
- Return Value Optimization (RVO)
- Moving from const objects
- Reference collapsing rules
- When NOT to use `std::move`

**Key Questions:**
- What does `std::move` actually do?
- When should you use `std::move` vs `std::forward`?
- Why is `return std::move(local)` an anti-pattern?

**Estimated Time:** 3 hours

### 4. Perfect Forwarding (`test_perfect_forwarding.cpp`)
**Topics Covered:**
- Universal references (forwarding references)
- Template type deduction with `T&&`
- Perfect forwarding with `std::forward<T>`
- Variadic template forwarding
- Reference collapsing in templates
- Emplace idioms

**Key Questions:**
- What is a "universal reference"?
- When is `T&&` a universal reference vs an rvalue reference?
- How does `std::forward` preserve value category?

**Estimated Time:** 4 hours

### 5. Move-Only Types & RVO (`test_move_only_types.cpp`)
**Topics Covered:**
- Creating move-only classes
- `std::unique_ptr` semantics
- Return Value Optimization (RVO)
- Copy elision rules
- Factory patterns with move-only types
- Automatic move from local variables

**Key Questions:**
- How do you make a class move-only?
- Do you need `std::move` when returning local variables?
- How does RVO differ from move semantics?

**Estimated Time:** 3 hours

## How to Use This Suite

### 1. One Test at a Time (Recommended)
```bash
cd build/gcc14

# Start with rvalue references
./test_move_semantics/test_rvalue_references --gtest_filter=RvalueReferencesTest.LvalueVsRvalue

# Work through each test methodically
```

### 2. Fill in TODOs
- Open the test file
- Read the questions carefully
- Fill in the `// YOUR CODE HERE` sections
- Write your answers in the `// A:` sections

### 3. Run and Observe
```bash
# Rebuild after changes
ninja test_rvalue_references

# Run the test
./test_move_semantics/test_rvalue_references --gtest_filter=RvalueReferencesTest.LvalueVsRvalue
```

### 4. Socratic Feedback Loop
- If the test passes, read the `// R:` feedback
- If you disagree with the feedback, investigate using the instrumentation
- Ask follow-up questions in the `// A:` section

### 5. Run All Tests
```bash
# After completing all exercises
ctest -R MoveSemantics --output-on-failure
```

## Instrumentation

The suite includes custom instrumentation classes:

- **`MoveTracked`**: Logs all construction, copy, move, and destruction events
- **`Resource`**: Move-only type for testing ownership transfer
- **`EventLog`**: Singleton that records all instrumentation events

### Using EventLog
```cpp
EventLog::instance().clear();  // Clear before test
// ... your code ...
auto events = EventLog::instance().events();
size_t move_count = EventLog::instance().count_events("move_ctor");
```

## Key Concepts Map

```
Move Semantics
├── Value Categories
│   ├── Lvalue (has identity, can't be moved from)
│   ├── Rvalue (temporary, can be moved from)
│   └── Xvalue (expiring value, result of std::move)
│
├── Move Operations
│   ├── Move Constructor (T&&)
│   ├── Move Assignment (T& operator=(T&&))
│   └── Rule of Five
│
├── Forwarding
│   ├── std::move (unconditional cast)
│   ├── std::forward (conditional cast)
│   └── Universal References (T&&)
│
└── Optimizations
    ├── RVO (Return Value Optimization)
    ├── Copy Elision
    └── Automatic Move from Locals
```

## Common Pitfalls

1. **Using `std::move` in return statements**
   - Defeats RVO
   - Compiler already treats local returns as rvalues

2. **Moving from const objects**
   - Results in copy, not move
   - `const T&&` binds to const lvalue reference

3. **Forgetting `noexcept` on move operations**
   - Containers use copy instead of move
   - Performance degradation

4. **Using `std::move` instead of `std::forward` in templates**
   - Loses lvalue-ness
   - Breaks perfect forwarding

5. **Accessing moved-from objects**
   - Valid but unspecified state
   - Must reassign before use

## Learning Path

1. **Start with rvalue references** - Understand the foundation
2. **Master move assignment** - Learn the Rule of Five
3. **Compare std::move and std::forward** - Understand when to use each
4. **Practice perfect forwarding** - Template programming patterns
5. **Study move-only types** - Real-world patterns with unique_ptr

## Integration with shared_ptr

Move semantics builds on your `shared_ptr` knowledge:

- `shared_ptr` itself has move constructor/assignment
- Moving `shared_ptr` transfers ownership without incrementing use_count
- Factory functions return `shared_ptr` via move or RVO
- Perfect forwarding in `make_shared`

## Next Steps

After completing this module, you'll be ready for:

- Template metaprogramming (uses perfect forwarding extensively)
- Performance optimization (understanding when copies vs moves occur)
- Library design (creating move-aware APIs)
- Concurrent programming (move semantics with futures, promises)

## Reference

- C++11: Introduced move semantics and rvalue references
- C++14: Clarified move semantics and improved type deduction
- C++17: Guaranteed copy elision in certain cases
- C++20: Enhanced constexpr support for move operations
