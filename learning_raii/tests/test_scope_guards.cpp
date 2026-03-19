// Test Suite: Scope Guards and Cleanup Patterns
// Estimated Time: 2 hours
// Difficulty: Easy

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <functional>
#include <memory>

class ScopeGuardsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Basic Scope Guard Implementation
// ============================================================================

template<typename Func>
class ScopeGuard
{
public:
    explicit ScopeGuard(Func&& func)
    : func_(std::forward<Func>(func))
    , active_(true)
    {
        EventLog::instance().record("ScopeGuard::ctor");
    }
    
    ~ScopeGuard()
    {
        if (active_)
        {
            EventLog::instance().record("ScopeGuard::dtor - executing cleanup");
            func_();
        }
        else
        {
            EventLog::instance().record("ScopeGuard::dtor - dismissed");
        }
    }
    
    void dismiss()
    {
        active_ = false;
    }
    
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    
private:
    Func func_;
    bool active_;
};

template<typename Func>
ScopeGuard<Func> make_scope_guard(Func&& func)
{
    return ScopeGuard<Func>(std::forward<Func>(func));
}

// ============================================================================
// Scenario 1: Basic Scope Guard (Easy)
// ============================================================================

TEST_F(ScopeGuardsTest, BasicScopeGuard)
{
    // Q: What is RAII?
    // A:
    // R:
    
    bool cleanup_called = false;
    
    {
        auto guard = make_scope_guard([&cleanup_called]() {
            cleanup_called = true;
            EventLog::instance().record("Cleanup executed");
        });
        
        // Q: When will the cleanup function be called?
        // A:
        // R:
        
        EXPECT_FALSE(cleanup_called);
    }
    
    // Q: What guarantees that cleanup_called is now true?
    // A:
    // R:
    
    EXPECT_TRUE(cleanup_called);
    EXPECT_EQ(EventLog::instance().count_events("Cleanup executed"), 1);
}

// ============================================================================
// Scenario 2: Scope Guard with Exception (Moderate)
// ============================================================================

TEST_F(ScopeGuardsTest, ScopeGuardWithException)
{
    bool cleanup_called = false;
    
    // Q: What happens to the scope guard if an exception is thrown?
    // A:
    // R:
    
    try
    {
        auto guard = make_scope_guard([&cleanup_called]() {
            cleanup_called = true;
            EventLog::instance().record("Exception cleanup");
        });
        
        throw std::runtime_error("Test exception");
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Q: Was the cleanup function called despite the exception?
    // A:
    // R:
    
    EXPECT_TRUE(cleanup_called);
    EXPECT_EQ(EventLog::instance().count_events("Exception cleanup"), 1);
}

// ============================================================================
// Scenario 3: Dismissible Scope Guard (Moderate)
// ============================================================================

TEST_F(ScopeGuardsTest, DismissibleScopeGuard)
{
    bool cleanup_called = false;
    
    {
        auto guard = make_scope_guard([&cleanup_called]() {
            cleanup_called = true;
        });
        
        // Q: What does dismiss() do?
        // A:
        // R:
        
        guard.dismiss();
    }
    
    // Q: Was the cleanup function called?
    // A:
    // R:
    
    EXPECT_FALSE(cleanup_called);
    EXPECT_EQ(EventLog::instance().count_events("dismissed"), 1);
}

// ============================================================================
// Scenario 4: Multiple Scope Guards (Moderate)
// ============================================================================

TEST_F(ScopeGuardsTest, MultipleScopeGuards)
{
    std::vector<int> execution_order;
    
    {
        auto guard1 = make_scope_guard([&execution_order]() {
            execution_order.push_back(1);
        });
        
        auto guard2 = make_scope_guard([&execution_order]() {
            execution_order.push_back(2);
        });
        
        auto guard3 = make_scope_guard([&execution_order]() {
            execution_order.push_back(3);
        });
        
        // Q: In what order will the guards execute their cleanup functions?
        // A:
        // R:
    }
    
    // Q: Why is this order important for resource cleanup?
    // A:
    // R:
    
    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 3);
    EXPECT_EQ(execution_order[1], 2);
    EXPECT_EQ(execution_order[2], 1);
}

// ============================================================================
// Scenario 5: Scope Guard with Tracked Objects (Hard)
// ============================================================================

TEST_F(ScopeGuardsTest, ScopeGuardWithTracked)
{
    std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Resource");
    
    EventLog::instance().clear();
    
    // Q: What is ptr's use_count before the scope guard?
    // A:
    // R:
    
    EXPECT_EQ(ptr.use_count(), 1);
    
    {
        auto guard = make_scope_guard([ptr]() {
            EventLog::instance().record("Guard cleanup with " + ptr->name());
        });
        
        // Q: What is ptr's use_count inside the scope?
        // A:
        // R:
        
        EXPECT_EQ(ptr.use_count(), 2);
    }
    
    // Q: What is ptr's use_count after the scope guard is destroyed?
    // A:
    // R:
    
    EXPECT_EQ(ptr.use_count(), 1);
    
    // Q: Walk through the destruction order: guard's destructor, then what?
    // A:
    // R:
}

// ============================================================================
// Scenario 6: RAII vs Manual Cleanup (Hard)
// ============================================================================

class ManualResource
{
public:
    ManualResource()
    {
        EventLog::instance().record("ManualResource::acquire");
    }
    
    void release()
    {
        EventLog::instance().record("ManualResource::release");
    }
};

class RAIIResource
{
public:
    RAIIResource()
    {
        EventLog::instance().record("RAIIResource::acquire");
    }
    
    ~RAIIResource()
    {
        EventLog::instance().record("RAIIResource::release");
    }
    
    RAIIResource(const RAIIResource&) = delete;
    RAIIResource& operator=(const RAIIResource&) = delete;
};

TEST_F(ScopeGuardsTest, RAIIvsManualCleanup)
{
    // Manual cleanup - error prone
    {
        ManualResource manual;
        
        // Q: What happens if an exception is thrown before release()?
        // A:
        // R:
        
        // manual.release();  // Easy to forget!
    }
    
    EXPECT_EQ(EventLog::instance().count_events("ManualResource::release"), 0);
    
    EventLog::instance().clear();
    
    // RAII cleanup - automatic
    {
        RAIIResource raii;
        
        // Q: What guarantees that release will be called?
        // A:
        // R:
    }
    
    // Q: What observable signal shows RAII cleanup occurred?
    // A:
    // R:
    
    EXPECT_EQ(EventLog::instance().count_events("RAIIResource::release"), 1);
}
