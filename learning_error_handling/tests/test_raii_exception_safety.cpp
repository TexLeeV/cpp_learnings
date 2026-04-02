// Test Suite: RAII for Exception Safety
// Estimated Time: 2 hours
// Difficulty: Easy

#include "instrumentation.h"

#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

class RaiiExceptionSafetyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }

    void TearDown() override
    {
        std::remove("test_raii_file.txt");
    }
};

// ============================================================================
// RAII Basics - Automatic Cleanup on Exception
// ============================================================================

class ManualResource
{
public:
    explicit ManualResource(const std::string& name) : name_(name), resource_(new Tracked(name))
    {
        EventLog::instance().record("ManualResource(" + name_ + ")::ctor");
    }

    ~ManualResource()
    {
        EventLog::instance().record("ManualResource(" + name_ + ")::dtor");
        delete resource_; // RAII ensures cleanup even on exception
    }

    ManualResource(const ManualResource&) = delete;
    ManualResource& operator=(const ManualResource&) = delete;

private:
    std::string name_;
    Tracked* resource_;
};

void operation_with_manual_cleanup()
{
    EventLog::instance().record("operation: start");

    ManualResource r1("R1");
    ManualResource r2("R2");

    EventLog::instance().record("operation: throwing exception");
    throw std::runtime_error("Simulated error");
}

// Q: What happens to r1 and r2 when the exception is thrown?
// A:
// R:

// Q: In what order are the destructors called during stack unwinding?
// A:
// R:

TEST_F(RaiiExceptionSafetyTest, BasicRAII_AutomaticCleanup)
{
    // Easy: RAII ensures cleanup even when exceptions are thrown

    try
    {
        operation_with_manual_cleanup();
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }

    // Verify both resources were cleaned up
    EXPECT_EQ(EventLog::instance().count_events("ManualResource(R1)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("ManualResource(R2)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("ManualResource(R2)::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("ManualResource(R1)::dtor"), 1);

    // Verify Tracked objects were also destroyed
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R1)::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R2)::dtor"), 1);
}

// ============================================================================
// Smart Pointers - RAII for Dynamic Memory
// ============================================================================

void operation_with_smart_pointers()
{
    EventLog::instance().record("operation: start");

    auto r1 = std::make_unique<Tracked>("R1");
    auto r2 = std::make_unique<Tracked>("R2");
    auto r3 = std::make_unique<Tracked>("R3");

    EventLog::instance().record("operation: throwing exception");
    throw std::runtime_error("Simulated error");
}

// Q: How do unique_ptr destructors ensure cleanup during stack unwinding?
// A:
// R:

TEST_F(RaiiExceptionSafetyTest, SmartPointers_AutomaticCleanup)
{
    // Easy: Smart pointers provide automatic RAII cleanup

    try
    {
        operation_with_smart_pointers();
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }

    // Verify all resources were cleaned up automatically
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R1)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R2)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R3)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R1)::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R2)::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R3)::dtor"), 1);
}

// ============================================================================
// Partial Construction - Exception in Constructor
// ============================================================================

class CompositeResource
{
public:
    CompositeResource(bool fail_on_second) : r1_(std::make_unique<Tracked>("R1")), r2_(nullptr), r3_(nullptr)
    {
        EventLog::instance().record("CompositeResource::ctor - r1 initialized");

        if (fail_on_second)
        {
            EventLog::instance().record("CompositeResource::ctor - throwing before r2");
            throw std::runtime_error("Failed during construction");
        }

        r2_ = std::make_unique<Tracked>("R2");
        EventLog::instance().record("CompositeResource::ctor - r2 initialized");

        r3_ = std::make_unique<Tracked>("R3");
        EventLog::instance().record("CompositeResource::ctor - r3 initialized");
    }

    ~CompositeResource()
    {
        EventLog::instance().record("CompositeResource::dtor");
    }

private:
    std::unique_ptr<Tracked> r1_;
    std::unique_ptr<Tracked> r2_;
    std::unique_ptr<Tracked> r3_;
};

// Q: What happens to r1 when the constructor throws before initializing r2?
// A:
// R:

// Q: Is the CompositeResource destructor called if the constructor throws?
// A:
// R:

TEST_F(RaiiExceptionSafetyTest, PartialConstruction_MemberCleanup)
{
    // Moderate: Members initialized before exception are automatically cleaned up

    try
    {
        CompositeResource resource(true); // Throws after r1 but before r2
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }

    // Verify r1 was constructed and destroyed
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R1)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R1)::dtor"), 1);

    // Verify r2 and r3 were never constructed
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R2)::ctor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(R3)::ctor"), 0);

    // Verify CompositeResource destructor was NOT called
    EXPECT_EQ(EventLog::instance().count_events("CompositeResource::dtor"), 0);

    // Q: Why is the CompositeResource destructor not called?
    // A:
    // R:
}

// ============================================================================
// Multiple Resources - Order of Cleanup
// ============================================================================

void operation_with_ordered_resources()
{
    EventLog::instance().record("operation: start");

    // TODO: Create resources in order: R1, R2, R3 using std::make_unique<Tracked>
    auto r1 = std::make_unique<Tracked>("R1");
    auto r2 = std::make_unique<Tracked>("R2");
    auto r3 = std::make_unique<Tracked>("R3");

    EventLog::instance().record("operation: all resources acquired");
    EventLog::instance().record("operation: throwing exception");
    throw std::runtime_error("Simulated error");
}

