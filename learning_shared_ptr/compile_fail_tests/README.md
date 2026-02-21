# Compile-Fail Tests

These tests are designed to **fail compilation** until you understand and fix the underlying ownership or lifetime issue.

## Workflow

1. **Read the test file** to understand the scenario.
2. **Predict why it won't compile** before attempting to build.
3. **Try to compile** and observe the compiler error.
4. **Reason about the error** in relation to `shared_ptr` semantics.
5. **Fix the issue** based on the analysis.
6. **Verify** the fix compiles and behaves as expected.

## Important Notes

- These files are **not included** in the main CMake build.
- You must compile them manually to observe the failure.
- Do not look for hints in comments—reason from first principles.

## Test Files

- `unique_ptr_copy.cpp` - Attempts to copy a `unique_ptr`
- `shared_from_raw.cpp` - Multiple `shared_ptr` instances from the same raw pointer
- `shared_ptr_to_stack.cpp` - Creating a `shared_ptr` to a stack-allocated object
- `weak_ptr_direct_use.cpp` - Using a `weak_ptr` without promotion
- `deleted_copy_constructor.cpp` - Copying a non-copyable type wrapped in `shared_ptr`

## Compilation Command

To test a specific file:

```bash
g++ -std=c++11 -I../src -c <filename>.cpp
```

Expected result: **compilation error**.
