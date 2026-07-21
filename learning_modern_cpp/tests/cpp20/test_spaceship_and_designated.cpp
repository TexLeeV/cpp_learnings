// Test Suite: Spaceship Operator and Designated Initializers
// Estimated Time: 1-2 hours
// Difficulty: Easy / Moderate
// Introduced in: C++20
// C++ Standard: C++20 (project build)

#include "instrumentation.h"

#include <compare>
#include <gtest/gtest.h>
#include <set>
#include <string>

class SpaceshipDesignatedTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

struct Point
{
    int x = 0;
    int y = 0;

    auto operator<=>(const Point&) const = default;
};

struct Named
{
    std::string label;
    int id = 0;
};

// ============================================================================
// Scenario 1: Designated Initializers (Easy)
// ============================================================================

TEST_F(SpaceshipDesignatedTest, DesignatedInitializersNameMembers)
{
    Named n{.label = "alpha", .id = 7};

    // Q: What does `.label = "alpha"` select, and why must designated members
    //    appear in declaration order in C++?
    // A:
    // R:

    EXPECT_EQ(n.label, "alpha");
    EXPECT_EQ(n.id, 7);

    // Contrast with C++11 aggregate init by position:
    Named positional{"beta", 8};
    EXPECT_EQ(positional.label, "beta");
    EXPECT_EQ(positional.id, 8);
}

// ============================================================================
// Scenario 2: Defaulted Spaceship Generates Ordering (Easy)
// ============================================================================

TEST_F(SpaceshipDesignatedTest, DefaultedSpaceshipProvidesOrdering)
{
    Point a{.x = 1, .y = 2};
    Point b{.x = 1, .y = 3};
    Point c{.x = 1, .y = 2};

    // Q: With `operator<=>` defaulted, which comparisons become available, and
    //    what member order decides `a < b` here?
    // A:
    // R:

    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a == c);
    EXPECT_TRUE(b > a);
}

// ============================================================================
// Scenario 3: Spaceship Enables Associative Containers (Moderate)
// ============================================================================

TEST_F(SpaceshipDesignatedTest, SpaceshipWorksWithSet)
{
    std::set<Point> points;
    points.insert(Point{.x = 2, .y = 1});
    points.insert(Point{.x = 1, .y = 9});
    points.insert(Point{.x = 2, .y = 1}); // duplicate

    // Q: Why can `std::set<Point>` work with only a defaulted `<=>`, and why is
    //    the duplicate `{2,1}` not stored twice?
    // A:
    // R:

    ASSERT_EQ(points.size(), 2u);
    auto it = points.begin();
    EXPECT_EQ(it->x, 1);
    EXPECT_EQ(it->y, 9);
}
