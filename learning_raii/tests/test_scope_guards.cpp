// Test Suite: Scope Guards and Cleanup
// Estimated Time: 1-2 hours
// Difficulty: Easy
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

class ScopeGuardsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

template <typename Func> class ScopeGuard
{
public:
    explicit ScopeGuard(Func&& func) : func_(std::forward<Func>(func)), active_(true) {}

    ~ScopeGuard()
    {
        if (active_)
        {
            EventLog::instance().record("ScopeGuard::cleanup");
            func_();
        }
        else
        {
            EventLog::instance().record("ScopeGuard::dismissed");
        }
    }

    void dismiss() { active_ = false; }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    Func func_;
    bool active_;
};

// ============================================================================
// Scenario 1: Basic Guard Runs Cleanup (Easy)
// ============================================================================

TEST_F(ScopeGuardsTest, BasicGuardRunsCleanupOnScopeExit)
{
    bool cleanup_called = false;

    {
        ScopeGuard guard([&]() {
            cleanup_called = true;
            EventLog::instance().record("Cleanup executed");
        });

        // Q: Why is `cleanup_called` still false here, while the guard already exists?
        // A:
        // R:

        EXPECT_FALSE(cleanup_called);
    }

    // Q: What language mechanism guarantees cleanup ran after the closing brace?
    // A:
    // R:

    EXPECT_TRUE(cleanup_called);
    EXPECT_EQ(EventLog::instance().count_events("Cleanup executed"), 1);
}

// ============================================================================
// Scenario 2: Cleanup on Exception (Moderate)
// ============================================================================

TEST_F(ScopeGuardsTest, CleanupRunsWhenExceptionUnwinds)
{
    bool cleanup_called = false;

    try
    {
        ScopeGuard guard([&]() {
            cleanup_called = true;
            EventLog::instance().record("Exception cleanup");
        });

        // Q: During stack unwinding, which ScopeGuard path still runs?
        // A:
        // R:

        throw std::runtime_error("Test exception");
    }
    catch (const std::runtime_error&)
    {
    }

    // Q: Which EventLog substring proves cleanup happened despite the throw?
    // A:
    // R:

    EXPECT_TRUE(cleanup_called);
    EXPECT_EQ(EventLog::instance().count_events("Exception cleanup"), 1);
}

// ============================================================================
// Scenario 3: Dismissible Guard (Moderate)
// ============================================================================

TEST_F(ScopeGuardsTest, DismissSkipsCleanup)
{
    bool cleanup_called = false;

    {
        ScopeGuard guard([&]() { cleanup_called = true; });

        // Q: After `dismiss()`, what must the destructor skip, and what does it still log?
        // A:
        // R:

        guard.dismiss();
    }

    EXPECT_FALSE(cleanup_called);
    EXPECT_EQ(EventLog::instance().count_events("ScopeGuard::dismissed"), 1);
    EXPECT_EQ(EventLog::instance().count_events("ScopeGuard::cleanup"), 0);
}

// ============================================================================
// Scenario 4: LIFO Cleanup Order (Moderate)
// ============================================================================

TEST_F(ScopeGuardsTest, GuardsCleanUpInReverseConstructionOrder)
{
    std::vector<int> order;

    {
        ScopeGuard g1([&]() { order.push_back(1); });
        ScopeGuard g2([&]() { order.push_back(2); });
        ScopeGuard g3([&]() { order.push_back(3); });

        // Q: Why must cleanup run 3, then 2, then 1 rather than construction order?
        // A:
        // R:
    }

    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 3);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 1);
}
