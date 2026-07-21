// Test Suite: std::span
// Estimated Time: 1-2 hours
// Difficulty: Easy / Moderate
// Introduced in: C++20
// C++ Standard: C++20 (project build)

#include "instrumentation.h"

#include <array>
#include <gtest/gtest.h>
#include <span>
#include <vector>

class SpanTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

int sum_span(std::span<const int> values)
{
    EventLog::instance().record("sum_span");
    int total = 0;
    for (int v : values)
    {
        total += v;
    }
    return total;
}

// ============================================================================
// Scenario 1: Non-Owning View Over Contiguous Storage (Easy)
// ============================================================================

TEST_F(SpanTest, SpanViewsVectorWithoutCopying)
{
    std::vector<int> data{1, 2, 3, 4};
    std::span<int> view = data;

    // Q: Does `view` own its elements? What happens to `view[0]` if you change
    //    `data[0]`?
    // A:
    // R:

    data[0] = 10;
    EXPECT_EQ(view[0], 10);
    EXPECT_EQ(view.size(), 4u);
}

// ============================================================================
// Scenario 2: One API Accepts Vector, Array, and C Array (Easy)
// ============================================================================

TEST_F(SpanTest, SpanUnifiesContiguousCallSites)
{
    std::vector<int> vec{1, 2, 3};
    std::array<int, 3> arr{4, 5, 6};
    int raw[] = {7, 8, 9};

    EXPECT_EQ(sum_span(vec), 6);
    EXPECT_EQ(sum_span(arr), 15);
    EXPECT_EQ(sum_span(raw), 24);

    // Q: Why can `sum_span` take a `vector`, `array`, and C array without
    //    overloads, and how is that similar to `string_view` for strings?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("sum_span"), 3);
}

// ============================================================================
// Scenario 3: Subspan Lifetime Still Bound to Owner (Moderate)
// ============================================================================

TEST_F(SpanTest, SubspanDoesNotExtendOwnerLifetime)
{
    std::vector<int> data{10, 20, 30, 40, 50};
    std::span<int> mid = std::span<int>(data).subspan(1, 3);

    // Q: What range of `data` does `mid` refer to, and what becomes invalid if
    //    `data` is destroyed or reallocated while `mid` is still used?
    // A:
    // R:

    EXPECT_EQ(mid.size(), 3u);
    EXPECT_EQ(mid[0], 20);
    EXPECT_EQ(mid[2], 40);

    data.push_back(60); // may reallocate
    // Do not touch `mid` after a potential reallocation — teaching point above.
    EXPECT_EQ(data.back(), 60);
}
