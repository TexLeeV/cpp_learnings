// Test Suite: Custom Resource Managers
// Estimated Time: 3 hours
// Difficulty: Moderate

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <utility>

class CustomResourceManagersTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Simple Resource Handle
// ============================================================================

class ResourceHandle
{
public:
    explicit ResourceHandle(int id) : id_(id), valid_(true)
    {
        EventLog::instance().record("ResourceHandle::acquire id=" + std::to_string(id_));
    }

    ~ResourceHandle()
    {
        if (valid_)
        {
            EventLog::instance().record("ResourceHandle::release id=" + std::to_string(id_));
        }
    }

    ResourceHandle(ResourceHandle&& other) noexcept : id_(other.id_), valid_(other.valid_)
    {
        other.valid_ = false;
        EventLog::instance().record("ResourceHandle::move_ctor id=" + std::to_string(id_));
    }

    ResourceHandle& operator=(ResourceHandle&& other) noexcept
    {
        if (this != &other)
        {
            if (valid_)
            {
                EventLog::instance().record("ResourceHandle::release id=" + std::to_string(id_) +
                                            " (before move_assign)");
            }

            id_ = other.id_;
            valid_ = other.valid_;
            other.valid_ = false;

            EventLog::instance().record("ResourceHandle::move_assign id=" + std::to_string(id_));
        }
        return *this;
    }

    int id() const
    {
        return id_;
    }

    bool is_valid() const
    {
        return valid_;
    }

    ResourceHandle(const ResourceHandle&) = delete;
    ResourceHandle& operator=(const ResourceHandle&) = delete;

private:
    int id_;
    bool valid_;
};

// ============================================================================
// Scenario 1: Basic Resource Acquisition (Easy)
// ============================================================================

TEST_F(CustomResourceManagersTest, BasicResourceAcquisition)
{
    // Q: What does RAII stand for?
    // A:
    // R:

    {
        ResourceHandle handle(1);

        EXPECT_TRUE(handle.is_valid());
        EXPECT_EQ(EventLog::instance().count_events("acquire"), 1);

        // Q: When will the resource be released?
        // A:
        // R:
    }

    // Q: What observable signal confirms release occurred?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("release"), 1);
}

// ============================================================================
// Scenario 2: Resource Transfer via Move (Moderate)
// ============================================================================

TEST_F(CustomResourceManagersTest, ResourceTransferViaMove)
{
    ResourceHandle handle1(10);

    EventLog::instance().clear();

    // Q: What happens to handle1's resource during the move?
    // A:
    // R:

    ResourceHandle handle2(std::move(handle1));

    // Q: Is handle1 still valid after the move?
    // A:
    // R:

    EXPECT_FALSE(handle1.is_valid());
    EXPECT_TRUE(handle2.is_valid());
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);

    // Q: How many times will the resource be released?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Move Assignment with Active Resource (Hard)
// ============================================================================

TEST_F(CustomResourceManagersTest, MoveAssignmentWithActiveResource)
{
    ResourceHandle handle1(20);
    ResourceHandle handle2(30);

    EventLog::instance().clear();

    // Q: What happens to handle2's resource when we assign handle1 to it?
    // A:
    // R:

    handle2 = std::move(handle1);

    // Q: How many release events occurred during move assignment?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("release"), 1);

    // Both "before move_assign" and "move_assign id=" contain "move_assign"
    // So we check for the specific move_assign event
    EXPECT_EQ(EventLog::instance().count_events("move_assign id="), 1);

    EXPECT_FALSE(handle1.is_valid());
    EXPECT_TRUE(handle2.is_valid());
    EXPECT_EQ(handle2.id(), 20);
}

// ============================================================================
// Scenario 4: Resource Manager with Tracked Objects (Moderate)
// ============================================================================

class TrackedResourceManager
{
public:
    explicit TrackedResourceManager(const std::string& name) : resource_(std::make_shared<Tracked>(name))
    {
        EventLog::instance().record("TrackedResourceManager::ctor");
    }

    ~TrackedResourceManager()
    {
        EventLog::instance().record("TrackedResourceManager::dtor");
    }

    std::string name() const
    {
        return resource_->name();
    }

    TrackedResourceManager(const TrackedResourceManager&) = delete;
    TrackedResourceManager& operator=(const TrackedResourceManager&) = delete;

private:
    std::shared_ptr<Tracked> resource_;
};

TEST_F(CustomResourceManagersTest, ResourceManagerWithTracked)
{
    // Q: What resources does TrackedResourceManager manage?
    // A:
    // R:

    {
        TrackedResourceManager manager("ManagedResource");

        EXPECT_EQ(manager.name(), "ManagedResource");
        EXPECT_EQ(EventLog::instance().count_events("TrackedResourceManager::ctor"), 1);

        // Q: What is the destruction order: TrackedResourceManager or Tracked?
        // A:
        // R:
    }

    // Q: Walk through the observable signals in EventLog for destruction order
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("TrackedResourceManager::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(ManagedResource)::dtor"), 1);
}

// ============================================================================
// Scenario 5: Exception Safety in Resource Acquisition (Hard)
// ============================================================================

class ThrowingResource
{
public:
    explicit ThrowingResource(bool should_throw)
    {
        EventLog::instance().record("ThrowingResource::acquire");

        if (should_throw)
        {
            throw std::runtime_error("Acquisition failed");
        }
    }

    ~ThrowingResource()
    {
        EventLog::instance().record("ThrowingResource::release");
    }
};

TEST_F(CustomResourceManagersTest, ExceptionSafetyInAcquisition)
{
    // Q: What happens if the constructor throws?
    // A:
    // R:

    bool exception_caught = false;

    try
    {
        ThrowingResource resource(true);
    }
    catch (const std::runtime_error&)
    {
        exception_caught = true;
    }

    EXPECT_TRUE(exception_caught);

    // Q: Was the destructor called for the failed resource?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("acquire"), 1);
    EXPECT_EQ(EventLog::instance().count_events("release"), 0);

    // Q: Why is this behavior correct for RAII?
    // A:
    // R:
}
