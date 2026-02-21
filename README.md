# C++ Learning Path

## About This Repository

This repository contains hands-on learning exercises for advanced C++ concepts, using a Socratic teaching methodology with fill-in exercises, broken code to fix, and guided Q/A/R (Question/Answer/Response) patterns.

## Current Progress

- ✅ **Smart Pointers (`test_shared_ptr/`)**: Comprehensive `shared_ptr` test suite with 17 test files covering ownership, lifetime, aliasing, weak_ptr, and multi-threaded patterns
- 🔄 **Deadlock Patterns (`test_deadlocks/`)**: 16 scenarios across 4 files demonstrating deadlock patterns with empty fix implementations

## Learning TODO List

Based on your background (Lead Sr. Software Engineer with C/C++, Python, TypeScript experience), here are the recommended next learning modules:

---

### 1. ✅ C++11 Minimum Version Support
**Status**: Partially Complete  
**Difficulty**: ⭐☆☆☆☆ (Trivial)  
**Estimated Time**: 15-30 minutes  
**Your Level**: You already use C++11 features in your work

**Tasks**:
- [ ] Add CMake version checks and compiler feature requirements
- [ ] Document C++11 features used throughout the codebase
- [ ] Add static assertions for minimum compiler versions
- [ ] Create a compatibility matrix document

**Why This Matters**: Ensures portability across different development environments and prevents subtle bugs from compiler differences.

---

### 2. Move Semantics & Perfect Forwarding
**Status**: Not Started  
**Difficulty**: ⭐⭐⭐☆☆ (Moderate)  
**Estimated Time**: 12-16 hours  
**Your Level**: Intermediate - You understand ownership deeply, move semantics is a natural extension

**Planned Modules**:
- [ ] Rvalue references and move constructors (4 hours)
- [ ] Move assignment and Rule of Five (3 hours)
- [ ] `std::move` vs `std::forward` mechanics (3 hours)
- [ ] Perfect forwarding and universal references (4 hours)
- [ ] Move-only types and return value optimization (3 hours)

**Prerequisites**: Strong understanding of `shared_ptr` ownership (✅ Complete)

**Deliverable**: Test suite similar to `test_shared_ptr` with 8-10 test files, ~40 scenarios

---

### 3. Concurrency Patterns
**Status**: Partially Started (Deadlock tests in progress)  
**Difficulty**: ⭐⭐⭐⭐☆ (Hard)  
**Estimated Time**: 20-25 hours  
**Your Level**: Intermediate-Advanced - You have multi-threaded experience from your work

**Planned Modules**:
- [x] Deadlock patterns (8-12 hours) - **In Progress**
- [ ] Thread-safe singleton patterns (3 hours)
- [ ] Reader-writer locks and `shared_mutex` (4 hours)
- [ ] Lock-free data structures basics (6 hours)
- [ ] Producer-consumer advanced patterns (4 hours)
- [ ] Thread pools and work stealing (5 hours)

**Prerequisites**: Deadlock patterns (🔄 In Progress)

**Deliverable**: 6-8 test suites covering synchronization primitives, lock-free basics, and async patterns

---

### 4. Template Metaprogramming
**Status**: Not Started  
**Difficulty**: ⭐⭐⭐⭐⭐ (Very Hard)  
**Estimated Time**: 25-30 hours  
**Your Level**: Beginner-Intermediate - This is likely new territory

**Planned Modules**:
- [ ] Function and class templates (4 hours)
- [ ] Template specialization (full and partial) (5 hours)
- [ ] SFINAE and `std::enable_if` (6 hours)
- [ ] Variadic templates and fold expressions (6 hours)
- [ ] Type traits and metafunctions (5 hours)
- [ ] Practical applications (generic containers, type-safe APIs) (6 hours)

**Prerequisites**: Strong C++ fundamentals (✅ Complete)

**Deliverable**: 8-10 test files with progressive complexity, focusing on practical applications

**Note**: This is the steepest learning curve on the list. Recommend tackling after Move Semantics and Concurrency.

---

### 5. RAII & Resource Management
**Status**: Partially Complete (via `shared_ptr` tests)  
**Difficulty**: ⭐⭐☆☆☆ (Easy-Moderate)  
**Estimated Time**: 8-10 hours  
**Your Level**: Intermediate - You understand RAII principles from smart pointers

