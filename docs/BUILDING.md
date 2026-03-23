# Building & Running Tests

Build setup, dependencies, and how to run tests. For project overview, see the [main README](../README.md).

---

## Requirements

- **CMake** — **3.23 or newer** if you use **`cmake --preset`** ([CMakePresets.json](../CMakePresets.json)). The root [CMakeLists.txt](../CMakeLists.txt) declares `cmake_minimum_required(3.14)` for manual configures; presets are the documented workflow.
- **Compiler** — **C++20** (GCC 14 or recent Clang recommended).
- **GoogleTest & GoogleMock** — Required (`find_package(GTest REQUIRED)`).
- **Threads** — For concurrency tests (and deadlock tests when enabled).

---

## Quick build

From the repository root:

```bash
cmake --preset gcc
cmake --build --preset gcc

# Run all tests registered with CTest
ctest --preset gcc --verbose
```

Use `cmake --preset clang` / `cmake --build --preset clang` if you prefer Clang.

---

## Running specific tests

```bash
# Run one executable (path matches your preset output directory)
./build/gcc/learning_shared_ptr/test_reference_counting

# GoogleTest filter
./build/gcc/learning_shared_ptr/test_reference_counting --gtest_filter=*BasicCreation*

# Build one target
cmake --build --preset gcc --target test_reference_counting
```

**Filter by name with ctest:**

```bash
cd build/gcc
ctest -R test_reference_counting --output-on-failure
```

### `learning_deadlocks`

Targets are **not** registered by default (`add_learning_test` lines are commented in `learning_deadlocks/CMakeLists.txt`). After you uncomment them and reconfigure, binaries would appear under `build/gcc/learning_deadlocks/`. See [learning_deadlocks/SUMMARY.txt](../learning_deadlocks/SUMMARY.txt) for scenario layout.

---

## Installing dependencies

### GoogleTest & GoogleMock (required)

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install libgtest-dev libgmock-dev cmake build-essential ninja-build
```

**Fedora/RHEL:**

```bash
sudo dnf install gtest-devel gmock-devel cmake gcc-c++ ninja-build
```

**macOS:**

```bash
brew install googletest cmake ninja
```

---

## Tools and environment

- **Compiler:** GCC 14 / recent Clang with **C++20**
- **Build:** CMake **3.23+** for presets; Ninja recommended (generator in presets)
- **Tests:** GoogleTest & GoogleMock
- **IDE:** Cursor (optional Socratic rules in `.cursor/rules/`)

---

## Known caveats

- **Timing-sensitive tests** (e.g. cache/branch microbenchmarks) may occasionally fail on virtualized or heavily loaded machines; re-run or compare trends rather than single-shot wall clock on CI-like hosts.