// Q: In what order are r1, r2, r3 destroyed during stack unwinding?
// A:
// R:

// Q: Why does C++ guarantee this specific destruction order?
// A:
// R:

TEST_F(RaiiExceptionSafetyTest, MultipleResources_DestructionOrder)
{
    // Moderate: Stack unwinding destroys objects in reverse construction order

    try
    {
        operation_with_ordered_resources();
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }

    // Verify construction order
    std::vector<std::string> events = EventLog::instance().events();
    size_t r1_ctor_idx = 0, r2_ctor_idx = 0, r3_ctor_idx = 0;
    size_t r1_dtor_idx = 0, r2_dtor_idx = 0, r3_dtor_idx = 0;

    for (size_t i = 0; i < events.size(); ++i)
    {
        if (events[i].find("Tracked(R1)::ctor") != std::string::npos)
            r1_ctor_idx = i;
        if (events[i].find("Tracked(R2)::ctor") != std::string::npos)
            r2_ctor_idx = i;
        if (events[i].find("Tracked(R3)::ctor") != std::string::npos)
            r3_ctor_idx = i;
        if (events[i].find("Tracked(R1)::dtor") != std::string::npos)
            r1_dtor_idx = i;
        if (events[i].find("Tracked(R2)::dtor") != std::string::npos)
            r2_dtor_idx = i;
        if (events[i].find("Tracked(R3)::dtor") != std::string::npos)
            r3_dtor_idx = i;
    }

    // Construction order: R1 -> R2 -> R3
    EXPECT_LT(r1_ctor_idx, r2_ctor_idx);
    EXPECT_LT(r2_ctor_idx, r3_ctor_idx);

    // Destruction order: R3 -> R2 -> R1 (reverse)
    EXPECT_GT(r1_dtor_idx, r2_dtor_idx);
    EXPECT_GT(r2_dtor_idx, r3_dtor_idx);
}

// ============================================================================
// Scope Guards - Generic RAII for Cleanup
// ============================================================================

template <typename F> class ScopeGuard
{
public:
    explicit ScopeGuard(F&& cleanup) : cleanup_(std::forward<F>(cleanup)), active_(true)
    {
        EventLog::instance().record("ScopeGuard::ctor");
    }

    ~ScopeGuard()
    {
        if (active_)
        {
            EventLog::instance().record("ScopeGuard::dtor - executing cleanup");
            cleanup_();
        }
    }

    void dismiss()
    {
        EventLog::instance().record("ScopeGuard::dismiss");
        active_ = false;
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    F cleanup_;
    bool active_;
};

template <typename F> ScopeGuard<F> make_scope_guard(F&& cleanup)
{
    return ScopeGuard<F>(std::forward<F>(cleanup));
}

// Q: What happens to the scope guard when an exception is thrown?
// A:
// R:

// Q: When would you use dismiss() on a scope guard?
// A:
// R:

TEST_F(RaiiExceptionSafetyTest, ScopeGuard_CleanupOnException)
{
    // Moderate: Scope guards execute cleanup even on exception

    try
    {
        EventLog::instance().record("operation: start");

        auto guard = make_scope_guard([]() { EventLog::instance().record("cleanup: executed"); });

        EventLog::instance().record("operation: throwing exception");
        throw std::runtime_error("Simulated error");
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }

    EXPECT_EQ(EventLog::instance().count_events("ScopeGuard::dtor - executing cleanup"), 1);
    EXPECT_EQ(EventLog::instance().count_events("cleanup: executed"), 1);
}

// ============================================================================
// Nested RAII - Multiple Levels of Cleanup
// ============================================================================

class OuterResource
{
public:
    explicit OuterResource(const std::string& name, bool throw_after_inner)
        : name_(name), inner_(std::make_unique<Tracked>("Inner_" + name))
    {
        EventLog::instance().record("OuterResource(" + name_ + ")::ctor - inner initialized");

        if (throw_after_inner)
        {
            EventLog::instance().record("OuterResource(" + name_ + ")::ctor - throwing");
            throw std::runtime_error("Outer construction failed");
        }

        EventLog::instance().record("OuterResource(" + name_ + ")::ctor - complete");
    }

    ~OuterResource()
    {
        EventLog::instance().record("OuterResource(" + name_ + ")::dtor");
    }

private:
    std::string name_;
    std::unique_ptr<Tracked> inner_;
};

// Q: What happens to inner_ when OuterResource constructor throws?
// A:
// R:

TEST_F(RaiiExceptionSafetyTest, NestedRAII_InnerCleanup)
{
    // Hard: Nested RAII objects are cleaned up even on partial construction

    try
    {
        OuterResource outer("Outer", true); // Throws after inner_ initialized
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }

    // Verify inner was constructed and destroyed
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Inner_Outer)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Inner_Outer)::dtor"), 1);

    // Verify outer destructor was NOT called (object never fully constructed)
    EXPECT_EQ(EventLog::instance().count_events("OuterResource(Outer)::dtor"), 0);

    // Q: What observable signal confirms that inner_ was cleaned up?
    // A:
    // R:
}
