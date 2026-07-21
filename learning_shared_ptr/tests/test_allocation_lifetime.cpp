// Test Suite: Allocation and Lifetime
// Estimated Time: 1-2 hours
// Difficulty: Easy / Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>

class AllocationLifetimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Last Owner Destroys the Object (Easy)
// ============================================================================

TEST_F(AllocationLifetimeTest, LastOwnerDestroyRunsTrackedDtor)
{
    {
        auto sole = std::make_shared<Tracked>("Sole");
        EXPECT_EQ(sole.use_count(), 1);
        EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);
    }

    // Q: After the sole owner leaves scope, which EventLog signal confirms
    //    destruction happened exactly once?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}

// ============================================================================
// Scenario 2: make_shared vs new (Easy)
// ============================================================================

TEST_F(AllocationLifetimeTest, MakeSharedAndNewBothStartAtOneOwner)
{
    std::shared_ptr<Tracked> from_new(new Tracked("FromNew"));
    auto from_make = std::make_shared<Tracked>("FromMake");

    // Q: Both report `use_count() == 1`. What does that tell you about the
    //    relationship between ownership count and allocation strategy?
    // A:
    // R:

    // Q: Typical layout: `new` uses two heap pieces (object + control block);
    //    `make_shared` often uses one. Which signal above still cannot distinguish them?
    // A:
    // R:

    EXPECT_EQ(from_new.use_count(), 1);
    EXPECT_EQ(from_make.use_count(), 1);
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 2);
}

// ============================================================================
// Scenario 3: Custom Deleter Runs on Last Release (Moderate)
// ============================================================================

TEST_F(AllocationLifetimeTest, CustomDeleterInvokedOnLastRelease)
{
    {
        std::shared_ptr<Tracked> p(new Tracked("WithDeleter"),
                                   LoggingDeleter<Tracked>("CustomDeleter"));

        // Q: Why is the deleter event still zero while `p` is alive?
        // A:
        // R:

        EXPECT_EQ(EventLog::instance().count_events("CustomDeleter::operator()"), 0);
    }

    // Q: When the last owner dies, what EventLog entries prove the custom
    //    deleter path ran?
    // A:
    // R:

    EXPECT_GE(EventLog::instance().count_events("CustomDeleter::operator()"), 1u);
    EXPECT_GE(EventLog::instance().count_events("::dtor"), 1u);
}

// ============================================================================
// Scenario 4: Shared Ownership Delays Destruction (Moderate)
// ============================================================================

TEST_F(AllocationLifetimeTest, SharedOwnershipDelaysDeleterUntilLastReset)
{
    std::shared_ptr<Tracked> p1(new Tracked("Shared"),
                                LoggingDeleter<Tracked>("SharedDeleter"));
    auto p2 = p1;
    auto p3 = p1;

    EXPECT_EQ(p1.use_count(), 3);

    EventLog::instance().clear();
    p1.reset();

    // Q: Why did neither the deleter nor `::dtor` fire after `p1.reset()`?
    // A:
    // R:

    EXPECT_EQ(p2.use_count(), 2);
    EXPECT_EQ(EventLog::instance().count_events("SharedDeleter::operator()"), 0);

    p2.reset();
    p3.reset();

    // Q: After which reset did destruction finally run, and what rule about
    //    `use_count` explains the timing?
    // A:
    // R:

    EXPECT_GE(EventLog::instance().count_events("SharedDeleter::operator()"), 1u);
    EXPECT_GE(EventLog::instance().count_events("::dtor"), 1u);
}
