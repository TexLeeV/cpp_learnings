// Test Suite: Performance Testing and Benchmarking
// Estimated Time: 3 hours
// Difficulty: Easy
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>

class BenchmarkTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic Timing Measurement - Easy
// ============================================================================

TEST_F(BenchmarkTest, BasicTimingMeasurement)
{
    std::vector<int> data(10000);
    std::iota(data.begin(), data.end(), 0);

    auto start = std::chrono::high_resolution_clock::now();

    int sum = std::accumulate(data.begin(), data.end(), 0);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    EventLog::instance().record("Accumulate took " + std::to_string(duration) + " microseconds");

    EXPECT_EQ(sum, 49995000);

    // Q: Why use high_resolution_clock instead of system_clock? What timing precision
    // Q: difference do they provide?
    // A:
    // R:

    // Q: If you run this test multiple times, will the duration be exactly the same?
    // Q: What factors cause timing variability?
    // A:
    // R:

    EXPECT_GT(duration, 0);
}

// ============================================================================
// TEST 2: Comparing Algorithm Performance - Moderate
// ============================================================================

TEST_F(BenchmarkTest, ComparingAlgorithmPerformance)
{
    constexpr size_t size = 100000;
    std::vector<int> data1(size);
    std::vector<int> data2(size);

    std::iota(data1.begin(), data1.end(), 0);
    std::iota(data2.begin(), data2.end(), 0);

    auto start1 = std::chrono::high_resolution_clock::now();
    std::reverse(data1.begin(), data1.end());
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

    auto start2 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < data2.size() / 2; ++i)
    {
        std::swap(data2[i], data2[data2.size() - 1 - i]);
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();

    EventLog::instance().record("std::reverse took " + std::to_string(duration1) + " us");
    EventLog::instance().record("manual swap took " + std::to_string(duration2) + " us");

    EXPECT_EQ(data1, data2);

    // Q: std::reverse is likely optimized by the standard library. What techniques might
    // Q: it use to be faster than the manual swap loop?
    // A:
    // R:

    // Q: To get statistically significant results, how many iterations should you run?
    // Q: What statistical measure (mean, median, percentiles) is most reliable?
    // A:
    // R:
}

// ============================================================================
// TEST 3: Cache Effects on Performance - Hard
// ============================================================================

struct CacheFriendly
{
    int data[8];
};

struct CacheUnfriendly
{
    int data;
    char padding[60];
};

TEST_F(BenchmarkTest, CacheEffectsOnPerformance)
{
    constexpr size_t count = 100000;

    std::vector<CacheFriendly> friendly(count);
    std::vector<CacheUnfriendly> unfriendly(count);

    auto start1 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i)
    {
        friendly[i].data[0] = i;
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

    auto start2 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i)
    {
        unfriendly[i].data = i;
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();

    EventLog::instance().record("CacheFriendly took " + std::to_string(duration1) + " us");
    EventLog::instance().record("CacheUnfriendly took " + std::to_string(duration2) + " us");

    // Q: CacheUnfriendly has 60 bytes of padding. How does this affect cache line utilization?
    // Q: How many CacheUnfriendly objects fit in a 64-byte cache line vs CacheFriendly?
    // A:
    // R:

    // Q: Why might CacheUnfriendly be slower even though both loops do the same work?
    // Q: What memory access pattern causes more cache misses?
    // A:
    // R:

    EXPECT_GT(duration2, duration1 * 0.5);
}

// ============================================================================
// TEST 4: Move vs Copy Performance - Moderate
// ============================================================================

class LargeObject
{
public:
    explicit LargeObject(size_t size)
    : data_(size, 0)
    {
        EventLog::instance().record("LargeObject::ctor size=" + std::to_string(size));
    }

    LargeObject(const LargeObject& other)
    : data_(other.data_)
    {
        EventLog::instance().record("LargeObject::copy_ctor size=" + std::to_string(data_.size()));
    }

