# Building & Running Tests

This document covers build setup, dependencies, and running tests. For an overview and quickstart, see the [main README](../README.md).

---

## Requirements

- CMake 3.14+
- GCC/Clang with C++17 support
- GoogleTest
- Threads (for multi-threaded tests)

---

## Quick Build

All test modules are part of a single unified CMake project:

```bash
# Configure and build
cmake --preset gcc
cmake --build --preset gcc

# Run all tests
ctest --preset gcc --verbose
```

---

## Running Specific Tests

```bash
# Run a specific test suite
./build/gcc/learning_shared_ptr/test_reference_counting
./build/gcc/learning_deadlocks/test_mutex_ordering_deadlocks

# Run with gtest filter
./build/gcc/learning_shared_ptr/test_reference_counting --gtest_filter=*BasicCreation*

# Build specific target only
cmake --build --preset gcc --target test_mutex_ordering_deadlocks
```

---

## Installing Dependencies

### GoogleTest (Required)

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install libgtest-dev cmake build-essential ninja-build
```

**Fedora/RHEL:**
```bash
sudo dnf install gtest-devel cmake gcc-c++ ninja-build
sudo yum install gtest-devel cmake gcc-c++ ninja-build
```

**macOS:**
```bash
brew install googletest cmake ninja
```

CMake will find GoogleTest via `find_package(GTest REQUIRED)`. If installed to a non-standard location:

```bash
cmake --preset gcc -DGTest_DIR=/path/to/gtest/lib/cmake/GTest
```

### Asio Setup (Optional)

Required only for `test_asio_basics` and `test_multi_threaded_patterns` in `learning_shared_ptr/`:

```bash
# Ubuntu/Debian
sudo apt install libasio-dev

# Fedora/RHEL
sudo dnf install asio-devel

# macOS
brew install asio
```

CMake will auto-detect Asio via `find_path`. If installed to a non-standard location:

```bash
cmake -DASIO_INCLUDE_DIR=/path/to/asio/include ..
```

---

## Tools & Environment

- **Compiler**: GCC 14 / Clang with C++17 support
- **Build System**: CMake 3.14+ (Ninja recommended)
- **Testing**: GoogleTest
- **IDE**: Cursor (with AI assistance for Socratic learning)
- **Optional**: Standalone Asio (header-only) for multi-threaded shared_ptr tests
