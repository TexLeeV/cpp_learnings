// Test Suite: Algorithms (find, sort, transform, accumulate)
// Estimated Time: 1-2 hours
// Difficulty: Easy
// C++ Standard: C++20

#include "instrumentation.h"

#include <algorithm>
#include <gtest/gtest.h>
#include <numeric>
#include <vector>

class AlgorithmsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: find (Easy)
// ============================================================================

TEST_F(AlgorithmsTest, FindReturnsIteratorOrEnd)
{
    std::vector<int> v{10, 20, 30, 40};

    auto hit = std::find(v.begin(), v.end(), 30);
    auto miss = std::find(v.begin(), v.end(), 99);

    // Q: Why does a miss return `end()` rather than a null or a boolean?
    // A:
    // R:

    EXPECT_NE(hit, v.end());
    EXPECT_EQ(*hit, 30);
    EXPECT_EQ(miss, v.end());

    // Q: What is the worst-case number of element comparisons `find` may perform
    //    on a range of size `n`?
    // A:
    // R:
}

// ============================================================================
// Scenario 2: sort (Easy)
// ============================================================================

TEST_F(AlgorithmsTest, SortOrdersRangeInPlace)
{
    std::vector<Tracked> v;
    v.push_back(Tracked("C"));
    v.push_back(Tracked("A"));
    v.push_back(Tracked("B"));

    EventLog::instance().clear();
    std::sort(v.begin(), v.end(), [](const Tracked& a, const Tracked& b) {
        EventLog::instance().record("compare");
        return a.name() < b.name();
    });

    // Q: Which EventLog signal shows comparisons ran, and what order should `name()`
    //    values have after sort?
    // A:
    // R:

    EXPECT_GT(EventLog::instance().count_events("compare"), 0u);
    EXPECT_EQ(v[0].name(), "A");
    EXPECT_EQ(v[1].name(), "B");
    EXPECT_EQ(v[2].name(), "C");
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end(),
                               [](const Tracked& a, const Tracked& b) { return a.name() < b.name(); }));
}

// ============================================================================
// Scenario 3: transform (Easy)
// ============================================================================

TEST_F(AlgorithmsTest, TransformMapsElements)
{
    std::vector<int> input{1, 2, 3, 4};
    std::vector<int> output;

    std::transform(input.begin(), input.end(), std::back_inserter(output),
                   [](int x) { return x * 2; });

    // Q: Does `transform` change `input` here, and what role does `back_inserter`
    //    play for `output`?
    // A:
    // R:

    EXPECT_EQ(input, (std::vector<int>{1, 2, 3, 4}));
    EXPECT_EQ(output, (std::vector<int>{2, 4, 6, 8}));

    std::transform(input.begin(), input.end(), input.begin(), [](int x) { return x + 1; });
    EXPECT_EQ(input, (std::vector<int>{2, 3, 4, 5}));
}

// ============================================================================
// Scenario 4: accumulate (Moderate)
// ============================================================================

TEST_F(AlgorithmsTest, AccumulateReducesToSingleValue)
{
    std::vector<int> v{1, 2, 3, 4, 5};

    const int sum = std::accumulate(v.begin(), v.end(), 0);
    const int product = std::accumulate(v.begin(), v.end(), 1, [](int acc, int x) { return acc * x; });

    // Q: Why does the product call pass `1` as the initial value instead of `0`?
    // A:
    // R:

    // Q: What does `accumulate` fold left-to-right, and what is the observable
    //    result type of that fold?
    // A:
    // R:

    EXPECT_EQ(sum, 15);
    EXPECT_EQ(product, 120);
}
