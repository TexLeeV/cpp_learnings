// Test Suite: Cache-Friendly Data Structures
// Estimated Time: 3 hours
// Difficulty: Moderate to Hard


#include <gtest/gtest.h>
#include "instrumentation.h"
#include <vector>
#include <list>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <random>

class CacheFriendlyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Array of Structs vs Struct of Arrays - Moderate
// ============================================================================

struct ParticleAoS
{
    float x, y, z;
    float vx, vy, vz;
    float mass;
    int id;
};

struct ParticlesSoA
{
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    std::vector<int> id;

    void resize(size_t n)
    {
        x.resize(n); y.resize(n); z.resize(n);
        vx.resize(n); vy.resize(n); vz.resize(n);
        mass.resize(n);
        id.resize(n);
    }

    size_t size() const { return x.size(); }
};

TEST_F(CacheFriendlyTest, ArrayOfStructsVsStructOfArrays)
{
    constexpr size_t count = 100000;

    std::vector<ParticleAoS> particles_aos(count);
    for (size_t i = 0; i < count; ++i)
    {
        particles_aos[i] = {1.0f, 2.0f, 3.0f, 0.1f, 0.2f, 0.3f, 1.0f, static_cast<int>(i)};
    }

    ParticlesSoA particles_soa;
    particles_soa.resize(count);
    for (size_t i = 0; i < count; ++i)
    {
        particles_soa.x[i] = 1.0f;
        particles_soa.y[i] = 2.0f;
        particles_soa.z[i] = 3.0f;
        particles_soa.vx[i] = 0.1f;
        particles_soa.vy[i] = 0.2f;
        particles_soa.vz[i] = 0.3f;
        particles_soa.mass[i] = 1.0f;
        particles_soa.id[i] = static_cast<int>(i);
    }

    auto start_aos = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i)
    {
        particles_aos[i].x += particles_aos[i].vx;
        particles_aos[i].y += particles_aos[i].vy;
        particles_aos[i].z += particles_aos[i].vz;
    }
    auto end_aos = std::chrono::high_resolution_clock::now();
    auto aos_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_aos - start_aos).count();

    auto start_soa = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i)
    {
        particles_soa.x[i] += particles_soa.vx[i];
        particles_soa.y[i] += particles_soa.vy[i];
        particles_soa.z[i] += particles_soa.vz[i];
    }
    auto end_soa = std::chrono::high_resolution_clock::now();
    auto soa_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_soa - start_soa).count();

    EventLog::instance().record("AoS update took " + std::to_string(aos_duration) + " us");
    EventLog::instance().record("SoA update took " + std::to_string(soa_duration) + " us");

    // Q: ParticleAoS is 32 bytes. When updating position, how many bytes of unused data
    // Q: (mass, id) are loaded into cache lines?
    // A:
    // R:

    // Q: SoA stores positions contiguously. How does this affect cache line utilization
    // Q: when iterating over x, y, z arrays?
    // A:
    // R:

    // Q: Which layout (AoS or SoA) is better for this access pattern? What if you needed
    // Q: to access all fields of a single particle instead?
    // A:
    // R:

    EXPECT_GT(aos_duration, 0);
    EXPECT_GT(soa_duration, 0);
}

// ============================================================================
// TEST 2: Cache Line Alignment and Padding - Hard
// ============================================================================

struct Unaligned
{
    char a;
    int b;
    char c;
    long d;
};

struct alignas(64) CacheLineAligned
{
    char a;
    int b;
    char c;
    long d;
};

TEST_F(CacheFriendlyTest, CacheLineAlignmentAndPadding)
{
    EXPECT_EQ(sizeof(Unaligned), 24);
    EXPECT_EQ(sizeof(CacheLineAligned), 64);
    EXPECT_EQ(alignof(CacheLineAligned), 64);

    EventLog::instance().record("Unaligned size: " + std::to_string(sizeof(Unaligned)));
    EventLog::instance().record("CacheLineAligned size: " + std::to_string(sizeof(CacheLineAligned)));

    // Q: Unaligned has 4 fields totaling ~14 bytes, but sizeof is 24. Where are the
    // Q: extra 10 bytes?
    // A:
    // R:

    // Q: CacheLineAligned is padded to 64 bytes. If two threads access different
    // Q: CacheLineAligned objects in an array, can they share a cache line?
    // A:
    // R:

    // Q: What is false sharing? How does alignas(64) prevent it?
    // A:
    // R:
}

// ============================================================================
// TEST 3: Data-Oriented Design - Loop Fusion - Moderate
// ============================================================================

