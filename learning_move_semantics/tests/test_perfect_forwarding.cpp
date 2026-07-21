// Test Suite: Perfect Forwarding
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "move_instrumentation.h"

#include <gtest/gtest.h>
#include <utility>
#include <vector>

class PerfectForwardingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Universal Ref vs Rvalue Ref (Easy)
// ============================================================================

TEST_F(PerfectForwardingTest, UniversalRefAcceptsLvalueRvalueRefDoesNot)
{
    MoveTracked obj("Bound");

    auto take_rvalue = [](MoveTracked&& /*param*/) {
        EventLog::instance().record("rvalue_ref called");
    };

    auto take_fwd = [](auto&& /*param*/) {
        EventLog::instance().record("universal_ref called");
    };

    take_fwd(obj);
    take_rvalue(std::move(obj));

    // Q: Why does deduced `auto&&` / `T&&` accept the lvalue `obj` while
    //    `MoveTracked&&` rejects it without `std::move`?
    // A:
    // R:

    // Q: After `take_rvalue(std::move(obj))`, which EventLog entries confirm
    //    both call paths ran?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("universal_ref called"), 1);
    EXPECT_EQ(EventLog::instance().count_events("rvalue_ref called"), 1);
}

// ============================================================================
// Scenario 2: std::forward Preserves Value Category (Moderate)
// ============================================================================

TEST_F(PerfectForwardingTest, StdForwardPreservesValueCategoryIntoVector)
{
    auto push_forwarded = [](auto&& arg) {
        std::vector<MoveTracked> vec;
        vec.reserve(1);
        vec.push_back(std::forward<decltype(arg)>(arg));
    };

    MoveTracked named("Named");

    EventLog::instance().clear();
    push_forwarded(named);

    // Q: With an lvalue argument, which constructor should `std::forward` select
    //    into the vector, and which EventLog count confirms it?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 0);

    EventLog::instance().clear();
    push_forwarded(MoveTracked("Temp"));

    // Q: With an rvalue argument, which constructor fires instead, and why did
    //    `std::forward` not force a copy?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 0);
    EXPECT_GE(EventLog::instance().count_events("move_ctor"), 1);
}

// ============================================================================
// Scenario 3: Emplace-Style Factory (Moderate)
// ============================================================================

TEST_F(PerfectForwardingTest, MakeValueForwardsIntoConstruction)
{
    MoveTracked named("Factory");

    EventLog::instance().clear();
    MoveTracked from_lvalue = make_value(named);
    MoveTracked from_rvalue = make_value(MoveTracked("Temp"));

    // Q: `make_value` records lvalue vs rvalue. Which EventLog lines show that
    //    distinction for the two calls above?
    // A:
    // R:

    // Q: Matching those categories, which of `copy_ctor` / `move_ctor` should
    //    appear for each `make_value` result?
    // A:
    // R:

    EXPECT_GE(EventLog::instance().count_events("make_value() called with lvalue"), 1);
    EXPECT_GE(EventLog::instance().count_events("make_value() called with rvalue"), 1);
    EXPECT_GE(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_GE(EventLog::instance().count_events("move_ctor"), 1);
    EXPECT_EQ(from_lvalue.name(), "Factory");
    EXPECT_EQ(from_rvalue.name(), "Temp");
}

// ============================================================================
// Scenario 4: Imperfect Forwarding With std::move (Hard)
// ============================================================================

TEST_F(PerfectForwardingTest, StdMoveInsideForwarderStealsLvalues)
{
    auto imperfect = [](auto&& arg) {
        std::vector<MoveTracked> vec;
        vec.reserve(1);
        vec.push_back(std::move(arg));
    };

    MoveTracked obj("Stolen");

    EventLog::instance().clear();
    imperfect(obj);

    // Q: The caller passed an lvalue. Why does `std::move(arg)` still produce a
    //    move into the vector, and what does `obj.is_moved_from()` report?
    // A:
    // R:

    // Q: What would `std::forward<decltype(arg)>(arg)` have done differently for
    //    this same lvalue call?
    // A:
    // R:

    EXPECT_TRUE(obj.is_moved_from());
    EXPECT_GE(EventLog::instance().count_events("move_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 0);
}
