# Asio Setup Guide

## Installing Asio (Standalone, Header-Only)

### Option 1: System Package Manager (Recommended)

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install libasio-dev
```

#### Fedora/RHEL
```bash
sudo dnf install asio-devel
```

#### macOS (Homebrew)
```bash
brew install asio
```

#### Arch Linux
```bash
sudo pacman -S asio
```

### Option 2: Manual Installation

```bash
# Download latest Asio (standalone, no Boost required)
cd /tmp
wget https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-28-0.tar.gz
tar -xzf asio-1-28-0.tar.gz
cd asio-asio-1-28-0/asio

# Install headers
sudo cp -r include/asio.hpp /usr/local/include/
sudo cp -r include/asio /usr/local/include/
```

### Option 3: CMake FetchContent (Project-Local)

Add to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-28-0
)

FetchContent_MakeAvailable(asio)

# Create interface library
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
```

---

## Building the Multi-Threaded Tests

### Standard Build

```bash
cd test_shared_ptr
mkdir build && cd build
cmake ..
make test_multi_threaded_patterns
```

### If Asio Not Found

If CMake can't find Asio, you have two options:

#### Option A: Use Header-Only (No find_package)

Modify `CMakeLists.txt`:

```cmake
# Remove this line:
# find_package(asio REQUIRED)

# Add this instead:
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE /usr/include)  # or /usr/local/include
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
```

#### Option B: Specify Asio Path

```bash
cmake -DASIO_INCLUDE_DIR=/usr/local/include ..
```

---

## Running the Tests

```bash
# Run all multi-threaded tests
./test_multi_threaded_patterns

# Run specific test
./test_multi_threaded_patterns --gtest_filter=MultiThreadedPatternsTest.OwnershipTransferSafeWithSharedFromThis

# Run with verbose output
./test_multi_threaded_patterns --gtest_color=yes

# Run and see EventLog output (for debugging)
./test_multi_threaded_patterns --gtest_filter=MultiThreadedPatternsTest.ThreadSafeCacheWithWeakPtr
```

---

## Troubleshooting

### Error: "asio.hpp: No such file or directory"

**Solution**: Asio not installed or not in include path.

```bash
# Verify installation
ls /usr/include/asio.hpp
# or
ls /usr/local/include/asio.hpp

# If not found, install using Option 1 or 2 above
```

### Error: "undefined reference to pthread_create"

**Solution**: Add pthread to linker flags.

Already included in CMakeLists.txt:
```cmake
target_link_libraries(test_multi_threaded_patterns
    ...
    pthread
)
```

### C++11 Compatibility

**Good news**: The test suite uses C++11-compatible implementations of latch and barrier.

No C++20 required! The code includes:
- `CountDownLatch` - C++11 compatible latch
- `Barrier` - C++11 compatible barrier

These are implemented using `std::mutex` and `std::condition_variable`.

### Compilation is slow

**Solution**: Asio is header-only and can be slow to compile. Use precompiled headers:

```cmake
target_precompile_headers(test_multi_threaded_patterns PRIVATE
    <asio.hpp>
    <memory>
    <thread>
)
```

---

## Verification

After building, verify the tests work:

```bash
# Should see 15 tests
./test_multi_threaded_patterns --gtest_list_tests

# Expected output:
# MultiThreadedPatternsTest.
#   OwnershipTransferSafeWithSharedFromThis
#   OwnershipTransferUnsafeWithRawThis
#   AtomicUseCountIncrementAcrossThreads
#   ParallelCopyAndMoveOperations
#   WeakCallbackExpiresBeforeExecution
#   SharedCallbackKeepsObjectAlive
#   ThreadSafeCacheWithWeakPtr
#   ProducerConsumerWithSharedPtr
#   DestructionInDifferentThread
#   AliasingWithConcurrentAccess
#   WeakAliasingAcrossThreads
#   ThreadPoolOwnershipTransfer
#   SharedPtrDoesNotPreventDataRaces
#   ThreadSafeSingletonAccess
#   CircularReferenceAcrossThreads
#   BreakCircularReferenceWithWeakPtr
#   TimerCallbackWithWeakPtr
#   TimerCancellation
#   ConnectionManagementPattern
#   BindVsLambdaComparison
```

---

## Alternative: Use Boost.Asio

If you prefer Boost.Asio (same API):

```bash
# Install Boost
sudo apt install libboost-all-dev

# Modify includes in test file
# Change: #include <asio.hpp>
# To:     #include <boost/asio.hpp>

# Update CMakeLists.txt
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(test_multi_threaded_patterns
    ...
    Boost::system
)
```

---

## Quick Start (Ubuntu/Debian)

```bash
# 1. Install dependencies
sudo apt update
sudo apt install build-essential cmake libgtest-dev libasio-dev

# 2. Build
cd test_shared_ptr
mkdir build && cd build
cmake ..
make test_multi_threaded_patterns

# 3. Run
./test_multi_threaded_patterns

# 4. Success! You should see all tests pass
```

---

## Notes

- **Asio is header-only**: No linking required (except pthread)
- **Standalone Asio**: No Boost dependency needed
- **C++11 compatible**: Uses custom latch/barrier implementations
- **Thread-safe**: Tests use proper synchronization primitives
- **Deterministic**: Uses barriers/latches for reproducible results

If you encounter any issues, check:
1. Asio installed? (`ls /usr/include/asio.hpp`)
2. C++11 compiler? (`g++ --version` should be ≥4.8)
3. CMake version? (`cmake --version` should be ≥3.14)
