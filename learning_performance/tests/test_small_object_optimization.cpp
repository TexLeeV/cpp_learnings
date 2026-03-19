// Test Suite: Small Object Optimization (SSO)
// Estimated Time: 3 hours
// Difficulty: Moderate to Hard
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <string>
#include <vector>
#include <functional>
#include <cstring>

class SmallObjectOptimizationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: std::string Small String Optimization - Easy
// ============================================================================

TEST_F(SmallObjectOptimizationTest, StringSSO)
{
    std::string small = "short";
    std::string large = "this is a much longer string that exceeds the SSO buffer size";

    const char* small_ptr = small.data();
    const char* large_ptr = large.data();

    const void* small_obj_addr = &small;
    const void* large_obj_addr = &large;

    ptrdiff_t small_offset = reinterpret_cast<const char*>(small_ptr) -
                              reinterpret_cast<const char*>(small_obj_addr);
    ptrdiff_t large_offset = reinterpret_cast<const char*>(large_ptr) -
                              reinterpret_cast<const char*>(large_obj_addr);

    EventLog::instance().record("Small string offset: " + std::to_string(small_offset));
    EventLog::instance().record("Large string offset: " + std::to_string(large_offset));

    // Q: If small_offset is small (0-32), what does this indicate about where the
    // Q: string data is stored?
    // A:
    // R:

    // Q: If large_offset is large or negative, what does this indicate about heap
    // Q: allocation?
    // A:
    // R:

    // Q: What is the typical SSO buffer size for std::string? How can you measure it?
    // A:
    // R:

    EXPECT_LT(std::abs(small_offset), 64);
}

// ============================================================================
// TEST 2: SSO Performance Impact - Moderate
// ============================================================================

TEST_F(SmallObjectOptimizationTest, SSOPerformanceImpact)
{
    constexpr int iterations = 100000;

    auto start_small = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        std::string s = "small";
        volatile char c = s[0];
        (void)c;
    }
    auto end_small = std::chrono::high_resolution_clock::now();
    auto small_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_small - start_small).count();

    auto start_large = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        std::string s = "this is a much longer string that definitely exceeds SSO buffer";
        volatile char c = s[0];
        (void)c;
    }
    auto end_large = std::chrono::high_resolution_clock::now();
    auto large_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_large - start_large).count();

    EventLog::instance().record("Small string creation: " + std::to_string(small_duration) + " us");
    EventLog::instance().record("Large string creation: " + std::to_string(large_duration) + " us");

    // Q: Small strings avoid heap allocation. What is the performance benefit?
    // A:
    // R:

    // Q: Large strings require heap allocation. What additional cost does this incur
    // Q: beyond the allocation itself? (Hint: cache, memory bandwidth)
    // A:
    // R:

    EXPECT_LT(small_duration, large_duration);
}

// ============================================================================
// TEST 3: std::function Small Object Optimization - Moderate
// ============================================================================

TEST_F(SmallObjectOptimizationTest, FunctionSSO)
{
    int small_capture = 42;
    std::function<int()> small_lambda = [small_capture]() { return small_capture; };

    std::vector<int> large_capture(100, 42);
    std::function<int()> large_lambda = [large_capture]() { return large_capture[0]; };

    EXPECT_EQ(small_lambda(), 42);
    EXPECT_EQ(large_lambda(), 42);

    // Q: std::function has an internal buffer for small callables. What happens when
    // Q: the lambda capture is too large?
    // A:
    // R:

    // Q: How can you measure whether a std::function uses heap allocation?
    // A:
    // R:
}

// ============================================================================
// TEST 4: Implementing Custom SSO Container - Hard
// ============================================================================

template<typename T, size_t BufferSize = 16>
class SmallVector
{
public:
    SmallVector()
    : size_(0), capacity_(BufferSize), heap_data_(nullptr)
    {
        EventLog::instance().record("SmallVector::ctor");
    }