struct Transform
{
    float x, y, z;
    float scale;
};

TEST_F(CacheFriendlyTest, LoopFusionOptimization)
{
    constexpr size_t count = 100000;
    std::vector<Transform> transforms(count);

    for (size_t i = 0; i < count; ++i)
    {
        transforms[i] = {static_cast<float>(i), static_cast<float>(i * 2),
                         static_cast<float>(i * 3), 1.0f};
    }

    auto start_separate = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i)
    {
        transforms[i].x *= transforms[i].scale;
    }
    for (size_t i = 0; i < count; ++i)
    {
        transforms[i].y *= transforms[i].scale;
    }
    for (size_t i = 0; i < count; ++i)
    {
        transforms[i].z *= transforms[i].scale;
    }
    auto end_separate = std::chrono::high_resolution_clock::now();
    auto separate_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_separate - start_separate).count();

    for (size_t i = 0; i < count; ++i)
    {
        transforms[i] = {static_cast<float>(i), static_cast<float>(i * 2),
                         static_cast<float>(i * 3), 1.0f};
    }

    auto start_fused = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; ++i)
    {
        transforms[i].x *= transforms[i].scale;
        transforms[i].y *= transforms[i].scale;
        transforms[i].z *= transforms[i].scale;
    }
    auto end_fused = std::chrono::high_resolution_clock::now();
    auto fused_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_fused - start_fused).count();

    EventLog::instance().record("Separate loops: " + std::to_string(separate_duration) + " us");
    EventLog::instance().record("Fused loop: " + std::to_string(fused_duration) + " us");

    // Q: Separate loops iterate over the array three times. How many times is each
    // Q: Transform loaded from memory?
    // A:
    // R:

    // Q: Fused loop iterates once. How does this improve cache locality?
    // A:
    // R:

    EXPECT_GT(separate_duration, 0);
    EXPECT_GT(fused_duration, 0);
}

// ============================================================================
// TEST 4: Prefetching and Sequential Access - Hard
// ============================================================================

TEST_F(CacheFriendlyTest, SequentialVsRandomAccess)
{
    constexpr size_t size = 1000000;
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 0);

    std::vector<size_t> sequential_indices(size);
    std::iota(sequential_indices.begin(), sequential_indices.end(), 0);

    std::vector<size_t> random_indices = sequential_indices;
    std::random_device rd;
    std::mt19937 gen(42);
    std::shuffle(random_indices.begin(), random_indices.end(), gen);

    constexpr int iterations = 5;

    long long total_seq = 0;
    auto start_seq = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter)
    {
        long long sum = 0;
        for (size_t idx : sequential_indices)
        {
            sum += data[idx];
        }
        total_seq += sum;
    }
    auto end_seq = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_seq - start_seq).count();

    long long total_rand = 0;
    auto start_rand = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter)
    {
        long long sum = 0;
        for (size_t idx : random_indices)
        {
            sum += data[idx];
        }
        total_rand += sum;
    }
    auto end_rand = std::chrono::high_resolution_clock::now();
    auto rand_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_rand - start_rand).count();

    EventLog::instance().record("Sequential access: " + std::to_string(seq_duration) + " ms");
    EventLog::instance().record("Random access: " + std::to_string(rand_duration) + " ms");

    EXPECT_EQ(total_seq, total_rand);

    // Q: Sequential access allows hardware prefetching. What pattern does the prefetcher
    // Q: detect to load cache lines ahead of time?
    // A:
    // R:

    // Q: Random access defeats prefetching. How many cache misses occur per access
    // Q: compared to sequential?
    // A:
    // R:

    EXPECT_GT(rand_duration, seq_duration);
}

// ============================================================================
// TEST 5: TODO - Implement Cache-Oblivious Algorithm - Hard
// ============================================================================

// TODO: Implement a cache-oblivious matrix transpose that:
// TODO: 1. Uses recursive divide-and-conquer
// TODO: 2. Works efficiently regardless of cache size
// TODO: 3. Compare against naive row-by-row transpose

