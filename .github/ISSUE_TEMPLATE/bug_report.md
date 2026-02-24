---
name: Bug Report
about: Report a bug in a test, build issue, or incorrect behavior
title: '[BUG] '
labels: bug
assignees: ''
---

## Description

A clear and concise description of what the bug is.

## Steps to Reproduce

1. Navigate to module: `learning_*/`
2. Build target: `cmake --build --preset gcc --target <target_name>`
3. Run test: `./build/gcc/<module>/<test_name>`
4. Observe the error at step...

## Expected Behavior

What you expected to happen.

## Actual Behavior

What actually happened. Include any error messages or unexpected output.

## Environment

- **OS**: [e.g., Ubuntu 24.04, macOS 14]
- **Compiler**: [e.g., GCC 14, Clang 17]
- **CMake Version**: [e.g., 3.28]
- **Build Preset**: [e.g., gcc]

## Test File

Which test file exhibits the bug?

- Module: [e.g., `learning_shared_ptr`]
- Test File: [e.g., `test_reference_counting.cpp`]
- Test Case: [e.g., `TEST(ReferenceCountingTest, BasicCreation)`]

## Build Output

```
Paste relevant build output or test failure here
```

## Additional Context

Add any other context about the problem here (e.g., instrumentation output, EventLog dump, Q/A/R comments).
