// Test Suite: Performance Profiling and Analysis
// Estimated Time: 3 hours
// Difficulty: Moderate to Hard


#include <gtest/gtest.h>
#include "instrumentation.h"
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>
#include <unordered_map>

class ProfilingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic Profiling with EventLog - Easy
// ============================================================================

void expensive_operation(int n)
{
    EventLog::instance().record("expensive_operation: start");
    std::vector<int> data(n);
    std::iota(data.begin(), data.end(), 0);
    std::sort(data.begin(), data.end(), std::greater<int>());
    EventLog::instance().record("expensive_operation: end");
}

TEST_F(ProfilingTest, BasicProfilingWithEventLog)
{
    auto start = std::chrono::high_resolution_clock::now();
    expensive_operation(100000);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    EventLog::instance().record("Total time: " + std::to_string(duration) + " ms");

    // Q: EventLog records entry and exit points. How can you use this to identify
    // Q: performance bottlenecks?
    // A:
    // R:

    // Q: What are the limitations of manual instrumentation compared to sampling profilers?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("expensive_operation"), 2);
    EXPECT_GT(duration, 0);
}

// ============================================================================
// TEST 2: Hotspot Identification - Moderate
// ============================================================================

int compute_fibonacci(int n)
{
    if (n <= 1) return n;
    return compute_fibonacci(n - 1) + compute_fibonacci(n - 2);
}

int compute_fibonacci_optimized(int n)
{
    if (n <= 1) return n;
    int prev = 0, curr = 1;
    for (int i = 2; i <= n; ++i)
    {
        int next = prev + curr;
        prev = curr;
        curr = next;
    }
    return curr;
}

TEST_F(ProfilingTest, HotspotIdentification)
{
    constexpr int n = 30;

    auto start_naive = std::chrono::high_resolution_clock::now();
    int result_naive = compute_fibonacci(n);
    auto end_naive = std::chrono::high_resolution_clock::now();
    auto naive_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_naive - start_naive).count();

    auto start_opt = std::chrono::high_resolution_clock::now();
    int result_opt = compute_fibonacci_optimized(n);
    auto end_opt = std::chrono::high_resolution_clock::now();
    auto opt_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_opt - start_opt).count();

    EventLog::instance().record("Naive fibonacci: " + std::to_string(naive_duration) + " ms");
    EventLog::instance().record("Optimized fibonacci: " + std::to_string(opt_duration) + " us");

    EXPECT_EQ(result_naive, result_opt);

    // Q: The naive version is exponentially slower. What profiling technique would
    // Q: identify the recursive calls as the hotspot?
    // A:
    // R:

    // Q: The optimized version is linear. What is the time complexity difference?
    // A:
    // R:

    EXPECT_GT(naive_duration * 1000, opt_duration);
}

// ============================================================================
// TEST 3: Memory Allocation Profiling - Moderate
// ============================================================================

class AllocationTracker
{
public:
    static void record_allocation(size_t bytes)
    {
        total_bytes_ += bytes;
        allocation_count_++;
        EventLog::instance().record("Allocation: " + std::to_string(bytes) + " bytes");
    }

    static void reset()
    {
        total_bytes_ = 0;
        allocation_count_ = 0;
    }

    static size_t total_bytes() { return total_bytes_; }
    static size_t allocation_count() { return allocation_count_; }

private:
    static size_t total_bytes_;
    static size_t allocation_count_;
};

size_t AllocationTracker::total_bytes_ = 0;
size_t AllocationTracker::allocation_count_ = 0;

void allocate_many_small()
{
    AllocationTracker::reset();
    for (int i = 0; i < 1000; ++i)
    {
        int* ptr = new int(i);
        AllocationTracker::record_allocation(sizeof(int));
        delete ptr;
    }
}

void allocate_one_large()
{
    AllocationTracker::reset();
    int* ptr = new int[1000];
    AllocationTracker::record_allocation(1000 * sizeof(int));
    delete[] ptr;
}

TEST_F(ProfilingTest, MemoryAllocationProfiling)
{
    allocate_many_small();
    size_t small_allocs = AllocationTracker::allocation_count();

    EventLog::instance().clear();

    allocate_one_large();
    size_t large_allocs = AllocationTracker::allocation_count();

    // Q: allocate_many_small makes 1000 allocations. What is the overhead per allocation
    // Q: compared to one large allocation?
    // A:
    // R:

    // Q: How would you use a memory profiler (e.g., valgrind, heaptrack) to identify
    // Q: allocation hotspots?
    // A:
    // R:

    EXPECT_EQ(small_allocs, 1000);
    EXPECT_EQ(large_allocs, 1);
}

