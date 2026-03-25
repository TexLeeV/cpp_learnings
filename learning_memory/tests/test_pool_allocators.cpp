// Test Suite: Pool Allocators (Fixed-Size and Variable-Size)
// Estimated Time: 5 hours
// Difficulty: Hard

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <new>
#include <vector>

class PoolAllocatorsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Fixed-Size Pool Allocator Implementation
// ============================================================================

template <typename T, size_t PoolSize> class FixedSizePool
{
public:
    FixedSizePool() : next_free_(0)
    {
        for (size_t i = 0; i < PoolSize; ++i)
        {
            free_list_[i] = true;
        }

        EventLog::instance().record("FixedSizePool::ctor (pool_size=" + std::to_string(PoolSize) + ")");
    }

    ~FixedSizePool()
    {
        EventLog::instance().record("FixedSizePool::dtor");
    }

    T* allocate()
    {
        // Q: What happens if the pool is exhausted?
        // A:
        // R:

        for (size_t i = 0; i < PoolSize; ++i)
        {
            if (free_list_[i])
            {
                free_list_[i] = false;

                EventLog::instance().record("FixedSizePool::allocate slot " + std::to_string(i));

                return reinterpret_cast<T*>(&storage_[i]);
            }
        }

        throw std::bad_alloc();
    }

    void deallocate(T* ptr)
    {
        // Q: How do we determine which slot in the pool this pointer belongs to?
        // A:
        // R:

        char* pool_start = reinterpret_cast<char*>(&storage_[0]);
        char* ptr_addr = reinterpret_cast<char*>(ptr);

        ptrdiff_t offset = ptr_addr - pool_start;
        size_t slot = offset / sizeof(Storage);

        // Q: What would happen if ptr doesn't belong to this pool?
        // A:
        // R:

        if (slot >= PoolSize)
        {
            throw std::invalid_argument("Pointer not from this pool");
        }

        free_list_[slot] = true;

        EventLog::instance().record("FixedSizePool::deallocate slot " + std::to_string(slot));
    }

    size_t available() const
    {
        size_t count = 0;
        for (size_t i = 0; i < PoolSize; ++i)
        {
            if (free_list_[i])
            {
                count++;
            }
        }
        return count;
    }

private:
    using Storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    Storage storage_[PoolSize];
    bool free_list_[PoolSize];
    size_t next_free_;
};

// ============================================================================
// Scenario 1: Fixed-Size Pool Basic Operations (Easy)
// ============================================================================

TEST_F(PoolAllocatorsTest, FixedSizePoolBasics)
{
    FixedSizePool<Tracked, 3> pool;

    EXPECT_EQ(pool.available(), 3);

    // Q: What is the advantage of a fixed-size pool over calling new/delete repeatedly?
    // A:
    // R:

    EventLog::instance().clear();

    // TODO: Allocate memory for first object
    Tracked* obj1 = pool.allocate();

    EXPECT_EQ(pool.available(), 2);
    EXPECT_EQ(EventLog::instance().count_events("FixedSizePool::allocate"), 1);

    // Q: Is obj1 constructed at this point?
    // A:
    // R:

    // TODO: Construct obj1 using placement new
    new (obj1) Tracked("PoolObj1");

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);

    // Allocate and construct second object
    Tracked* obj2 = pool.allocate();
    new (obj2) Tracked("PoolObj2");

    EXPECT_EQ(pool.available(), 1);

    // Q: What happens when we destroy and deallocate obj1?
    // A:
    // R:

    obj1->~Tracked();
    pool.deallocate(obj1);

    EXPECT_EQ(pool.available(), 2);
    EXPECT_EQ(EventLog::instance().count_events("FixedSizePool::deallocate"), 1);

    // Q: Can we reuse the slot that obj1 occupied?
    // A:
    // R:

    // Cleanup
    obj2->~Tracked();
    pool.deallocate(obj2);
}

// ============================================================================
// Scenario 2: Pool Exhaustion (Moderate)
// ============================================================================

TEST_F(PoolAllocatorsTest, PoolExhaustion)
{
    FixedSizePool<Tracked, 2> pool;

    Tracked* obj1 = pool.allocate();
    new (obj1) Tracked("Obj1");

    Tracked* obj2 = pool.allocate();
    new (obj2) Tracked("Obj2");

    EXPECT_EQ(pool.available(), 0);

    // Q: What exception is thrown when the pool is exhausted?
    // A:
    // R:

    // TODO: Try to allocate when pool is full
    bool exception_thrown = false;
    try
    {
        Tracked* obj3 = pool.allocate();
        (void)obj3;
    }
    catch (const std::bad_alloc&)
    {
        exception_thrown = true;
    }

    EXPECT_TRUE(exception_thrown);

    // Q: After deallocating one object, can we allocate again?
    // A:
    // R:

    obj1->~Tracked();
    pool.deallocate(obj1);

    EXPECT_EQ(pool.available(), 1);

    Tracked* obj3 = pool.allocate();
    new (obj3) Tracked("Obj3");

    EXPECT_EQ(pool.available(), 0);

    // Cleanup
    obj2->~Tracked();
    pool.deallocate(obj2);
    obj3->~Tracked();
    pool.deallocate(obj3);
}

// ============================================================================
// Scenario 3: Pool Reuse and Fragmentation (Hard)
// ============================================================================

TEST_F(PoolAllocatorsTest, PoolReuseAndFragmentation)
{
    FixedSizePool<Tracked, 4> pool;

    // Allocate all slots
    Tracked* obj1 = pool.allocate();
    new (obj1) Tracked("Obj1");

    Tracked* obj2 = pool.allocate();
    new (obj2) Tracked("Obj2");

    Tracked* obj3 = pool.allocate();
    new (obj3) Tracked("Obj3");

    Tracked* obj4 = pool.allocate();
    new (obj4) Tracked("Obj4");

    EXPECT_EQ(pool.available(), 0);

    // Q: What is the memory layout of these 4 objects in the pool?
    // A:
    // R:

    // Deallocate obj2 (middle slot)
    obj2->~Tracked();
    pool.deallocate(obj2);

    EXPECT_EQ(pool.available(), 1);

    // Q: Can we allocate a new object in the freed slot?
    // A:
    // R:

    Tracked* obj5 = pool.allocate();
    new (obj5) Tracked("Obj5");

    // Q: What is the relationship between obj2's address and obj5's address?
    // A:
    // R:

    EXPECT_EQ(obj2, obj5);

    // Q: What would happen if we tried to deallocate obj2 again?
    // A:
    // R:

    // Cleanup
    obj1->~Tracked();
    pool.deallocate(obj1);
    obj3->~Tracked();
    pool.deallocate(obj3);
    obj4->~Tracked();
    pool.deallocate(obj4);
    obj5->~Tracked();
    pool.deallocate(obj5);
}
