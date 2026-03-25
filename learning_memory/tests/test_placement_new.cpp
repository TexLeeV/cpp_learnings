// Test Suite: Placement New and Aligned Storage
// Estimated Time: 3 hours
// Difficulty: Moderate

#include "instrumentation.h"

#include <cstddef>
#include <gtest/gtest.h>
#include <memory>
#include <new>
#include <type_traits>

class PlacementNewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Basic Placement New (Easy)
// ============================================================================

TEST_F(PlacementNewTest, BasicPlacementNew)
{
    // Q: What is placement new, and how does it differ from regular new?
    // A:
    // R:

    alignas(Tracked) char buffer[sizeof(Tracked)];

    // Q: What does alignas(Tracked) ensure about the buffer?
    // A:
    // R:

    // Construct object in pre-allocated buffer
    Tracked* obj = new (buffer) Tracked("PlacementObj");

    // Q: Where is the Tracked object's memory allocated?
    // A:
    // R:

    EXPECT_EQ(obj->name(), "PlacementObj");
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);

    // Q: Why must we manually call the destructor here?
    // A:
    // R:

    obj->~Tracked();

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);

    // Q: What happens if we forget to call the destructor manually?
    // A:
    // R:
}

// ============================================================================
// Scenario 2: Multiple Objects in Buffer (Moderate)
// ============================================================================

TEST_F(PlacementNewTest, MultipleObjectsInBuffer)
{
    constexpr size_t buffer_size = sizeof(Tracked) * 3;
    alignas(Tracked) char buffer[buffer_size];

    // TODO: Use placement new to construct 3 Tracked objects in the buffer
    // Hint: Calculate offset for each object
    Tracked* obj1 = new (buffer) Tracked("Obj1");
    Tracked* obj2 = new (buffer + sizeof(Tracked)) Tracked("Obj2");
    Tracked* obj3 = new (buffer + 2 * sizeof(Tracked)) Tracked("Obj3");

    // Q: What is the relationship between the pointer arithmetic and object alignment?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 3);
    EXPECT_EQ(obj1->name(), "Obj1");
    EXPECT_EQ(obj2->name(), "Obj2");
    EXPECT_EQ(obj3->name(), "Obj3");

    // Q: In what order should we destroy these objects, and why does it matter?
    // A:
    // R:

    // TODO: Destroy the objects in reverse order of construction
    obj3->~Tracked();
    obj2->~Tracked();
    obj1->~Tracked();

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 3);
}

// ============================================================================
// Scenario 3: Aligned Storage with std::aligned_storage (Moderate)
// ============================================================================

TEST_F(PlacementNewTest, AlignedStorage)
{
    // Q: What is std::aligned_storage, and why would you use it instead of a char buffer?
    // A:
    // R:

    using Storage = typename std::aligned_storage<sizeof(Tracked), alignof(Tracked)>::type;
    Storage storage;

    // Q: What guarantees does std::aligned_storage provide that char[] does not?
    // A:
    // R:

    Tracked* obj = new (&storage) Tracked("AlignedObj");

    EXPECT_EQ(obj->name(), "AlignedObj");
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);

    // Q: Can we reuse this storage after destroying the object?
    // A:
    // R:

    obj->~Tracked();

    // Reuse the storage
    Tracked* obj2 = new (&storage) Tracked("ReusedObj");
    EXPECT_EQ(obj2->name(), "ReusedObj");
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 2);

    obj2->~Tracked();
}

// ============================================================================
// Scenario 4: Manual Object Lifetime Management (Hard)
// ============================================================================

class ManualLifetimeManager
{
public:
    ManualLifetimeManager() : obj_(nullptr)
    {
    }

    void construct(const std::string& name)
    {
        // TODO: Use placement new to construct a Tracked object in storage_
        obj_ = new (&storage_) Tracked(name);
    }

    void destroy()
    {
        // TODO: Manually call destructor and set obj_ to nullptr
        if (obj_)
        {
            obj_->~Tracked();
            obj_ = nullptr;
        }
    }

    Tracked* get()
    {
        return obj_;
    }

    bool is_constructed() const
    {
        return obj_ != nullptr;
    }

    ~ManualLifetimeManager()
    {
        // Q: Why is it critical to check is_constructed() in the destructor?
        // A:
        // R:

        if (is_constructed())
        {
            destroy();
        }
    }

private:
    using Storage = typename std::aligned_storage<sizeof(Tracked), alignof(Tracked)>::type;
    Storage storage_;
    Tracked* obj_;
};

TEST_F(PlacementNewTest, ManualLifetimeManagement)
{
    ManualLifetimeManager manager;

    EXPECT_FALSE(manager.is_constructed());

    // Q: What is the state of storage_ before construct() is called?
    // A:
    // R:

    manager.construct("ManagedObj");

    EXPECT_TRUE(manager.is_constructed());
    EXPECT_EQ(manager.get()->name(), "ManagedObj");
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);

    // Q: What happens if we call construct() twice without calling destroy()?
    // A:
    // R:

    manager.destroy();

    EXPECT_FALSE(manager.is_constructed());
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);

    // Reuse the manager
    manager.construct("SecondObj");
    EXPECT_EQ(manager.get()->name(), "SecondObj");
}

// ============================================================================
// Scenario 5: Placement New with Exceptions (Hard)
// ============================================================================

class ThrowingTracked
{
public:
    explicit ThrowingTracked(const std::string& name, bool should_throw) : tracked_(name)
    {
        if (should_throw)
        {
            throw std::runtime_error("Construction failed");
        }
    }

    ~ThrowingTracked()
    {
        EventLog::instance().record("ThrowingTracked::dtor " + tracked_.name());
    }

    std::string name() const
    {
        return tracked_.name();
    }

private:
    Tracked tracked_;
};

TEST_F(PlacementNewTest, PlacementNewWithExceptions)
{
    alignas(ThrowingTracked) char buffer[sizeof(ThrowingTracked)];

    // Q: What happens to the buffer if the constructor throws?
    // A:
    // R:

    ThrowingTracked* obj = nullptr;
    bool exception_caught = false;

    try
    {
        obj = new (buffer) ThrowingTracked("WillThrow", true);
    }
    catch (const std::runtime_error&)
    {
        exception_caught = true;
    }

    EXPECT_TRUE(exception_caught);
    EXPECT_EQ(obj, nullptr);

    // Q: Do we need to call the destructor if the constructor threw?
    // A:
    // R:

    // Q: Is the buffer still usable after the failed construction?
    // A:
    // R:

    // Reuse buffer after failed construction
    obj = new (buffer) ThrowingTracked("WillSucceed", false);
    EXPECT_EQ(obj->name(), "WillSucceed");

    obj->~ThrowingTracked();
}

// ============================================================================
// Scenario 6: Placement New vs Heap Allocation (Moderate)
// ============================================================================

TEST_F(PlacementNewTest, PlacementNewVsHeapAllocation)
{
    EventLog::instance().clear();

    // Heap allocation
    Tracked* heap_obj = new Tracked("HeapObj");

    // Stack buffer with placement new
    alignas(Tracked) char buffer[sizeof(Tracked)];
    Tracked* placement_obj = new (buffer) Tracked("PlacementObj");

    // Q: What observable signal in EventLog distinguishes heap vs placement allocation?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 2);

    // Q: Which object requires delete, and which requires manual destructor call?
    // A:
    // R:

    delete heap_obj;
    placement_obj->~Tracked();

    // Q: What would happen if we called delete on placement_obj?
    // A:
    // R:
}
