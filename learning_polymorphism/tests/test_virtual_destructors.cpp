// Test Suite: Virtual Destructors
// Estimated Time: 1-2 hours
// Difficulty: Easy / Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>

class VirtualDestructorsTest : public ::testing::Test
{
protected:
    void SetUp() override { EventLog::instance().clear(); }
};

// Hierarchy A: NON-virtual destructor (demonstrates missing derived cleanup).
class BadBase
{
public:
    BadBase() { EventLog::instance().record("BadBase::ctor"); }
    ~BadBase() { EventLog::instance().record("BadBase::dtor"); }
};

class BadDerived : public BadBase
{
public:
    BadDerived() { EventLog::instance().record("BadDerived::ctor"); }
    ~BadDerived() { EventLog::instance().record("BadDerived::dtor"); }
};

// Hierarchy B: virtual destructor (correct polymorphic cleanup).
class GoodBase
{
public:
    GoodBase() { EventLog::instance().record("GoodBase::ctor"); }
    virtual ~GoodBase() { EventLog::instance().record("GoodBase::dtor"); }
};

class GoodDerived : public GoodBase
{
public:
    GoodDerived() { EventLog::instance().record("GoodDerived::ctor"); }
    ~GoodDerived() override { EventLog::instance().record("GoodDerived::dtor"); }
};

class GoodGrandchild : public GoodDerived
{
public:
    GoodGrandchild() { EventLog::instance().record("GoodGrandchild::ctor"); }
    ~GoodGrandchild() override { EventLog::instance().record("GoodGrandchild::dtor"); }
};

// ============================================================================
// Scenario 1: Construction Order (Easy)
// ============================================================================

TEST_F(VirtualDestructorsTest, ConstructionOrderBaseFirst)
{
    {
        GoodGrandchild g;
        // Q: In what order do the three constructors run, and why must each
        //    base subobject finish before the next derived ctor begins?
        // A:
        // R:
    }

    auto events = EventLog::instance().events();
    ASSERT_EQ(events.size(), 6u);
    EXPECT_EQ(events[0], "GoodBase::ctor");
    EXPECT_EQ(events[1], "GoodDerived::ctor");
    EXPECT_EQ(events[2], "GoodGrandchild::ctor");
    EXPECT_EQ(events[3], "GoodGrandchild::dtor");
    EXPECT_EQ(events[4], "GoodDerived::dtor");
    EXPECT_EQ(events[5], "GoodBase::dtor");

    // Q: How does destruction order relate to construction order here?
    // A:
    // R:
}

// ============================================================================
// Scenario 2: Virtual Destructor Runs Full Chain (Easy)
// ============================================================================

TEST_F(VirtualDestructorsTest, VirtualDestructorRunsFullChain)
{
    GoodBase* p = new GoodDerived();
    delete p;

    // Q: How many destructor entries appear, and in what order? Why does that
    //    match the number of subobjects in a GoodDerived?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("GoodDerived::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("GoodBase::dtor"), 1);

    auto events = EventLog::instance().events();
    ASSERT_GE(events.size(), 2u);
    EXPECT_EQ(events[events.size() - 2], "GoodDerived::dtor");
    EXPECT_EQ(events[events.size() - 1], "GoodBase::dtor");
}

// ============================================================================
// Scenario 3: Non-Virtual Destructor Skips Derived (Moderate)
// ============================================================================

TEST_F(VirtualDestructorsTest, NonVirtualDestructorSkipsDerivedSubobject)
{
    BadBase* p = new BadDerived();
    EventLog::instance().clear();

    // delete through a non-virtual base dtor is undefined behavior when the
    // dynamic type differs. Common implementations skip BadDerived::dtor;
    // we only assert that observable EventLog gap (no heap member access).
    delete p;

    // Q: Which EventLog entry is missing that would appear if BadBase's
    //    destructor were virtual?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("BadDerived::dtor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("BadBase::dtor"), 1);

    // Q: If BadDerived held a unique_ptr member, what would the missing
    //    destructor call leak in practice?
    // A:
    // R:
}

// ============================================================================
// Scenario 4: unique_ptr Polymorphic Cleanup (Moderate)
// ============================================================================

TEST_F(VirtualDestructorsTest, UniquePtrUsesVirtualDestructorChain)
{
    {
        std::unique_ptr<GoodBase> owner = std::make_unique<GoodGrandchild>();
        EventLog::instance().clear();
    }

    // Q: Why does unique_ptr<GoodBase> correctly destroy a GoodGrandchild?
    //    What single declaration on the base type makes the difference?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("GoodGrandchild::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("GoodDerived::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("GoodBase::dtor"), 1);

    // Q: Would unique_ptr<BadBase> owning a BadDerived show the same full
    //    dtor chain in EventLog? Why or why not?
    // A:
    // R:
}
