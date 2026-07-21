// Test Suite: Concepts
// Estimated Time: 1-2 hours
// Difficulty: Easy / Moderate
// Introduced in: C++20
// C++ Standard: C++20 (project build)

#include "instrumentation.h"

#include <concepts>
#include <gtest/gtest.h>
#include <string>
#include <type_traits>

class ConceptsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template <Addable T> T add_twice(T a, T b)
{
    EventLog::instance().record("add_twice");
    return a + b;
}

// ============================================================================
// Scenario 1: Constrained Function Template (Easy)
// ============================================================================

TEST_F(ConceptsTest, ConceptConstrainsCallSite)
{
    EXPECT_EQ(add_twice(2, 3), 5);
    EXPECT_EQ(add_twice(std::string("a"), std::string("b")), "ab");

    // Q: What does `Addable` require of `T`, and what fails at the call site
    //    if you pass a type that has no `operator+`?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("add_twice"), 2);

    // TODO (learner): Uncomment and explain the diagnostic you expect.
    // struct NoAdd {};
    // add_twice(NoAdd{}, NoAdd{});
}

// ============================================================================
// Scenario 2: Standard Library Concepts (Easy)
// ============================================================================

TEST_F(ConceptsTest, StandardConceptsClassifyTypes)
{
    static_assert(std::integral<int>);
    static_assert(!std::integral<double>);
    static_assert(std::floating_point<double>);
    static_assert(std::same_as<decltype(1 + 1), int>);

    // Q: How does `std::integral<T>` differ from writing the constraint by hand
    //    with `std::is_integral_v<T>` in an `enable_if`?
    // A:
    // R:

    EXPECT_TRUE(std::integral<int>);
    EXPECT_FALSE(std::integral<std::string>);
}

// ============================================================================
// Scenario 3: requires Expression Inspects Syntax (Moderate)
// ============================================================================

template <typename T>
concept HasName = requires(const T& t) {
    { t.name() } -> std::convertible_to<std::string>;
};

template <HasName T> std::string read_name(const T& t)
{
    EventLog::instance().record("read_name");
    return t.name();
}

TEST_F(ConceptsTest, RequiresExpressionChecksMembers)
{
    Tracked t("ConceptTracked");
    EXPECT_EQ(read_name(t), "ConceptTracked");

    // Q: Why does `Tracked` satisfy `HasName` here, and what would falsify that
    //    without changing `read_name`'s body?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("read_name"), 1);
}
