// Test Suite: Advanced Benchmarking Techniques
// Estimated Time: 3 hours
// Difficulty: Hard
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

class BenchmarkingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Statistical Benchmarking - Moderate
// ============================================================================

struct BenchmarkStats
{
    double mean;
    double median;
    double std_dev;
    double min;
    double max;
};

BenchmarkStats compute_stats(const std::vector<double>& samples)
{
    BenchmarkStats stats;

    stats.mean = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();

    std::vector<double> sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    stats.median = sorted[sorted.size() / 2];
    stats.min = sorted.front();
    stats.max = sorted.back();

    double variance = 0.0;
    for (double sample : samples)
    {
        variance += (sample - stats.mean) * (sample - stats.mean);
    }
    variance /= samples.size();
    stats.std_dev = std::sqrt(variance);

    return stats;
}

TEST_F(BenchmarkingTest, StatisticalBenchmarking)
{
    constexpr int iterations = 100;
    std::vector<double> samples;

    for (int i = 0; i < iterations; ++i)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<int> data(10000);
        std::iota(data.begin(), data.end(), 0);
        std::sort(data.begin(), data.end());

        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        samples.push_back(duration);
    }

    BenchmarkStats stats = compute_stats(samples);

    EventLog::instance().record("Mean: " + std::to_string(stats.mean) + " us");
    EventLog::instance().record("Median: " + std::to_string(stats.median) + " us");
    EventLog::instance().record("Std dev: " + std::to_string(stats.std_dev) + " us");

    // Q: Why is median often more reliable than mean for benchmarking?
    // A:
    // R:

    // Q: High standard deviation indicates variability. What factors cause timing
    // Q: variability in benchmarks?
    // A:
    // R:

    EXPECT_GT(stats.mean, 0);
    EXPECT_GT(stats.median, 0);
}

// ============================================================================
// TEST 2: Warmup Iterations - Moderate
// ============================================================================

TEST_F(BenchmarkingTest, WarmupIterations)
{
    constexpr int warmup = 10;
    constexpr int bench = 50;

    std::vector<double> warmup_samples;
    std::vector<double> bench_samples;

    for (int i = 0; i < warmup; ++i)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<int> data(50000);
        std::iota(data.begin(), data.end(), 0);
        auto end = std::chrono::high_resolution_clock::now();
        warmup_samples.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }

    for (int i = 0; i < bench; ++i)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<int> data(50000);
        std::iota(data.begin(), data.end(), 0);
        auto end = std::chrono::high_resolution_clock::now();
        bench_samples.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }

    BenchmarkStats warmup_stats = compute_stats(warmup_samples);
    BenchmarkStats bench_stats = compute_stats(bench_samples);

    EventLog::instance().record("Warmup mean: " + std::to_string(warmup_stats.mean) + " us");
    EventLog::instance().record("Bench mean: " + std::to_string(bench_stats.mean) + " us");

    // Q: Warmup iterations stabilize CPU frequency and cache state. What observable
    // Q: difference do you expect between warmup and benchmark means?
    // A:
    // R:

    // Q: How many warmup iterations are sufficient? What factors determine this?
    // A:
    // R:
}

// ============================================================================
// TEST 3: Compiler Optimization Effects - Hard
// ============================================================================

volatile int sink = 0;

int compute_sum_naive(const std::vector<int>& data)
{
    int sum = 0;
    for (int val : data)
    {
        sum += val;
    }
    return sum;
}

int compute_sum_stl(const std::vector<int>& data)
{
    return std::accumulate(data.begin(), data.end(), 0);
}

TEST_F(BenchmarkingTest, CompilerOptimizationEffects)
{
    std::vector<int> data(100000);
    std::iota(data.begin(), data.end(), 0);

    auto start_naive = std::chrono::high_resolution_clock::now();
    int result_naive = compute_sum_naive(data);
    sink = result_naive;
    auto end_naive = std::chrono::high_resolution_clock::now();
    auto naive_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_naive - start_naive).count();

    auto start_stl = std::chrono::high_resolution_clock::now();
    int result_stl = compute_sum_stl(data);
    sink = result_stl;
    auto end_stl = std::chrono::high_resolution_clock::now();
    auto stl_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_stl - start_stl).count();

    EventLog::instance().record("Naive sum: " + std::to_string(naive_duration) + " us");
    EventLog::instance().record("STL sum: " + std::to_string(stl_duration) + " us");

    EXPECT_EQ(result_naive, result_stl);

    // Q: We assign results to volatile sink. Why is this necessary to prevent compiler
    // Q: from optimizing away the computation?
    // A:
    // R:

    // Q: The compiler may auto-vectorize these loops. How would you verify vectorization
    // Q: occurred? (Hint: assembly inspection, compiler flags)
    // A:
    // R:
}

