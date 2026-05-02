// Test Suite: Virtual Destructors and Destruction Order
// Estimated Time: 2 hours
// Difficulty: Easy / Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>

class VirtualDestructorsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Hierarchy A: NON-virtual destructor in base (BUG SCAFFOLD)
// ============================================================================

class BadBase
{
public:
    BadBase()
    {
        EventLog::instance().record("BadBase::ctor");
    }

    // Intentionally NOT virtual.
    ~BadBase()
    {
        EventLog::instance().record("BadBase::dtor");
    }
};

class BadDerived : public BadBase
{
public:
    BadDerived()
    {
        EventLog::instance().record("BadDerived::ctor");
    }

    ~BadDerived()
    {
        EventLog::instance().record("BadDerived::dtor");
    }
};

// ============================================================================
// Hierarchy B: virtual destructor in base (correct shape)
// ============================================================================

class GoodBase
{
public:
    GoodBase()
    {
        EventLog::instance().record("GoodBase::ctor");
    }

    virtual ~GoodBase()
    {
        EventLog::instance().record("GoodBase::dtor");
    }
};

class GoodDerived : public GoodBase
{
public:
    GoodDerived()
    {
        EventLog::instance().record("GoodDerived::ctor");
    }

    ~GoodDerived() override
    {
        EventLog::instance().record("GoodDerived::dtor");
    }
};

class GoodGrandchild : public GoodDerived
{
public:
    GoodGrandchild()
    {
        EventLog::instance().record("GoodGrandchild::ctor");
    }

    ~GoodGrandchild() override
    {
        EventLog::instance().record("GoodGrandchild::dtor");
    }
};

// ============================================================================
// Scenario 1: Construction Order (Easy)
// ============================================================================

TEST_F(VirtualDestructorsTest, ConstructionOrderBaseFirst)
{
    {
        GoodGrandchild g;
        // Q: In what order do the three constructors run, and which subobject
        //    must be fully constructed before the next can begin?
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
}

// ============================================================================
// Scenario 2: Destruction Through a Base Pointer With Virtual Destructor (Easy)
// ============================================================================

TEST_F(VirtualDestructorsTest, VirtualDestructorRunsFullChain)
{
    GoodBase* p = new GoodDerived();
    delete p;

    // Q: How many destructor entries should appear in the EventLog, and in
    //    what order? Why does that count match exactly the number of
    //    subobjects in a GoodDerived?
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
// Scenario 3: Destruction Through a Base Pointer Without `virtual` (Moderate)
// ============================================================================

TEST_F(VirtualDestructorsTest, NonVirtualDestructorSkipsDerivedSubobject)
{
    BadBase* p = new BadDerived();

    EventLog::instance().clear();

    // `delete p` calls BadBase::~BadBase. Because the base destructor is not
    // virtual, the call is dispatched statically. The standard says this is
    // undefined behavior when the dynamic type differs from the static type
    // unless the destructor is trivial. Most implementations exhibit the
    // observable failure tested below: BadDerived::dtor never runs.
    delete p;

    // Q: What did the EventLog NOT record that it would have recorded if
    //    BadBase's destructor were virtual? Name the specific entry.
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("BadDerived::dtor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("BadBase::dtor"), 1);

    // Q: If BadDerived held a `std::unique_ptr<int>` member, what would the
    //    real-world consequence of the missing destructor call be?
    // A:
    // R:
}

// ============================================================================
// Scenario 4: smart_ptr Polymorphic Cleanup (Moderate)
// ============================================================================

TEST_F(VirtualDestructorsTest, UniquePtrUsesVirtualDestructorChain)
{
    {
        std::unique_ptr<GoodBase> owner = std::make_unique<GoodGrandchild>();
        EventLog::instance().clear();
    }

    // Q: Why does `unique_ptr<GoodBase>` correctly destroy a GoodGrandchild,
    //    while `unique_ptr<BadBase>` of a BadDerived would not? What single
    //    declaration on the base type makes the difference?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("GoodGrandchild::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("GoodDerived::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("GoodBase::dtor"), 1);
}

// ============================================================================
// Scenario 5: shared_ptr Erases the Deleter Type (Hard)
// ============================================================================

TEST_F(VirtualDestructorsTest, SharedPtrCapturesDeleterAtConstruction)
{
    // shared_ptr stores a type-erased deleter chosen at the construction
    // site, so a shared_ptr<BadBase> built from `new BadDerived` still tears
    // down through BadDerived's destructor. This is a real exception to the
    // "always make destructors virtual" rule: shared_ptr can compensate for
    // a missing virtual destructor at the cost of a heap-stored deleter.
    {
        std::shared_ptr<BadBase> sp = std::shared_ptr<BadBase>(new BadDerived());
        EventLog::instance().clear();
    }

    // Q: What entry appears in EventLog here that DID NOT appear in the
    //    raw-pointer scenario above? What does that prove about where the
    //    "correct destructor" decision was actually made?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("BadDerived::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("BadBase::dtor"), 1);

    // Q: This works because shared_ptr remembers the static type at
    //    construction. If you reconstructed `sp` via
    //    `std::shared_ptr<BadBase>(static_cast<BadBase*>(new BadDerived()))`,
    //    would BadDerived::dtor still run? Why or why not?
    // A:
    // R:
}

// ============================================================================
// Scenario 6: Pure Virtual Destructor Must Still Have a Definition (Hard)
// ============================================================================

class AbstractCleanup
{
public:
    AbstractCleanup()
    {
        EventLog::instance().record("AbstractCleanup::ctor");
    }

    // Pure virtual makes the class abstract, but the destructor body is
    // still required because every derived destructor implicitly chains up
    // to the base destructor.
    virtual ~AbstractCleanup() = 0;
};

AbstractCleanup::~AbstractCleanup()
{
    EventLog::instance().record("AbstractCleanup::dtor");
}

class ConcreteCleanup : public AbstractCleanup
{
public:
    ConcreteCleanup()
    {
        EventLog::instance().record("ConcreteCleanup::ctor");
    }

    ~ConcreteCleanup() override
    {
        EventLog::instance().record("ConcreteCleanup::dtor");
    }
};

TEST_F(VirtualDestructorsTest, PureVirtualDestructorStillRunsBaseBody)
{
    {
        std::unique_ptr<AbstractCleanup> p = std::make_unique<ConcreteCleanup>();
        EventLog::instance().clear();
    }

    // Q: A pure virtual function with `= 0` usually has no body. Why is the
    //    destructor a special case that requires a definition anyway?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("ConcreteCleanup::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("AbstractCleanup::dtor"), 1);

    // TODO (learner): Try removing the `AbstractCleanup::~AbstractCleanup`
    // body above. The link should fail. Capture the linker error in a
    // comment here, and explain which destructor's implicit chain-up
    // referenced the missing symbol.
}
