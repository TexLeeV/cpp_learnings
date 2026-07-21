// Test Suite: Move Basics
// Estimated Time: 1-2 hours
// Difficulty: Easy
// C++ Standard: C++20

#include "move_instrumentation.h"

#include <gtest/gtest.h>
#include <utility>
#include <vector>

class MoveBasicsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Lvalue Copies, Rvalue Moves (Easy)
// ============================================================================

TEST_F(MoveBasicsTest, LvalueCopyVsRvalueMoveIntoVector)
{
    MoveTracked obj("Lvalue");
    std::vector<MoveTracked> vec;
    vec.reserve(2);

    EventLog::instance().clear();

    vec.push_back(obj);
    vec.push_back(MoveTracked("Rvalue"));

    // Q: Which EventLog signals distinguish `push_back(obj)` from
    //    `push_back(MoveTracked("Rvalue"))`?
    // A:
    // R:

    // Q: What property of each argument selects copy_ctor versus move_ctor?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_GE(EventLog::instance().count_events("move_ctor"), 1);
}

// ============================================================================
// Scenario 2: std::move Casts to xvalue (Easy)
// ============================================================================

TEST_F(MoveBasicsTest, StdMoveCastsToXvalueAndLeavesMovedFrom)
{
    MoveTracked obj1("Original");
    MoveTracked obj2(std::move(obj1));

    // Q: What does `std::move(obj1)` change about the expression's value
    //    category, and what does it leave unchanged about `obj1` itself?
    // A:
    // R:

    // Q: Which EventLog signal and which `obj1` API confirm a move, not a copy?
    // A:
    // R:

    EXPECT_TRUE(obj1.is_moved_from());
    EXPECT_FALSE(obj2.is_moved_from());
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 0);
}

// ============================================================================
// Scenario 3: Move Assignment Replaces Existing State (Easy)
// ============================================================================

TEST_F(MoveBasicsTest, MoveAssignmentTransfersIntoExistingObject)
{
    MoveTracked src("Source");
    MoveTracked dst("Destination");

    EventLog::instance().clear();

    dst = std::move(src);

    // Q: Which EventLog signal confirms move assignment rather than move
    //    construction, and what state is `src` left in?
    // A:
    // R:

    // Q: Move ctor initializes a new object; move assign replaces an existing
    //    one. What resource work must assignment do that construction need not?
    // A:
    // R:

    EXPECT_TRUE(src.is_moved_from());
    EXPECT_EQ(dst.name(), "Source");
    EXPECT_EQ(EventLog::instance().count_events("move_assign"), 1);
}

// ============================================================================
// Scenario 4: const + std::move Still Copies (Moderate)
// ============================================================================

TEST_F(MoveBasicsTest, ConstPlusStdMoveStillCopies)
{
    const MoveTracked obj("Const");
    std::vector<MoveTracked> vec;
    vec.reserve(1);

    EventLog::instance().clear();

    vec.push_back(std::move(obj));

    // Q: `std::move(obj)` was used, yet which constructor fired? Why could the
    //    move constructor not bind?
    // A:
    // R:

    // Q: What is the type of `std::move(obj)` when `obj` is `const MoveTracked`?
    // A:
    // R:

    EXPECT_GE(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 0);
    EXPECT_FALSE(obj.is_moved_from());
}
