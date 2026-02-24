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
   git clone https://github.com/YOUR_USERNAME/cpp.git
   cd cpp
   ```
3. **Create a branch** for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Environment

### Required Dependencies

- **CMake 3.14+** (3.28+ recommended)
- **C++17 compatible compiler**:
  - GCC 14 (reference compiler)
  - Clang 17+
- **GoogleTest** (for testing)
- **Ninja** (recommended build system)

### Optional Dependencies

- **Asio** (standalone, for multi-threaded shared_ptr tests only)

### Installation

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install libgtest-dev cmake build-essential ninja-build
```

**Fedora/RHEL:**
```bash
sudo dnf install gtest-devel cmake gcc-c++ ninja-build
```

**macOS:**
```bash
brew install googletest cmake ninja
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

## Understanding the Socratic Q/A/R Pattern

This repository uses inline comments for guided learning:

- `// Q:` - Question posed about the code's behavior
- `// A:` - Space for learner's answer (fill this in)
- `// R:` - Response/feedback on the answer (provided after learner answers)

### Example

```cpp
// Q: What is the use_count after this line?
auto sp2 = sp1;
// A: 2
// R: Correct! Copy construction increments the reference count.
```

When contributing Q/A/R content:
- Questions should be specific and answerable by running the code
- Focus on observable behavior (use_count, destructor calls, EventLog output)
- Responses should validate or correct the answer with technical precision
- Use instrumentation (`EventLog::instance().dump()`) to make behavior visible

## Code Style Guidelines

We follow specific C++ coding standards defined in `.cursor/rules/`:

### Syntax Rules

- **C++17 standard** is required
- **Add newlines before opening braces** for readability
- **No trailing whitespace** on any line
- **File must end with exactly one newline**

### Constructor Initializer Lists

Use leading comma format:

```cpp
SignalHandler::SignalHandler(asio::io_context& io_context)
: io_context_(io_context)
, signals_(io_context, SIGUSR1)
{
}
```

- Constructor signature on its own line
- Colon on the next line after signature
- First initializer on same line as colon (or next line)
- Each additional initializer on its own line with **leading comma**
- Opening brace `{` on its own line

### Documentation

- **DO NOT** create additional README.md files in subdirectories
- **DO NOT** add large summary sections at the end of code files
- **DO** add concise inline comments for specific code sections
- **DO** use Q/A/R pattern for learning content
- **DO** update main README.md with new module information

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

3. Create module `CMakeLists.txt`:
   ```cmake
   include(${CMAKE_SOURCE_DIR}/cmake/AddLearningTest.cmake)
   add_learning_test(test_basic_coroutines tests/test_basic_coroutines.cpp)
   ```

4. Include instrumentation headers:
   ```cpp
   #include "instrumentation.h"
   #include <gtest/gtest.h>
   ```

5. Update main README.md with:
   - Module description in "Learning TODO List"
   - Status and difficulty rating
   - Prerequisites

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

### Verify Code Style

```bash
# Check for trailing whitespace
git diff --check

# Verify file ends with newline
tail -c 1 your_file.cpp | od -An -tx1
# Should output: 0a (newline)
```

### Test Instrumentation Output

For tests using `EventLog`:

```cpp
TEST(YourTest, YourCase)
{
    EventLog::instance().clear();
    
    // Your test code
    
    EventLog::instance().dump();  // Verify output is correct
    auto events = EventLog::instance().events();
    EXPECT_EQ(events.size(), expected_count);
}
```

## Submitting a Pull Request

### Before Submitting

- [ ] All tests pass locally (`ctest --preset gcc`)
- [ ] Code follows style guidelines (no trailing whitespace, correct formatting)
- [ ] Q/A/R patterns are preserved and correct (if applicable)
- [ ] Main README.md updated (if adding new module)
- [ ] No syntax violations or compiler warnings
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
- **Create new learning modules** from the TODO list in README
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