TEST_F(CacheFriendlyTest, DISABLED_CacheObliviousTranspose)
{
    constexpr size_t N = 1024;
    std::vector<std::vector<int>> matrix(N, std::vector<int>(N));

    // TODO: Implement cache-oblivious transpose
    // TODO: Compare against naive transpose
    // TODO: Measure performance difference

    // Q: Cache-oblivious algorithms adapt to any cache size. How does recursive
    // Q: subdivision achieve this?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Memory Bandwidth and Vectorization - Hard
// ============================================================================

TEST_F(CacheFriendlyTest, MemoryBandwidthVectorization)
{
    constexpr size_t size = 1000000;
    std::vector<float> a(size, 1.0f);
    std::vector<float> b(size, 2.0f);
    std::vector<float> c(size);

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < size; ++i)
    {
        c[i] = a[i] + b[i];
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    EventLog::instance().record("Vector addition took " + std::to_string(duration) + " us");

    float sum = std::accumulate(c.begin(), c.end(), 0.0f);
    EXPECT_FLOAT_EQ(sum, 3.0f * size);

    // Q: This loop reads 8MB (a + b) and writes 4MB (c). What limits performance:
    // Q: CPU computation or memory bandwidth?
    // A:
    // R:

    // Q: Modern compilers can auto-vectorize this loop using SIMD instructions. What
    // Q: observable signal would confirm vectorization occurred?
    // A:
    // R:

    // Q: If you unroll the loop manually (process 4 elements per iteration), how does
    // Q: this interact with vectorization?
    // A:
    // R:
}

// ============================================================================
// TEST 7: Cache-Friendly Container Choice - Moderate
// ============================================================================

TEST_F(CacheFriendlyTest, ContainerChoiceForIteration)
{
    constexpr size_t count = 100000;

    std::vector<int> vec(count);
    std::iota(vec.begin(), vec.end(), 0);

    auto start_vec = std::chrono::high_resolution_clock::now();
    long long sum_vec = 0;
    for (int val : vec)
    {
        sum_vec += val;
    }
    auto end_vec = std::chrono::high_resolution_clock::now();
    auto vec_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_vec - start_vec).count();

    std::list<int> lst;
    for (size_t i = 0; i < count; ++i)
    {
        lst.push_back(i);
    }

    auto start_list = std::chrono::high_resolution_clock::now();
    long long sum_list = 0;
    for (int val : lst)
    {
        sum_list += val;
    }
    auto end_list = std::chrono::high_resolution_clock::now();
    auto list_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_list - start_list).count();

    EventLog::instance().record("Vector iteration: " + std::to_string(vec_duration) + " us");
    EventLog::instance().record("List iteration: " + std::to_string(list_duration) + " us");

    EXPECT_EQ(sum_vec, sum_list);

    // Q: std::vector stores elements contiguously. How many cache misses occur when
    // Q: iterating sequentially?
    // A:
    // R:

    // Q: std::list nodes are scattered in memory. What is the cache miss rate for
    // Q: list iteration?
    // A:
    // R:

    // Q: When is std::list preferable despite worse cache performance?
    // A:
    // R:

    EXPECT_GT(list_duration, vec_duration);
}

// ============================================================================
// TEST 8: Branch Prediction and Data Dependencies - Hard
// ============================================================================

TEST_F(CacheFriendlyTest, BranchPredictionImpact)
{
    constexpr size_t size = 1000000;
    std::vector<int> data(size);
    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, 255);

    for (size_t i = 0; i < size; ++i)
    {
        data[i] = dis(gen);
    }

    std::vector<int> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    constexpr int iterations = 10;

    long long total_unsorted = 0;
    auto start_unsorted = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter)
    {
        long long sum = 0;
        for (int val : data)
        {
            if (val < 128)
            {
                sum += val;
            }
        }
        total_unsorted += sum;
    }
    auto end_unsorted = std::chrono::high_resolution_clock::now();
    auto unsorted_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_unsorted - start_unsorted).count();

    long long total_sorted = 0;
    auto start_sorted = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter)
    {
        long long sum = 0;
        for (int val : sorted_data)
        {
            if (val < 128)
            {
                sum += val;
            }
        }
        total_sorted += sum;
    }
    auto end_sorted = std::chrono::high_resolution_clock::now();
    auto sorted_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_sorted - start_sorted).count();

    EventLog::instance().record("Unsorted branch: " + std::to_string(unsorted_duration) + " ms");
    EventLog::instance().record("Sorted branch: " + std::to_string(sorted_duration) + " ms");

    EXPECT_EQ(total_unsorted, total_sorted);

    // Q: The sorted data produces a predictable branch pattern (all true, then all false).
    // Q: How does the CPU's branch predictor exploit this?
    // A:
    // R:

    // Q: Unsorted data has random branches. What penalty does a branch misprediction incur?
    // A:
    // R:

    EXPECT_LT(sorted_duration, unsorted_duration);
}
