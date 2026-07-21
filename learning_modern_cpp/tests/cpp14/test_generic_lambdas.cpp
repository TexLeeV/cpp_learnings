// Test Suite: Generic Lambdas
// Estimated Time: 1 hour
// Difficulty: Easy / Moderate
// Introduced in: C++14
// C++ Standard: C++20 (project build)

#include "instrumentation.h"

#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

class GenericLambdasTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: auto Parameters (Easy)
// ============================================================================

TEST_F(GenericLambdasTest, AutoParametersDeducePerCall)
{
    auto generic_add = [](auto a, auto b) { return a + b; };

    // Q: What does `auto` in the parameter list enable that a C++11 lambda could not?
    // A:
    // R:

    EXPECT_EQ(generic_add(1, 2), 3);
    EXPECT_EQ(generic_add(1.5, 2.5), 4.0);

    // Q: How many distinct operator() instantiations do the two calls above create?
    // A:
    // R:
}

// ============================================================================
// Scenario 2: Generic Lambda with Algorithms (Moderate)
// ============================================================================

TEST_F(GenericLambdasTest, GenericLambdaWithTransform)
{
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::vector<int> doubled(vec.size());

    std::transform(vec.begin(), vec.end(), doubled.begin(), [](auto x) { return x * 2; });

    // Q: Why can this same lambda form be reused later with a vector of doubles
    //    without writing a second functor type?
    // A:
    // R:

    EXPECT_EQ(doubled[0], 2);
    EXPECT_EQ(doubled[4], 10);
}