// ============================================================================
// TEST 4: Microbenchmark Pitfalls - Hard
// ============================================================================

TEST_F(BenchmarkingTest, MicrobenchmarkPitfalls)
{
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 0);

    constexpr int outer_iterations = 100;
    constexpr int inner_iterations = 1000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int outer = 0; outer < outer_iterations; ++outer)
    {
        for (int inner = 0; inner < inner_iterations; ++inner)
        {
            volatile int val = data[inner % data.size()];
            (void)val;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    EventLog::instance().record("Microbenchmark duration: " + std::to_string(duration) + " us");

    // Q: This benchmark accesses data in a tight loop. What effect does this have on
    // Q: cache hit rate compared to real-world usage?
    // A:
    // R:

    // Q: Microbenchmarks can be misleading. What real-world factors are missing from
    // Q: this test? (Hint: cache pollution, branch misprediction, memory pressure)
    // A:
    // R:

    EXPECT_GT(duration, 0);
}

// ============================================================================
// TEST 5: TODO - Implement Benchmark Comparison Framework - Hard
// ============================================================================

// TODO: Implement a BenchmarkRunner class that:
// TODO: 1. Runs multiple benchmark functions
// TODO: 2. Computes statistics for each
// TODO: 3. Compares results and reports speedup/slowdown
// TODO: 4. Detects statistically significant differences

TEST_F(BenchmarkingTest, DISABLED_BenchmarkComparisonFramework)
{
    // TODO: Implement BenchmarkRunner
    // TODO: Compare two algorithms
    // TODO: Report speedup with confidence interval

    // Q: When comparing benchmarks, how do you determine if the difference is
    // Q: statistically significant?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Benchmark Stability and Outliers - Moderate
// ============================================================================

TEST_F(BenchmarkingTest, BenchmarkStabilityAndOutliers)
{
    constexpr int iterations = 100;
    std::vector<double> samples;

    for (int i = 0; i < iterations; ++i)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<int> data(10000);
        for (size_t j = 0; j < data.size(); ++j)
        {
            data[j] = j * j;
        }

        auto end = std::chrono::high_resolution_clock::now();
        samples.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    }

    BenchmarkStats stats = compute_stats(samples);

    double outlier_threshold = stats.mean + 3 * stats.std_dev;
    int outlier_count = 0;
    for (double sample : samples)
    {
        if (sample > outlier_threshold)
        {
            outlier_count++;
        }
    }

    EventLog::instance().record("Outliers: " + std::to_string(outlier_count) + " / " + std::to_string(iterations));

    // Q: Outliers can skew benchmark results. What causes outliers? (Hint: OS scheduling,
    // Q: interrupts, cache misses)
    // A:
    // R:

    // Q: Should you remove outliers from benchmark results? What are the tradeoffs?
    // A:
    // R:

    EXPECT_LT(outlier_count, iterations / 10);
}

// ============================================================================
// TEST 7: Benchmark Iteration Count Selection - Easy
// ============================================================================

TEST_F(BenchmarkingTest, IterationCountSelection)
{
    auto measure_operation = [](int iterations) -> double
    {
        std::vector<int> data(1000);
        std::iota(data.begin(), data.end(), 0);

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i)
        {
            std::sort(data.begin(), data.end());
            std::reverse(data.begin(), data.end());
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    double duration_10 = measure_operation(10);
    double duration_100 = measure_operation(100);
    double duration_1000 = measure_operation(1000);

    EventLog::instance().record("10 iterations: " + std::to_string(duration_10) + " us");
    EventLog::instance().record("100 iterations: " + std::to_string(duration_100) + " us");
    EventLog::instance().record("1000 iterations: " + std::to_string(duration_1000) + " us");

    // Q: Very fast operations (< 1us) are hard to measure accurately. How many iterations
    // Q: are needed to get reliable timing?
    // A:
    // R:

    // Q: What is the minimum measurable duration for high_resolution_clock on your system?
    // A:
    // R:

    EXPECT_GT(duration_1000, duration_100);
    EXPECT_GT(duration_100, duration_10);
}
