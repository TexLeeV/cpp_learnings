# Contributing to C++ Learning Path

Thank you for your interest in contributing! This repository helps developers learn advanced C++ concepts through hands-on exercises using a Socratic teaching methodology.

## Table of Contents

- [Getting Started](#getting-started)
- [Development Environment](#development-environment)
- [Understanding the Socratic Q/A/R Pattern](#understanding-the-socratic-qar-pattern)
- [Code Style Guidelines](#code-style-guidelines)
- [Making Changes](#making-changes)
- [Testing Your Changes](#testing-your-changes)
- [Submitting a Pull Request](#submitting-a-pull-request)
- [Contribution Ideas](#contribution-ideas)

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/YOUR_FORK.git
   cd YOUR_FORK   # or your local directory name
   ```
3. **Create a branch** for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Environment

### Required Dependencies

- **CMake 3.21+** when using **`cmake --preset`** (see `CMakePresets.json`); root `CMakeLists.txt` allows 3.21+ for manual configures
- **C++20-compatible compiler** (GCC or Clang with full C++20 support)
- **GoogleTest and GoogleMock** (for testing; see installation)
- **Ninja** (recommended build system)

### Installation

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install cmake build-essential ninja-build
```

**Fedora/RHEL:**
```bash
sudo dnf install cmake gcc-c++ ninja-build
```

**macOS:**
```bash
brew install cmake ninja
```

### Building

```bash
# Configure with CMake preset
cmake --preset gcc

# Build all targets
cmake --build --preset gcc

# Run all tests
ctest --preset gcc --verbose

# Build specific target
cmake --build --preset gcc --target test_reference_counting
```

### Comments

- Avoid obvious comments that just narrate code
- Explain non-obvious intent, trade-offs, or constraints
- Use Q/A/R pattern for pedagogical questions

## Making Changes

### Types of Contributions

1. **Bug Fixes** - Correct test failures, build issues, or incorrect behavior
2. **New Learning Modules** - Add new C++ concept exercises
3. **Documentation** - Improve main README, fix typos
4. **Instrumentation** - Enhance EventLog or tracking capabilities
5. **Build System** - Improve CMake configuration

### Adding a New Learning Module

If adding a new module (e.g., `learning_coroutines`):

1. Create directory structure:
   ```
   learning_coroutines/
   ├── CMakeLists.txt
   └── tests/
       └── test_basic_coroutines.cpp
   ```

2. Add to root `CMakeLists.txt`:
   ```cmake
   add_subdirectory(learning_coroutines)
   ```

3. Create module `CMakeLists.txt` (same pattern as existing modules — `AddLearningTest` is already included from the root):
   ```cmake
   add_learning_test(test_basic_coroutines tests/test_basic_coroutines.cpp instrumentation)
   ```
   Link `Threads::Threads` as a fourth argument if the test uses `std::thread` or similar.

4. Include instrumentation headers:
   ```cpp
   #include "instrumentation.h"
   #include <gtest/gtest.h>
   ```

5. Update main **README.md** module table and [docs/LEARNING_PATH.md](../docs/LEARNING_PATH.md) if the curriculum or registered test count changes.

## Testing Your Changes

### Run Tests Locally

Before submitting:

```bash
# Build all targets
cmake --build --preset gcc

# Run all tests
ctest --preset gcc --verbose

# Run specific test suite
./build/gcc/learning_shared_ptr/test_reference_counting

# Run with filter
./build/gcc/learning_shared_ptr/test_reference_counting --gtest_filter=*BasicCreation*
```

Configure presets write outputs under `build/<preset>/` (here `gcc`); use `build/clang` if you use the clang preset.

## Submitting a Pull Request

### Before Submitting

- [ ] All tests pass locally (`ctest --preset gcc`)
- [ ] Code follows style guidelines (no trailing whitespace, correct formatting)
- [ ] Q/A/R patterns are preserved and correct (if applicable)
- [ ] Main README.md updated (if adding new module)
- [ ] No syntax violations or compiler warnings (Clang duplicated linker warnings are fine)
- [ ] Commit messages are clear and descriptive

### PR Description

Use the provided PR template. Include:

- **What changed**: Brief description of your changes
- **Why**: Motivation for the change
- **Testing**: How you verified the changes work
- **Related Issues**: Link any related issue numbers

### Review Process

1. Maintainer will review your PR
2. Address any feedback or requested changes
3. Once approved, your PR will be merged

### Commit Messages

Write clear, descriptive commit messages:

```
Add weak_ptr cycle breaking exercise

- Implement parent-child relationship test
- Add Q/A/R questions about observer pattern
- Update README with new test case
```

## Contribution Ideas

Looking for something to work on?

- **Fix typos** in comments or documentation
- **Improve Q/A/R questions** for clarity
- **Add compile-fail tests** for common mistakes
- **Enhance instrumentation output** for better debugging
- **Create new learning modules** (see [docs/LEARNING_PATH.md](../docs/LEARNING_PATH.md) and the README module table)
- **Add cross-platform tests** (Windows, additional compilers)
- **Improve build system** (better dependency detection, install targets)

## License Agreement

By contributing, you agree that your contributions will be licensed under the MIT License, the same license covering this project. Your contributions must be your original work.

## Questions?

If you have questions about contributing, please:

- Open a [Question issue](.github/ISSUE_TEMPLATE/question.md)
- Review existing issues and pull requests
- Check the main README.md for project context

Thank you for helping make this learning resource better for everyone!
