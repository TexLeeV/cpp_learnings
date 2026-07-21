// Test Suite: Move Assignment
// Estimated Time: 1-2 hours
// Difficulty: Easy / Moderate
// C++ Standard: C++20

#include "move_instrumentation.h"

#include <gtest/gtest.h>
#include <utility>

class MoveAssignmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Self-Move Assignment (Moderate)
// ============================================================================

TEST_F(MoveAssignmentTest, SelfMoveAssignmentIsObservable)
{
    MoveTracked obj("SelfMove");

    EventLog::instance().clear();

    obj = std::move(obj);

    // Q: Which EventLog signal fires for self-move-assignment, and what does
    //    `is_moved_from()` report afterward on this MoveTracked?
    // A:
    // R:

    // Q: If move-assign deleted its resource before stealing from `other`, what
    //    would go wrong when `this == &other`?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("move_assign"), 1);
    EXPECT_TRUE(obj.is_moved_from());
}

// ============================================================================
// Scenario 2: Move From Temporary (Easy)
// ============================================================================

TEST_F(MoveAssignmentTest, MoveAssignFromTemporary)
{
    MoveTracked obj("Target");

    EventLog::instance().clear();

    obj = MoveTracked("Temporary");

    // Q: No `std::move` appears in the assignment. Which EventLog signals show
    //    construction of the temporary and move-assignment into `obj`?
    // A:
    // R:

    // Q: What value category is `MoveTracked("Temporary")`, and why does that
    //    select move assignment rather than copy assignment?
    // A:
    // R:

    EXPECT_EQ(obj.name(), "Temporary");
    EXPECT_GE(EventLog::instance().count_events("::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_assign"), 1);
}

// ============================================================================
// Scenario 3: Moved-From Remains Destructible (Easy)
// ============================================================================

TEST_F(MoveAssignmentTest, MovedFromObjectRemainsDestructible)
{
    {
        MoveTracked src("Alive");
        MoveTracked dst(std::move(src));

        // Q: After the move, which signals show `src` is moved-from while `dst`
        //    still owns the name "Alive"?
        // A:
        // R:

        EXPECT_TRUE(src.is_moved_from());
        EXPECT_EQ(dst.name(), "Alive");
        EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
    }

    // Q: Both objects left scope. Which EventLog dtor entries prove a moved-from
    //    object is still destroyed safely?
    // A:
    // R:

    EXPECT_GE(EventLog::instance().count_events("::dtor"), 2);
}

// ============================================================================
// Scenario 4: Chained Move Assignment (Moderate)
// ============================================================================

TEST_F(MoveAssignmentTest, ChainedMoveAssignmentPropagatesMovedFrom)
{
    MoveTracked a("First");
    MoveTracked b("Second");
    MoveTracked c("Third");

    EventLog::instance().clear();

    b = std::move(a);
    c = std::move(b);

    // Q: After both assignments, which objects report `is_moved_from()`, and
    //    what name does `c` hold?
    // A:
    // R:

    // Q: How many `move_assign` events fire, and what does a second move from
    //    already-moved-from `a` remain allowed to do under the type's contract?
    // A:
    // R:

    EXPECT_TRUE(a.is_moved_from());
    EXPECT_TRUE(b.is_moved_from());
    EXPECT_EQ(c.name(), "First");
    EXPECT_EQ(EventLog::instance().count_events("move_assign"), 2);
}
