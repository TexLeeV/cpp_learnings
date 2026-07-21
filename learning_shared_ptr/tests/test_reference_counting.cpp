// Test Suite: Reference Counting
// Estimated Time: 1-2 hours
// Difficulty: Easy
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>

class ReferenceCountingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Copy Shares Ownership (Easy)
// ============================================================================

TEST_F(ReferenceCountingTest, CopyIncrementsUntilLastOwnerDestroys)
{
    long after_creation = 0;
    long after_copy = 0;

    {
        std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("A");
        after_creation = p1.use_count();

        // Q: What does `use_count()` count here?
        // A:
        // R:

        {
            std::shared_ptr<Tracked> p2 = p1;
            after_copy = p1.use_count();

            // Q: Why do `p1.use_count()` and `p2.use_count()` agree?
            // A:
            // R:

            EXPECT_EQ(after_copy, 2);
        }

        EXPECT_EQ(p1.use_count(), 1);
        EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);
    }

    // Q: Which EventLog signal confirms destruction only after the last owner left?
    // A:
    // R:

    EXPECT_EQ(after_creation, 1);
    EXPECT_EQ(after_copy, 2);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}

// ============================================================================
// Scenario 2: Move Transfers Without Increment (Easy)
// ============================================================================

TEST_F(ReferenceCountingTest, MoveTransfersOwnershipWithoutIncrement)
{
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("B");
    std::shared_ptr<Tracked> p2 = std::move(p1);

    // Q: After the move, what are `p1.use_count()` and `p2.use_count()`, and why
    //    is neither 2?
    // A:
    // R:

    EXPECT_EQ(p1.use_count(), 0);
    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(p2.use_count(), 1);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);
}

// ============================================================================
// Scenario 3: reset() Releases or Retargets (Moderate)
// ============================================================================

TEST_F(ReferenceCountingTest, ResetReleasesThenRetargetsIndependently)
{
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Tracked1");
    std::shared_ptr<Tracked> p2 = p1;

    p1.reset();

    // Q: After `p1.reset()`, why is Tracked("Tracked1") still alive?
    // A:
    // R:

    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(p2.use_count(), 1);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);

    p1.reset(new Tracked("E"));

    // Q: Do `p1` and `p2` share a control block after retargeting `p1`?
    // A:
    // R:

    EXPECT_EQ(p1.use_count(), 1);
    EXPECT_NE(p1.get(), p2.get());

    p2.reset();
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}

// ============================================================================
// Scenario 4: Move Across Scopes (Moderate)
// ============================================================================

TEST_F(ReferenceCountingTest, MoveOutlivesInnerScope)
{
    std::shared_ptr<Tracked> outer;

    {
        std::shared_ptr<Tracked> inner = std::make_shared<Tracked>("inner");
        outer = std::move(inner);

        // Q: Why can `inner`'s destructor run without destroying Tracked("inner")?
        // A:
        // R:
    }

    // Q: What EventLog count proves the object outlived the inner scope?
    // A:
    // R:

    EXPECT_EQ(outer.use_count(), 1);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);

    outer.reset();
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}
