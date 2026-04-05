# Building & Running Tests

Build setup, dependencies, and how to run tests. For project overview, see the [main README](../README.md).

---

## Requirements

- **CMake** — **3.21 or newer** if you use **`cmake --preset`** ([CMakePresets.json](../CMakePresets.json)). The root [CMakeLists.txt](../CMakeLists.txt) declares `cmake_minimum_required(3.21)` for manual configures; presets are the documented workflow.
- **Compiler** — **C++20** required (use a C++20-capable GCC or Clang).
- **GoogleTest & GoogleMock** — Downloaded automatically during `cmake --preset <name>` configuration via **FetchContent**. System packages below are optional fallback if your environment blocks FetchContent/network access.
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

Each configure preset writes its build tree under **`build/<preset>/`** (for example `build/gcc`, `build/clang`, `build/gcc-asan`). Test binaries and `compile_commands.json` live there.

---

## Running specific tests

```bash
# Run one executable (use the directory for your preset, e.g. build/gcc)
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

Targets are registered by default. Binaries appear under `build/<preset>/learning_deadlocks/` (for example `build/gcc/learning_deadlocks/`). Some scenario tests are intentionally disabled while remaining fixes are in progress. Scenarios live in `learning_deadlocks/tests/` (`test_mutex_ordering_deadlocks.cpp`, `test_circular_reference_deadlocks.cpp`, `test_condition_variable_deadlocks.cpp`, `test_ownership_transfer_deadlocks.cpp`).

---

## Installing optional system dependencies

### GoogleTest & GoogleMock (optional fallback)

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

- **Compiler:** C++20-capable **GCC** or **Clang**
- **Build:** CMake **3.21+** for presets; Ninja recommended (generator in presets)
- **Tests:** GoogleTest & GoogleMock (auto-fetched by preset-based configure)
- **IDE:** Cursor (optional Socratic rules in `.cursor/rules/`). `.clangd` points at `build/gcc` for `compile_commands.json`; switch to `build/clang` if you use the clang preset.

---

## Known caveats

- **Timing-sensitive tests** (e.g. cache/branch microbenchmarks) may occasionally fail on virtualized or heavily loaded machines; re-run or compare trends rather than single-shot wall clock on CI-like hosts.