**Planned Modules**:
- [ ] Scope guards and cleanup patterns (2 hours)
- [ ] File handle and socket management (2 hours)
- [ ] Custom resource managers (3 hours)
- [ ] Building smart pointers from scratch (4 hours)

**Prerequisites**: `shared_ptr` deep dive (✅ Complete)

**Deliverable**: 4-5 test files with practical resource management scenarios

---

### 6. Design Patterns (C++ Specific)
**Status**: Not Started  
**Difficulty**: ⭐⭐⭐☆☆ (Moderate)  
**Estimated Time**: 18-22 hours  
**Your Level**: Intermediate - You've implemented patterns in production (Observer, Factory from your work)

**Planned Modules**:
- [ ] Creational patterns (Factory, Builder, Singleton) (5 hours)
- [ ] Structural patterns (Adapter, Decorator, Proxy, Flyweight) (6 hours)
- [ ] Behavioral patterns (Observer, Strategy, Visitor, Command) (7 hours)
- [ ] Modern C++ pattern implementations (5 hours)

**Prerequisites**: Move semantics, RAII

**Deliverable**: 10-12 test files, each implementing a pattern with broken/correct versions

**Note**: This will feel familiar given your production experience, but C++-specific implementations (using templates, move semantics) will be new.

---

### 7. Memory Management Deep Dive
**Status**: Not Started  
**Difficulty**: ⭐⭐⭐⭐☆ (Hard)  
**Estimated Time**: 15-18 hours  
**Your Level**: Intermediate - You work with semiconductors and understand hardware

**Planned Modules**:
- [ ] Custom allocators and `std::allocator` interface (4 hours)
- [ ] Pool allocators (fixed-size and variable-size) (5 hours)
- [ ] Memory alignment and cache-friendly structures (4 hours)
- [ ] Placement new and aligned storage (3 hours)
- [ ] Memory profiling and leak detection (3 hours)

**Prerequisites**: RAII, understanding of control blocks from `shared_ptr`

**Deliverable**: 6-7 test files with allocator implementations and performance comparisons

**Note**: Your semiconductor background gives you an advantage here - you understand cache lines and memory hierarchies.

---

### 8. Error Handling
**Status**: Not Started  
**Difficulty**: ⭐⭐⭐☆☆ (Moderate)  
**Estimated Time**: 10-12 hours  
**Your Level**: Intermediate - You understand exception safety from RAII

**Planned Modules**:
- [ ] Exception safety guarantees (basic, strong, no-throw) (3 hours)
- [ ] RAII for exception safety (2 hours)
- [ ] `std::optional` and result types (3 hours)
- [ ] Error codes vs exceptions (when to use which) (2 hours)
- [ ] `noexcept` specifications and move operations (3 hours)

**Prerequisites**: RAII, Move Semantics

**Deliverable**: 5-6 test files demonstrating exception-safe code patterns

---

### 9. STL Deep Dive
**Status**: Partially Started (using STL in tests)  
**Difficulty**: ⭐⭐⭐☆☆ (Moderate)  
**Estimated Time**: 14-16 hours  
**Your Level**: Intermediate - You use STL containers daily

**Planned Modules**:
- [ ] Container internals (vector growth, deque, map vs unordered_map) (4 hours)
- [ ] Iterator categories and custom iterators (4 hours)
- [ ] Algorithm complexity and parallel algorithms (3 hours)
- [ ] Custom comparators and hash functions (3 hours)
- [ ] Iterator invalidation rules (2 hours)

**Prerequisites**: Template basics

**Deliverable**: 6-8 test files exploring container internals and performance characteristics

---

### 10. Modern C++ Features (C++11/14/17)
**Status**: Partially Using (C++11 in current tests)  
**Difficulty**: ⭐⭐☆☆☆ (Easy-Moderate)  
**Estimated Time**: 12-15 hours  
**Your Level**: Intermediate - You use some features, but not all

**Planned Modules**:

**C++11/14**:
- [ ] Lambda expressions (captures, mutable, generic) (3 hours)
- [ ] `auto` and type deduction rules (2 hours)
- [ ] Uniform initialization and initializer lists (2 hours)
- [ ] Delegating and inheriting constructors (2 hours)

**C++17**:
- [ ] Structured bindings (2 hours)
- [ ] `std::optional`, `std::variant`, `std::any` (3 hours)
- [ ] `std::string_view` (2 hours)
- [ ] `if constexpr` and fold expressions (3 hours)

