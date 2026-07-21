// Test Suite: Pitfalls and Contrast
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>

class PitfallsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: unique_ptr vs shared_ptr Ownership (Easy)
// ============================================================================

TEST_F(PitfallsTest, UniqueIsExclusiveSharedIsShared)
{
    auto unique = std::make_unique<Tracked>("Unique");
    auto shared = std::make_shared<Tracked>("Shared");
    auto shared_copy = shared;

    // Q: What compile-time constraint makes `unique` non-copyable, and what
    //    runtime signal shows `shared` accepted a second owner?
    // A:
    // R:

    EXPECT_EQ(shared.use_count(), 2);
    EXPECT_NE(unique.get(), nullptr);
}

// ============================================================================
// Scenario 2: Pass-By-Value Bumps use_count (Easy)
// ============================================================================

TEST_F(PitfallsTest, PassByValueBumpsUseCountUnnecessarily)
{
    auto resource = std::make_shared<Tracked>("Arg");

    long seen_by_value = 0;
    auto take_by_value = [&](std::shared_ptr<Tracked> item) { seen_by_value = item.use_count(); };

    long seen_by_cref = 0;
    auto take_by_cref = [&](const std::shared_ptr<Tracked>& item) { seen_by_cref = item.use_count(); };

    take_by_value(resource);
    take_by_cref(resource);

    // Q: Why does by-value see 2 while const-ref sees 1?
    // A:
    // R:

    EXPECT_EQ(seen_by_value, 2);
    EXPECT_EQ(seen_by_cref, 1);
    EXPECT_EQ(resource.use_count(), 1);
}

// ============================================================================
// Scenario 3: Two Control Blocks from the Same Raw Pointer (Hard)
// ============================================================================

TEST_F(PitfallsTest, TwoSharedPtrsFromSameRawMakeIndependentOwners)
{
    Tracked* raw = new Tracked("DoubleOwned");

    // Second owner uses a no-op deleter so this test process does not double-free.
    // With a real `delete` deleter on both, destruction is undefined behavior.
    std::shared_ptr<Tracked> a(raw);
    std::shared_ptr<Tracked> b(raw, [](Tracked*) {});

    // Q: Both point at the same address. Why does each report `use_count() == 1`?
    // A:
    // R:

    EXPECT_EQ(a.get(), b.get());
    EXPECT_EQ(a.use_count(), 1);
    EXPECT_EQ(b.use_count(), 1);

    // Q: If both used a deleting deleter, what would happen when each `shared_ptr`
    //    destroyed its "sole" ownership of `raw`?
    // A:
    // R:

    a.reset();
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
    b.reset();
}

// ============================================================================
// Scenario 4: Ignoring a Failed weak_ptr::lock (Moderate)
// ============================================================================

TEST_F(PitfallsTest, IgnoringFailedWeakLockIsUnsafe)
{
    std::weak_ptr<Tracked> weak;

    {
        auto shared = std::make_shared<Tracked>("Ephemeral");
        weak = shared;
    }

    auto locked = weak.lock();

    // Q: After the owner dies, what must you check before using `locked`?
    // A:
    // R:

    EXPECT_EQ(locked.get(), nullptr);
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}