    LargeObject(LargeObject&& other) noexcept
    : data_(std::move(other.data_))
    {
        EventLog::instance().record("LargeObject::move_ctor");
    }

    size_t size() const { return data_.size(); }

private:
    std::vector<int> data_;
};

TEST_F(BenchmarkTest, MoveVsCopyPerformance)
{
    constexpr size_t object_size = 100000;
    constexpr int iterations = 100;

    std::vector<LargeObject> copy_results;
    copy_results.reserve(iterations);

    auto start_copy = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        LargeObject obj1(object_size);
        LargeObject obj2 = obj1;
        copy_results.push_back(std::move(obj2));
    }
    auto end_copy = std::chrono::high_resolution_clock::now();
    auto copy_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_copy - start_copy).count();

    EventLog::instance().clear();

    std::vector<LargeObject> move_results;
    move_results.reserve(iterations);

    auto start_move = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        LargeObject obj1(object_size);
        LargeObject obj2 = std::move(obj1);
        move_results.push_back(std::move(obj2));
    }
    auto end_move = std::chrono::high_resolution_clock::now();
    auto move_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_move - start_move).count();

    EventLog::instance().record("Copy: " + std::to_string(copy_duration) + " ms");
    EventLog::instance().record("Move: " + std::to_string(move_duration) + " ms");

    // Q: Move constructor transfers ownership of the vector's internal buffer. What is
    // Q: the time complexity of move vs copy for a vector of size N?
    // A:
    // R:

    // Q: The move is significantly faster. What observable signal in EventLog confirms
    // Q: that moves occurred instead of copies?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), iterations * 2);
    EXPECT_GT(copy_duration, move_duration);
}

// ============================================================================
// TEST 5: TODO - Implement Microbenchmark with Warmup - Moderate
// ============================================================================

// TODO: Implement a microbenchmark that:
// TODO: 1. Runs warmup iterations to stabilize CPU frequency
// TODO: 2. Runs multiple iterations and collects timing data
// TODO: 3. Computes mean, median, and standard deviation
// TODO: 4. Detects and reports outliers

class Microbenchmark
{
public:
    template<typename Func>
    void run(const std::string& name, Func func, int warmup_iters, int bench_iters)
    {
        // TODO: Run warmup iterations
        // TODO: Collect timing data for bench_iters
        // TODO: Compute statistics
        // TODO: Log results to EventLog
    }
};

TEST_F(BenchmarkTest, DISABLED_MicrobenchmarkWithWarmup)
{
    Microbenchmark bench;

    bench.run("vector_push_back", []()
    {
        std::vector<int> v;
        for (int i = 0; i < 1000; ++i)
        {
            v.push_back(i);
        }
    }, 10, 100);

    // Q: Why are warmup iterations necessary? What CPU/cache effects do they mitigate?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Memory Allocation Performance - Hard
// ============================================================================

TEST_F(BenchmarkTest, MemoryAllocationPerformance)
{
    constexpr int iterations = 10000;

    auto start_individual = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        int* ptr = new int(i);
        delete ptr;
    }
    auto end_individual = std::chrono::high_resolution_clock::now();
    auto individual_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_individual - start_individual).count();

    auto start_batch = std::chrono::high_resolution_clock::now();
    int* batch = new int[iterations];
    for (int i = 0; i < iterations; ++i)
    {
        batch[i] = i;
    }
    delete[] batch;
    auto end_batch = std::chrono::high_resolution_clock::now();
    auto batch_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_batch - start_batch).count();

    EventLog::instance().record("Individual allocations: " + std::to_string(individual_duration) + " us");
    EventLog::instance().record("Batch allocation: " + std::to_string(batch_duration) + " us");

    // Q: Individual allocations call new/delete 10,000 times. What overhead does each
    // Q: allocation incur compared to a single batch allocation?
    // A:
    // R:

    // Q: Batch allocation is contiguous in memory. What cache benefit does this provide
    // Q: during the initialization loop?
    // A:
    // R:

    EXPECT_LT(batch_duration, individual_duration);
}