**Prerequisites**: Template basics for advanced features

**Deliverable**: 8-10 test files demonstrating feature usage and gotchas

---

### 11. Performance & Optimization
**Status**: Not Started  
**Difficulty**: ⭐⭐⭐⭐☆ (Hard)  
**Estimated Time**: 16-20 hours  
**Your Level**: Intermediate-Advanced - Your production work requires performance awareness

**Planned Modules**:
- [ ] Profiling and identifying bottlenecks (4 hours)
- [ ] Cache-friendly data structures (4 hours)
- [ ] Copy elision and RVO (3 hours)
- [ ] Small object optimization (3 hours)
- [ ] `constexpr` and compile-time computation (4 hours)
- [ ] Benchmarking methodology (3 hours)

**Prerequisites**: Move semantics, Memory management

**Deliverable**: 6-8 test files with before/after optimization comparisons and benchmarks

**Note**: Your semiconductor testing background means you understand performance measurement - this will feel natural.

---

### 12. Testing & Debugging
**Status**: Partially Started (using GoogleTest)  
**Difficulty**: ⭐⭐⭐☆☆ (Moderate)  
**Estimated Time**: 12-14 hours  
**Your Level**: Advanced - You develop test programs professionally

**Planned Modules**:
- [ ] GoogleMock for mocking (4 hours)
- [ ] Benchmark testing with Google Benchmark (3 hours)
- [ ] Static analysis tools (clang-tidy, cppcheck) (3 hours)
- [ ] Sanitizers (AddressSanitizer, ThreadSanitizer, UBSan) (3 hours)
- [ ] Property-based testing (2 hours)

**Prerequisites**: None - can start anytime

**Deliverable**: Enhanced test infrastructure, CI/CD integration examples

**Note**: This leverages your existing strength - you'll pick this up quickly.

---

## Recommended Learning Order

Based on your background and the dependencies between topics:

### **Phase 1: Foundations (Complete Current Work)**
1. ✅ Finish `shared_ptr` tests (if any remaining)
2. 🔄 Complete `test_deadlocks/` (8-12 hours remaining)
3. ⭐ C++11 version support documentation (30 minutes)

**Total Phase 1**: ~8-12 hours

---

### **Phase 2: Core Modern C++ (Build on Ownership Knowledge)**
4. Move Semantics & Perfect Forwarding (12-16 hours)
5. RAII & Resource Management (8-10 hours)
6. Modern C++ Features C++11/14/17 (12-15 hours)

**Total Phase 2**: ~32-41 hours (~1-1.5 months at 2-3 hours/day)

---

### **Phase 3: Advanced Concurrency (Leverage Your Multi-threaded Experience)**
7. Complete Concurrency Patterns (12-13 hours remaining after deadlocks)
8. Memory Management Deep Dive (15-18 hours)

**Total Phase 3**: ~27-31 hours (~3-4 weeks at 2-3 hours/day)

---

### **Phase 4: Practical Patterns (Apply Everything)**
9. Design Patterns (18-22 hours)
10. Error Handling (10-12 hours)
11. STL Deep Dive (14-16 hours)

**Total Phase 4**: ~42-50 hours (~1.5-2 months at 2-3 hours/day)

---

### **Phase 5: Advanced Topics (Mastery)**
12. Template Metaprogramming (25-30 hours)
13. Performance & Optimization (16-20 hours)
14. Testing & Debugging (12-14 hours)

**Total Phase 5**: ~53-64 hours (~2-2.5 months at 2-3 hours/day)

---

## Overall Estimates

| Category | Total Hours | Difficulty | Your Advantage |
|----------|-------------|------------|----------------|
| **Foundations** | 8-12 | ⭐⭐☆☆☆ | Strong ownership understanding |
| **Core Modern C++** | 32-41 | ⭐⭐⭐☆☆ | Production C++ experience |
| **Advanced Concurrency** | 27-31 | ⭐⭐⭐⭐☆ | Multi-threaded production work |
| **Practical Patterns** | 42-50 | ⭐⭐⭐☆☆ | Design pattern experience |
| **Advanced Topics** | 53-64 | ⭐⭐⭐⭐⭐ | Semiconductor/hardware knowledge helps |
| **TOTAL** | **162-198 hours** | - | **6-8 months at 2-3 hours/day** |

