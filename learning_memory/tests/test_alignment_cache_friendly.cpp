// Test Suite: Memory Alignment and Cache-Friendly Structures
// Estimated Time: 4 hours
// Difficulty: Hard

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <chrono>

class AlignmentCacheFriendlyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Memory Alignment Basics (Easy)
// ============================================================================

struct UnalignedData
{
    char a;
    int b;
    char c;
};

struct AlignedData
{
    int b;
    char a;
    char c;
};

TEST_F(AlignmentCacheFriendlyTest, AlignmentAndPadding)
{
    // Q: What is the sizeof(UnalignedData)?
    // A:
    // R:
    
    // Q: What is the sizeof(AlignedData)?
    // A:
    // R:
    
    EXPECT_EQ(sizeof(UnalignedData), 12);
    EXPECT_EQ(sizeof(AlignedData), 8);
    
    // Q: Why is UnalignedData larger despite having the same members?
    // A:
    // R:
    
    // Q: What is the alignment requirement of int on most platforms?
    // A:
    // R:
    
    EXPECT_EQ(alignof(int), 4);
    EXPECT_EQ(alignof(UnalignedData), 4);
    EXPECT_EQ(alignof(AlignedData), 4);
}

// ============================================================================
// Scenario 2: Custom Alignment with alignas (Moderate)
// ============================================================================

struct CacheLineAligned
{
    alignas(64) int data;
    
    // Q: What does alignas(64) guarantee?
    // A:
    // R:
};

struct OverAligned
{
    alignas(128) char data[16];
};

TEST_F(AlignmentCacheFriendlyTest, CustomAlignment)
{
    CacheLineAligned obj1;
    CacheLineAligned obj2;
    
    // Q: What is the sizeof(CacheLineAligned)?
    // A:
    // R:
    
    EXPECT_EQ(sizeof(CacheLineAligned), 64);
    EXPECT_EQ(alignof(CacheLineAligned), 64);
    
    // Q: Why might we align data to 64 bytes?
    // A:
    // R:
    
    // Q: What is the sizeof(OverAligned)?
    // A:
    // R:
    
    EXPECT_EQ(sizeof(OverAligned), 128);
    EXPECT_EQ(alignof(OverAligned), 128);
    
    // Q: What is the memory cost of over-alignment?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Array of Structures vs Structure of Arrays (Hard)
// ============================================================================

struct Point3D_AoS
{
    float x, y, z;
};

class Points_AoS
{
public:
    void add(float x, float y, float z)
    {
        points_.push_back({x, y, z});
    }
    
    float sum_x() const
    {
        float sum = 0.0f;
        for (const auto& p : points_)
        {
            sum += p.x;
        }
        return sum;
    }
    
    size_t size() const
    {
        return points_.size();
    }
    
private:
    std::vector<Point3D_AoS> points_;
};

class Points_SoA
{
public:
    void add(float x, float y, float z)
    {
        x_.push_back(x);
        y_.push_back(y);
        z_.push_back(z);
    }
    
    float sum_x() const
    {
        float sum = 0.0f;
        for (float x : x_)
        {
            sum += x;
        }
        return sum;
    }
    
    size_t size() const
    {
        return x_.size();
    }
    
private:
    std::vector<float> x_;
    std::vector<float> y_;
    std::vector<float> z_;
};

TEST_F(AlignmentCacheFriendlyTest, AoS_vs_SoA_Structure)
{
    Points_AoS aos;
    Points_SoA soa;
    
    // Q: What is the memory layout difference between AoS and SoA?
    // A:
    // R:
    
    for (int i = 0; i < 100; ++i)
    {
        aos.add(i * 1.0f, i * 2.0f, i * 3.0f);
        soa.add(i * 1.0f, i * 2.0f, i * 3.0f);
    }
    
    EXPECT_EQ(aos.size(), 100);
    EXPECT_EQ(soa.size(), 100);
    
    // Q: When iterating to sum only x values, which layout is more cache-friendly?
    // A:
    // R:
    
    // Q: What is the cache miss pattern for AoS when accessing only x values?
    // A:
    // R:
    
    float aos_sum = aos.sum_x();
    float soa_sum = soa.sum_x();
    
    EXPECT_FLOAT_EQ(aos_sum, soa_sum);
    
    // Q: In what scenario would AoS be more cache-friendly than SoA?
    // A:
    // R:
}

// ============================================================================
// Scenario 4: False Sharing (Hard)
// ============================================================================

struct SharedCacheLine
{
    int counter1;
    int counter2;
};

struct SeparateCacheLines
{
    alignas(64) int counter1;
    alignas(64) int counter2;
};

TEST_F(AlignmentCacheFriendlyTest, FalseSharing)
{
    // Q: What is false sharing?
    // A:
    // R:
    
    SharedCacheLine shared;
    shared.counter1 = 0;
    shared.counter2 = 0;
    
    // Q: What is the memory offset between counter1 and counter2 in SharedCacheLine?
    // A:
    // R:
    
    EXPECT_EQ(offsetof(SharedCacheLine, counter2) - offsetof(SharedCacheLine, counter1), 
              sizeof(int));
    
    SeparateCacheLines separate;
    separate.counter1 = 0;
    separate.counter2 = 0;
    
    // Q: What is the memory offset between counter1 and counter2 in SeparateCacheLines?
    // A:
    // R:
    
    EXPECT_EQ(offsetof(SeparateCacheLines, counter2) - offsetof(SeparateCacheLines, counter1), 
              64);
    
    // Q: Why would separate cache lines improve performance in multi-threaded scenarios?
    // A:
    // R:
    
    // Q: What is the trade-off of using alignas(64) to prevent false sharing?
    // A:
    // R:
}

// ============================================================================
// Scenario 5: Alignment and Dynamic Allocation (Moderate)
// ============================================================================

TEST_F(AlignmentCacheFriendlyTest, AlignedAllocation)
{
    // Q: Does operator new guarantee alignment for over-aligned types?
    // A:
    // R:
    
    struct OverAligned
    {
        alignas(64) int data;
    };
    
    // TODO: Allocate an OverAligned object
    OverAligned* obj = new OverAligned();
    
    // Q: How can we verify that obj is properly aligned to 64 bytes?
    // A:
    // R:
    
    uintptr_t addr = reinterpret_cast<uintptr_t>(obj);
    EXPECT_EQ(addr % 64, 0);
    
    delete obj;
    
    // Q: In C++11/14, what happens if we use operator new for over-aligned types?
    // A:
    // R:
}
