// Test Suite: Ranges and Views
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// Introduced in: C++20
// C++ Standard: C++20 (project build)

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <ranges>
#include <vector>

class RangesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Filter View Is Lazy (Easy)
// ============================================================================

TEST_F(RangesTest, FilterViewIsLazy)
{
    std::vector<int> data{1, 2, 3, 4, 5, 6};

    auto evens = data | std::views::filter([](int x) {
                     EventLog::instance().record("filter_pred");
                     return x % 2 == 0;
                 });

    // Q: After building `evens` but before iterating, how many times has the
    //    predicate run? What does that imply about when views do work?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("filter_pred"), 0);

    std::vector<int> collected;
    for (int v : evens)
    {
        collected.push_back(v);
    }

    EXPECT_EQ(collected, (std::vector<int>{2, 4, 6}));
    EXPECT_GT(EventLog::instance().count_events("filter_pred"), 0u);
}

// ============================================================================
// Scenario 2: Pipeline Compose Filter and Transform (Moderate)
// ============================================================================

TEST_F(RangesTest, PipelineFilterThenTransform)
{
    std::vector<int> data{1, 2, 3, 4, 5};

    auto pipeline = data | std::views::filter([](int x) { return x % 2 == 1; }) |
                    std::views::transform([](int x) {
                        EventLog::instance().record("transform");
                        return x * x;
                    });

    std::vector<int> squares;
    for (int v : pipeline)
    {
        squares.push_back(v);
    }

    // Q: For input `{1,2,3,4,5}`, which values reach `transform`, and why is
    //    that different from transforming first and filtering afterward?
    // A:
    // R:

    EXPECT_EQ(squares, (std::vector<int>{1, 9, 25}));
    EXPECT_EQ(EventLog::instance().count_events("transform"), 3);
}

// ============================================================================
// Scenario 3: Views Borrow; They Do Not Own (Moderate)
// ============================================================================

TEST_F(RangesTest, ViewDoesNotOwnUnderlyingRange)
{
    std::vector<int> data{10, 20, 30};
    auto doubled = data | std::views::transform([](int x) { return x * 2; });

    data[0] = 11;

    auto it = doubled.begin();

    // Q: Why can changing `data[0]` affect what you observe through `doubled`?
    // A:
    // R:

    EXPECT_EQ(*it, 22);
}