// ============================================================================
// TEST 4: Container Performance Profiling - Moderate
// ============================================================================

TEST_F(ProfilingTest, ContainerPerformanceProfiling)
{
    constexpr int operations = 10000;

    std::vector<int> vec;
    auto start_vec = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < operations; ++i)
    {
        vec.push_back(i);
    }
    auto end_vec = std::chrono::high_resolution_clock::now();
    auto vec_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_vec - start_vec).count();

    std::map<int, int> map;
    auto start_map = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < operations; ++i)
    {
        map[i] = i;
    }
    auto end_map = std::chrono::high_resolution_clock::now();
    auto map_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_map - start_map).count();

    std::unordered_map<int, int> umap;
    auto start_umap = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < operations; ++i)
    {
        umap[i] = i;
    }
    auto end_umap = std::chrono::high_resolution_clock::now();
    auto umap_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_umap - start_umap).count();

    EventLog::instance().record("vector push_back: " + std::to_string(vec_duration) + " us");
    EventLog::instance().record("map insert: " + std::to_string(map_duration) + " us");
    EventLog::instance().record("unordered_map insert: " + std::to_string(umap_duration) + " us");

    // Q: vector has O(1) amortized push_back. What causes the amortization?
    // A:
    // R:

    // Q: map has O(log n) insert. Why is it slower than unordered_map's O(1)?
    // A:
    // R:

    // Q: When profiling, you observe map is the bottleneck. What container would you
    // Q: switch to and why?
    // A:
    // R:

    EXPECT_LT(vec_duration, map_duration);
    EXPECT_LT(umap_duration, map_duration);
}

// ============================================================================
// TEST 5: TODO - Implement Custom Profiling Scope Guard - Moderate
// ============================================================================

// TODO: Implement a ScopeTimer class that:
// TODO: 1. Records start time in constructor
// TODO: 2. Records duration in destructor
// TODO: 3. Logs to EventLog with function name
// TODO: 4. Use RAII to ensure timing even with exceptions

TEST_F(ProfilingTest, DISABLED_CustomProfilingScopeGuard)
{
    // TODO: Implement ScopeTimer
    // TODO: Test with normal execution
    // TODO: Test with exception thrown

    // Q: How does RAII ensure timing is recorded even when exceptions occur?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Profiling with Call Counts - Easy
// ============================================================================

int call_count = 0;

int expensive_function(int x)
{
    call_count++;
    int result = 0;
    for (int i = 0; i < x; ++i)
    {
        result += i * i;
    }
    return result;
}

TEST_F(ProfilingTest, ProfilingWithCallCounts)
{
    call_count = 0;

    for (int i = 0; i < 100; ++i)
    {
        expensive_function(1000);
    }

    EventLog::instance().record("expensive_function called " + std::to_string(call_count) + " times");

    // Q: Call count profiling identifies hot functions. What additional metric would
    // Q: help prioritize optimization? (Hint: time per call)
    // A:
    // R:

    // Q: If expensive_function is called 100 times but takes 1us each, vs another
    // Q: function called once taking 1ms, which should you optimize first?
    // A:
    // R:

    EXPECT_EQ(call_count, 100);
}

// ============================================================================
// TEST 7: Profiling Allocation Patterns - Hard
// ============================================================================

TEST_F(ProfilingTest, ProfilingAllocationPatterns)
{
    constexpr int iterations = 1000;

    auto start_vector = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        std::vector<int> v;
        for (int j = 0; j < 100; ++j)
        {
            v.push_back(j);
        }
    }
    auto end_vector = std::chrono::high_resolution_clock::now();
    auto vector_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_vector - start_vector).count();

    auto start_reserved = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        std::vector<int> v;
        v.reserve(100);
        for (int j = 0; j < 100; ++j)
        {
            v.push_back(j);
        }
    }
    auto end_reserved = std::chrono::high_resolution_clock::now();
    auto reserved_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_reserved - start_reserved).count();

    EventLog::instance().record("Without reserve: " + std::to_string(vector_duration) + " us");
    EventLog::instance().record("With reserve: " + std::to_string(reserved_duration) + " us");

    // Q: Without reserve, vector reallocates multiple times. How many reallocations
    // Q: occur when growing from 0 to 100 elements?
    // A:
    // R:

    // Q: With reserve, vector allocates once. What is the memory and performance tradeoff?
    // A:
    // R:

    // Q: A profiler shows high allocation overhead. What pattern in the code suggests
    // Q: adding reserve() calls?
    // A:
    // R:

    EXPECT_LT(reserved_duration, vector_duration);
}