---

## Your Strengths (Based on Resume)

✅ **Production C/C++ development** - You write production-quality code daily  
✅ **Multi-threaded experience** - Your work involves concurrent systems  
✅ **Test development** - You build test programs professionally  
✅ **Multiple languages** - Python, TypeScript, Java experience aids learning  
✅ **Hardware understanding** - Semiconductor work gives you memory/cache intuition  
✅ **Team leadership** - You've trained engineers, so you understand teaching methodology  

---

## Success Metrics

After completing this learning path, you will be able to:

- ✅ Write exception-safe, move-aware modern C++ code
- ✅ Design and implement lock-free data structures
- ✅ Create generic, reusable template libraries
- ✅ Optimize code with profiling and benchmarking
- ✅ Debug complex multi-threaded issues with sanitizers
- ✅ Implement custom allocators and resource managers
- ✅ Apply appropriate design patterns in C++
- ✅ Make informed decisions about C++11/14/17/20 features

---

## Repository Structure

```
cpp/
├── README.md (this file)
├── test_shared_ptr/          # ✅ Complete - Smart pointer deep dive
│   ├── tests/                # 17 test files covering ownership patterns
│   └── README.md
├── test_deadlocks/           # 🔄 In Progress - 16 deadlock scenarios
│   ├── tests/
│   └── SUMMARY.txt
├── test_move_semantics/      # 📋 Planned - Phase 2
├── test_concurrency/         # 📋 Planned - Phase 3
├── test_templates/           # 📋 Planned - Phase 5
├── test_raii/                # 📋 Planned - Phase 2
├── test_design_patterns/     # 📋 Planned - Phase 4
├── test_memory/              # 📋 Planned - Phase 3
├── test_error_handling/      # 📋 Planned - Phase 4
├── test_stl/                 # 📋 Planned - Phase 4
├── test_modern_cpp/          # 📋 Planned - Phase 2
├── test_performance/         # 📋 Planned - Phase 5
└── test_debugging/           # 📋 Planned - Phase 5
```

---

## Getting Started

### Current Focus: Complete Deadlock Patterns

You're currently working on `test_deadlocks/` with 16 scenarios to fix:

1. Navigate to `test_deadlocks/`
2. Read `SUMMARY.txt` for overview
3. Start with `test_mutex_ordering_deadlocks.cpp` Scenario 1
4. Follow the Socratic Q/A/R pattern
5. Implement the `*_fixed()` functions

**Estimated Completion**: 8-12 hours remaining

---

## Contributing

This is a personal learning repository. Each module follows the same pattern:

1. **Broken implementations** - Complete code demonstrating anti-patterns
2. **Empty fix sections** - You implement the correct solution
3. **Socratic Q/A/R** - Inline questions guide your learning
4. **Progressive difficulty** - Easy → Moderate → Hard → Very Hard

---

## Tools & Environment

- **Compiler**: GCC/Clang with C++11 minimum (C++17 recommended)
- **Build System**: CMake 3.14+
- **Testing**: GoogleTest
- **IDE**: Cursor (with AI assistance for Socratic learning)

---

## Building & Running Tests

All test modules are part of a single unified CMake project:

```bash
# From project root
mkdir build && cd build
cmake ..
make

# Run all tests
ctest --verbose

# Run specific test suite
./test_shared_ptr/test_reference_counting
./test_deadlocks/test_mutex_ordering_deadlocks

# Run with gtest filter
./test_shared_ptr/test_reference_counting --gtest_filter=*BasicCreation*

# Build specific target only
make test_mutex_ordering_deadlocks
```

### Requirements

- CMake 3.14+
- GCC/Clang with C++11 support
- GoogleTest (will be found by CMake)
- pthread (for multi-threaded tests)

---

## Notes

- All time estimates assume 2-3 focused hours per day
- Difficulty ratings are calibrated to your experience level (Lead Sr. SWE)
- Prerequisites are suggestions, not strict requirements
- Feel free to jump around based on project needs or interest

---

## Next Steps

1. ✅ Review this README
2. 🔄 Complete `test_deadlocks/` (current focus)
3. ⭐ Add C++11 version checks to CMake
4. 📋 Choose Phase 2 module to start (recommend Move Semantics)

---

**Last Updated**: February 2026  
**Current Phase**: Phase 1 - Foundations (90% complete)