    ~SmallVector()
    {
        if (heap_data_)
        {
            EventLog::instance().record("SmallVector::dtor (heap)");
            delete[] heap_data_;
        }
        else
        {
            EventLog::instance().record("SmallVector::dtor (stack)");
        }
    }

    void push_back(const T& value)
    {
        if (size_ == capacity_)
        {
            grow();
        }

        if (heap_data_)
        {
            heap_data_[size_++] = value;
        }
        else
        {
            new (&buffer_[size_++]) T(value);
        }
    }

    size_t size() const { return size_; }
    bool uses_heap() const { return heap_data_ != nullptr; }

private:
    void grow()
    {
        size_t new_capacity = capacity_ * 2;
        T* new_data = new T[new_capacity];

        EventLog::instance().record("SmallVector::grow to " + std::to_string(new_capacity));

        for (size_t i = 0; i < size_; ++i)
        {
            if (heap_data_)
            {
                new_data[i] = std::move(heap_data_[i]);
            }
            else
            {
                new_data[i] = std::move(*reinterpret_cast<T*>(&buffer_[i * sizeof(T)]));
            }
        }

        if (heap_data_)
        {
            delete[] heap_data_;
        }

        heap_data_ = new_data;
        capacity_ = new_capacity;
    }

    size_t size_;
    size_t capacity_;
    T* heap_data_;
    alignas(T) char buffer_[BufferSize * sizeof(T)];
};

TEST_F(SmallObjectOptimizationTest, CustomSSOContainer)
{
    SmallVector<int, 8> vec;

    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i);
    }

    EXPECT_FALSE(vec.uses_heap());
    EXPECT_EQ(EventLog::instance().count_events("SmallVector::grow"), 0);

    for (int i = 5; i < 20; ++i)
    {
        vec.push_back(i);
    }

    EXPECT_TRUE(vec.uses_heap());
    EXPECT_GT(EventLog::instance().count_events("SmallVector::grow"), 0);

    // Q: SmallVector stores up to 8 elements in an internal buffer. What is the memory
    // Q: layout of this buffer?
    // A:
    // R:

    // Q: When the vector grows beyond 8 elements, it allocates on the heap. What happens
    // Q: to the elements in the buffer?
    // A:
    // R:

    // Q: What is the memory overhead of SmallVector compared to std::vector?
    // A:
    // R:
}

// ============================================================================
// TEST 5: TODO - Implement SSO for Custom String Class - Hard
// ============================================================================

// TODO: Implement a SmallString class with:
// TODO: 1. Internal buffer for strings up to 15 characters
// TODO: 2. Heap allocation for longer strings
// TODO: 3. Proper copy/move semantics
// TODO: 4. Measure performance vs std::string

TEST_F(SmallObjectOptimizationTest, DISABLED_CustomSSOString)
{
    // TODO: Implement SmallString with SSO
    // TODO: Test small vs large string behavior
    // TODO: Verify no heap allocation for small strings

    // Q: How does SmallString determine whether to use the buffer or heap?
    // A:
    // R:
}

// ============================================================================
// TEST 6: SSO and Move Semantics - Moderate
// ============================================================================

TEST_F(SmallObjectOptimizationTest, SSOAndMoveSemantics)
{
    std::string small1 = "short";
    const char* small1_ptr = small1.data();

    std::string small2 = std::move(small1);
    const char* small2_ptr = small2.data();

    std::string large1 = "this is a very long string that exceeds the SSO buffer size";
    const char* large1_ptr = large1.data();

    std::string large2 = std::move(large1);
    const char* large2_ptr = large2.data();

    // Q: When moving a small string, does the data pointer change? What does this
    // Q: indicate about the move operation?
    // A:
    // R:

    // Q: When moving a large string, does the data pointer stay the same? What does
    // Q: this indicate about heap ownership transfer?
    // A:
    // R:

    // Q: Why might moving a small string be slower than moving a large string?
    // A:
    // R:

    EXPECT_EQ(large2_ptr, large1_ptr);
    EXPECT_EQ(small2, "short");
    EXPECT_EQ(large2, "this is a very long string that exceeds the SSO buffer size");
}
