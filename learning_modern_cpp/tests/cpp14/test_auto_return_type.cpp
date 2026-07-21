// Test Suite: auto Return Type Deduction
// Estimated Time: 1 hour
// Difficulty: Easy / Moderate
// Introduced in: C++14
// C++ Standard: C++20 (project build)

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>

class AutoReturnTypeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Deduce Return Type from return Statements (Easy)
// ============================================================================

auto make_tracked(const std::string& name)
{
    return std::make_shared<Tracked>(name);
}

TEST_F(AutoReturnTypeTest, AutoReturnDeduceFromReturn)
{
    auto ptr = make_tracked("AutoReturn");

    // Q: What is the deduced return type of `make_tracked`, and what observable
    //    compile-time check confirms it?
    // A:
    // R:

    static_assert(std::is_same_v<decltype(ptr), std::shared_ptr<Tracked>>);
    EXPECT_EQ(ptr->name(), "AutoReturn");
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);
}

// ============================================================================
// Scenario 2: All Returns Must Agree (Moderate)
// ============================================================================

auto pick_int(bool flag)
{
    if (flag)
    {
        return 1;
    }
    return 2;
}

TEST_F(AutoReturnTypeTest, AllReturnStatementsMustAgree)
{
    // Q: What goes wrong if one `return` produced `int` and another produced
    //    `double` in the same `auto`-return function?
    // A:
    // R:

    EXPECT_EQ(pick_int(true), 1);
    EXPECT_EQ(pick_int(false), 2);
    static_assert(std::is_same_v<decltype(pick_int(true)), int>);
}
