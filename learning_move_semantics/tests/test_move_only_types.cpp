// Test Suite: Move-Only Types
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "move_instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <utility>
#include <vector>

class MoveOnlyTypesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: unique_ptr Ownership Transfer (Easy)
// ============================================================================

TEST_F(MoveOnlyTypesTest, UniquePtrTransferLeavesSourceNull)
{
    auto ptr1 = std::make_unique<MoveTracked>("Unique");
    auto ptr2 = std::move(ptr1);

    // Q: After the move, what is `ptr1.get()`, and which EventLog counts show the
    //    MoveTracked was neither copied nor destroyed during the transfer?
    // A:
    // R:

    // Q: Why must `unique_ptr` delete its copy operations rather than share?
    // A:
    // R:

    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_NE(ptr2.get(), nullptr);
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);
}

// ============================================================================
// Scenario 2: Simple Move-Only Resource (Easy)
// ============================================================================

TEST_F(MoveOnlyTypesTest, ResourceMoveOnlyTransfer)
{
    Resource res1("MoveOnly");
    Resource res2(std::move(res1));

    // Q: Which special members make `Resource` move-only, and which EventLog
    //    signal confirms the move constructor ran?
    // A:
    // R:

    // Q: After the move, what do `res1.is_valid()` and `res2.is_valid()` report?
    // A:
    // R:

    EXPECT_FALSE(res1.is_valid());
    EXPECT_TRUE(res2.is_valid());
    EXPECT_EQ(res2.name(), "MoveOnly");
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
}

// ============================================================================
// Scenario 3: Move-Only Elements in vector (Moderate)
// ============================================================================

TEST_F(MoveOnlyTypesTest, MoveOnlyResourceInVector)
{
    std::vector<Resource> vec;
    vec.reserve(2);

    Resource named("InVector");

    EventLog::instance().clear();

    vec.push_back(std::move(named));
    vec.push_back(Resource("Temporary"));

    // Q: What prevents `push_back(named)` without `std::move` from compiling?
    // A:
    // R:

    // Q: For the temporary push, why is an explicit `std::move` unnecessary, and
    //    which EventLog counts show moves rather than copies?
    // A:
    // R:

    EXPECT_EQ(vec.size(), 2u);
    EXPECT_FALSE(named.is_valid());
    EXPECT_GE(EventLog::instance().count_events("move_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 0);
}

// ============================================================================
// Scenario 4: Factory Returning unique_ptr (Moderate)
// ============================================================================

TEST_F(MoveOnlyTypesTest, FactoryReturnsUniquePtr)
{
    auto factory = [](const std::string& name) {
        return std::make_unique<MoveTracked>(name);
    };

    EventLog::instance().clear();

    auto ptr = factory("Factory");

    // Q: Which EventLog signals show how many MoveTracked objects were created,
    //    and why returning `unique_ptr` transfers ownership without a deep copy?
    // A:
    // R:

    // Q: Who owns the MoveTracked after `factory` returns, and what happens to
    //    that object when `ptr` leaves scope?
    // A:
    // R:

    EXPECT_NE(ptr.get(), nullptr);
    EXPECT_EQ(ptr->name(), "Factory");
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);

    ptr.reset();
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}
