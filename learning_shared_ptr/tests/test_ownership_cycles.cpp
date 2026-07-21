// Test Suite: Ownership and Cycles
// Estimated Time: 2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>

class OwnershipCyclesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

class Widget : public std::enable_shared_from_this<Widget>
{
public:
    explicit Widget(const std::string& name) : tracked_(name)
    {
    }

    std::shared_ptr<Widget> get_shared()
    {
        return shared_from_this();
    }

private:
    Tracked tracked_;
};

struct SharedNode
{
    Tracked tracked;
    std::shared_ptr<SharedNode> next;
    explicit SharedNode(const std::string& name) : tracked(name)
    {
    }
};

struct WeakNode
{
    Tracked tracked;
    std::weak_ptr<WeakNode> next;
    explicit WeakNode(const std::string& name) : tracked(name)
    {
    }
};

// ============================================================================
// Scenario 1: enable_shared_from_this Shares the Existing Control Block (Easy)
// ============================================================================

TEST_F(OwnershipCyclesTest, SharedFromThisSharesExistingControlBlock)
{
    auto w1 = std::make_shared<Widget>("W1");
    auto w2 = w1->get_shared();

    // Q: Why is `use_count` 2 after `get_shared()`, and do `w1`/`w2` point at
    //    the same object?
    // A:
    // R:

    EXPECT_EQ(w1.use_count(), 2);
    EXPECT_EQ(w1.get(), w2.get());
}

// ============================================================================
// Scenario 2: shared_from_this Requires Prior shared_ptr Ownership (Moderate)
// ============================================================================

TEST_F(OwnershipCyclesTest, SharedFromThisBeforeSharedThrows)
{
    Widget stack_widget("Stack");

    // Q: Why must `shared_from_this()` fail when no `shared_ptr` owns the object yet?
    // A:
    // R:

    EXPECT_THROW(stack_widget.get_shared(), std::bad_weak_ptr);
}

// ============================================================================
// Scenario 3: Circular shared_ptr Leaks (Moderate)
// ============================================================================

TEST_F(OwnershipCyclesTest, CircularSharedPtrLeak)
{
    {
        auto a = std::make_shared<SharedNode>("NodeA");
        auto b = std::make_shared<SharedNode>("NodeB");
        a->next = b;
        b->next = a;

        // Q: After linking both ways, why is each `use_count()` 2?
        // A:
        // R:

        EXPECT_EQ(a.use_count(), 2);
        EXPECT_EQ(b.use_count(), 2);
    }

    // Q: Locals are gone. Why is there still no `::dtor` in EventLog?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0u);
}

// ============================================================================
// Scenario 4: weak_ptr Breaks the Cycle (Moderate)
// ============================================================================

TEST_F(OwnershipCyclesTest, WeakPtrBreaksCycle)
{
    {
        auto a = std::make_shared<WeakNode>("WeakA");
        auto b = std::make_shared<WeakNode>("WeakB");
        a->next = b;
        b->next = a;

        // Q: Why do both `use_count()` values stay 1 after the link?
        // A:
        // R:

        EXPECT_EQ(a.use_count(), 1);
        EXPECT_EQ(b.use_count(), 1);
    }

    // Q: What EventLog signal proves both nodes were destroyed once the cycle
    //    no longer held strong ownership?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 2u);
}
