// Test Suite: Containers (vector, map, unordered_map)
// Estimated Time: 1-2 hours
// Difficulty: Easy
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <map>
#include <unordered_map>
#include <vector>

class ContainersTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Capacity vs Size (Easy)
// ============================================================================

TEST_F(ContainersTest, VectorCapacityGrowsAheadOfSize)
{
    std::vector<int> v;
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);

    v.push_back(1);
    const auto cap_after_one = v.capacity();

    // Q: After one `push_back`, why can `capacity()` be greater than `size()`?
    // A:
    // R:

    EXPECT_EQ(v.size(), 1u);
    EXPECT_GE(cap_after_one, 1u);

    while (v.size() < cap_after_one)
    {
        v.push_back(static_cast<int>(v.size()) + 1);
    }
    const auto cap_before_growth = v.capacity();
    v.push_back(99);
    const auto cap_after_growth = v.capacity();

    // Q: What observable change in `capacity()` signals a reallocation just occurred?
    // A:
    // R:

    EXPECT_EQ(v.size(), cap_before_growth + 1);
    EXPECT_GT(cap_after_growth, cap_before_growth);
}

// ============================================================================
// Scenario 2: reserve Avoids Reallocation (Easy)
// ============================================================================

TEST_F(ContainersTest, ReserveAvoidsReallocationMoves)
{
    std::vector<Tracked> reserved;
    reserved.reserve(4);

    // Q: After `reserve(4)`, how many Tracked objects exist? What do `size()` and
    //    `capacity()` report?
    // A:
    // R:

    EXPECT_EQ(reserved.size(), 0u);
    EXPECT_GE(reserved.capacity(), 4u);
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 0u);

    EventLog::instance().clear();
    reserved.push_back(Tracked("A"));
    reserved.push_back(Tracked("B"));
    reserved.push_back(Tracked("C"));
    const auto* data_before = reserved.data();
    const auto cap = reserved.capacity();

    EventLog::instance().clear();
    reserved.push_back(Tracked("D"));

    // Q: Why did filling four elements after `reserve(4)` leave `capacity()` and
    //    `data()` unchanged?
    // A:
    // R:

    EXPECT_EQ(reserved.size(), 4u);
    EXPECT_EQ(reserved.capacity(), cap);
    EXPECT_EQ(reserved.data(), data_before);
    EXPECT_EQ(EventLog::instance().count_events("::move_ctor"), 1u);
}

// ============================================================================
// Scenario 3: map Is Ordered (Moderate)
// ============================================================================

TEST_F(ContainersTest, MapIterationIsKeyOrdered)
{
    std::map<int, std::string> ordered{{3, "c"}, {1, "a"}, {2, "b"}};

    std::vector<int> keys;
    for (const auto& [k, v] : ordered)
    {
        keys.push_back(k);
        (void)v;
    }

    // Q: Why does iteration yield keys `1, 2, 3` even though insertion was `3, 1, 2`?
    // A:
    // R:

    EXPECT_EQ(keys, (std::vector<int>{1, 2, 3}));
    EXPECT_TRUE(ordered.contains(2));
}

// ============================================================================
// Scenario 4: unordered_map Is Hashed (Moderate)
// ============================================================================

TEST_F(ContainersTest, UnorderedMapLookupIsHashBased)
{
    std::unordered_map<std::string, int> hashed{{"beta", 2}, {"alpha", 1}, {"gamma", 3}};

    // Q: What mechanism does `unordered_map` use for lookup, and what guarantee does
    //    it *not* make about iteration order?
    // A:
    // R:

    EXPECT_EQ(hashed.at("alpha"), 1);
    EXPECT_EQ(hashed.size(), 3u);
    EXPECT_TRUE(hashed.contains("gamma"));

    const auto bucket = hashed.bucket("alpha");
    EXPECT_LT(bucket, hashed.bucket_count());
}
