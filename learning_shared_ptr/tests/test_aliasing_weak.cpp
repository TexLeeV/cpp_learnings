// Test Suite: Aliasing and weak_ptr
// Estimated Time: 1-2 hours
// Difficulty: Easy / Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>

class AliasingWeakTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

struct Container
{
    int tag = 0; // first member so `&member` differs from `owner.get()`
    Tracked member;
    explicit Container(const std::string& name) : member(name)
    {
    }
};

// ============================================================================
// Scenario 1: Aliasing Shares the Control Block (Easy)
// ============================================================================

TEST_F(AliasingWeakTest, AliasingSharesControlBlockNotStoredPointer)
{
    auto owner = std::make_shared<Container>("Container1");
    std::shared_ptr<Tracked> alias(owner, &owner->member);

    // Q: Which constructor argument chooses the control block, and which chooses
    //    what `alias.get()` returns?
    // A:
    // R:

    EXPECT_EQ(owner.use_count(), 2);
    EXPECT_EQ(alias.use_count(), 2);
    EXPECT_EQ(alias.get(), &owner->member);
    EXPECT_NE(static_cast<void*>(alias.get()), static_cast<void*>(owner.get()));
}

// ============================================================================
// Scenario 2: Alias Keeps the Owner Alive (Moderate)
// ============================================================================

TEST_F(AliasingWeakTest, AliasKeepsOwnerAllocationAlive)
{
    std::shared_ptr<Tracked> alias;

    {
        auto owner = std::make_shared<Container>("Container2");
        alias = std::shared_ptr<Tracked>(owner, &owner->member);
        owner.reset();

        // Q: After `owner.reset()`, why is there still no `::dtor`?
        // A:
        // R:

        EXPECT_EQ(alias.use_count(), 1);
        EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);
    }

    alias.reset();

    // Q: Which reset finally destroys the Container, and what EventLog signal confirms it?
    // A:
    // R:

    EXPECT_GE(EventLog::instance().count_events("::dtor"), 1u);
}

// ============================================================================
// Scenario 3: weak_ptr Does Not Extend Lifetime (Easy)
// ============================================================================

TEST_F(AliasingWeakTest, WeakPtrDoesNotExtendLifetime)
{
    std::weak_ptr<Tracked> weak;

    {
        auto shared = std::make_shared<Tracked>("TrackedObj");
        weak = shared;

        // Q: After `weak = shared`, why is `shared.use_count()` still 1?
        // A:
        // R:

        EXPECT_EQ(shared.use_count(), 1);
        EXPECT_FALSE(weak.expired());
    }

    // Q: After the owner leaves scope, what do `expired()` and EventLog jointly prove?
    // A:
    // R:

    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(weak.lock().get(), nullptr);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}

// ============================================================================
// Scenario 4: lock() Temporarily Promotes (Moderate)
// ============================================================================

TEST_F(AliasingWeakTest, LockPromotesWhileObjectLives)
{
    auto shared = std::make_shared<Tracked>("TrackedObj");
    std::weak_ptr<Tracked> weak = shared;

    auto locked = weak.lock();

    // Q: Why does `use_count` become 2 after a successful `lock()`?
    // A:
    // R:

    EXPECT_EQ(shared.use_count(), 2);
    EXPECT_EQ(locked.get(), shared.get());

    shared.reset();

    // Q: After resetting `shared`, what keeps the object alive?
    // A:
    // R:

    EXPECT_EQ(locked.use_count(), 1);
    EXPECT_FALSE(weak.expired());
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);
}
