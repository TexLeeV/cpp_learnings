// Test Suite: Custom Allocators and std::allocator Interface
// Estimated Time: 4 hours
// Difficulty: Hard

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class CustomAllocatorsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Understanding std::allocator Interface (Easy)
// ============================================================================

TEST_F(CustomAllocatorsTest, StdAllocatorBasics)
{
    std::allocator<Tracked> alloc;

    // Q: What does std::allocator<T>::allocate(n) return?
    // A:
    // R:

    // TODO: Allocate memory for 1 Tracked object
    Tracked* ptr = alloc.allocate(1);

    // Q: Is the object constructed at this point?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 0);

    // Q: What is the difference between allocate() and construct()?
    // A:
    // R:

    // TODO: Construct the object using std::allocator_traits
    std::allocator_traits<std::allocator<Tracked>>::construct(alloc, ptr, "AllocatedObj");

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);
    EXPECT_EQ(ptr->name(), "AllocatedObj");

    // Q: In what order must we call destroy() and deallocate()?
    // A:
    // R:

    // TODO: Destroy the object
    std::allocator_traits<std::allocator<Tracked>>::destroy(alloc, ptr);

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);

    // TODO: Deallocate the memory
    alloc.deallocate(ptr, 1);

    // Q: What would happen if we deallocated before destroying?
    // A:
    // R:
}

// ============================================================================
// Scenario 2: Custom Logging Allocator (Moderate)
// ============================================================================

template <typename T> class LoggingAllocator
{
public:
    using value_type = T;

    LoggingAllocator() noexcept = default;

    template <typename U> LoggingAllocator(const LoggingAllocator<U>&) noexcept
    {
    }

    T* allocate(std::size_t n)
    {
        // Q: Why do we multiply n by sizeof(T)?
        // A:
        // R:

        std::ostringstream oss;
        oss << "LoggingAllocator::allocate(" << n << " objects, " << (n * sizeof(T)) << " bytes)";
        EventLog::instance().record(oss.str());

        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* ptr, std::size_t n) noexcept
    {
        std::ostringstream oss;
        oss << "LoggingAllocator::deallocate(" << n << " objects, " << (n * sizeof(T)) << " bytes)";
        EventLog::instance().record(oss.str());

        ::operator delete(ptr);
    }
};

template <typename T, typename U> bool operator==(const LoggingAllocator<T>&, const LoggingAllocator<U>&)
{
    return true;
}

template <typename T, typename U> bool operator!=(const LoggingAllocator<T>&, const LoggingAllocator<U>&)
{
    return false;
}

TEST_F(CustomAllocatorsTest, LoggingAllocatorWithVector)
{
    // Q: When does std::vector call allocate()?
    // A:
    // R:

    std::vector<Tracked, LoggingAllocator<Tracked>> vec;

    // Q: Has any allocation occurred yet?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("LoggingAllocator::allocate"), 0);

    // TODO: Add one element to trigger allocation
    vec.emplace_back("Vec1");

    // Q: How many allocate() calls occurred, and for how many objects?
    // A:
    // R:

    EXPECT_GE(EventLog::instance().count_events("LoggingAllocator::allocate"), 1);
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);

    // TODO: Add more elements to trigger reallocation
    vec.emplace_back("Vec2");
    vec.emplace_back("Vec3");

    // Q: What happens to the old allocation when std::vector reallocates?
    // A:
    // R:

    // Q: Walk through the observable signals: how many allocate() and deallocate() calls?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Allocator Rebinding (Hard)
// ============================================================================

TEST_F(CustomAllocatorsTest, AllocatorRebinding)
{
    // Q: What is allocator rebinding, and why is it necessary?
    // A:
    // R:

    using NodeAllocator = typename std::allocator_traits<LoggingAllocator<int>>::rebind_alloc<Tracked>;

    // Q: What type is NodeAllocator?
    // A:
    // R:

    NodeAllocator node_alloc;
    Tracked* ptr = node_alloc.allocate(1);

    std::allocator_traits<NodeAllocator>::construct(node_alloc, ptr, "ReboundObj");

    EXPECT_EQ(ptr->name(), "ReboundObj");

    // Q: Why would std::list<int, LoggingAllocator<int>> need to rebind to allocate list nodes?
    // A:
    // R:

    std::allocator_traits<NodeAllocator>::destroy(node_alloc, ptr);
    node_alloc.deallocate(ptr, 1);
}

// ============================================================================
// Scenario 4: Stateful Allocator (Hard)
// ============================================================================

template <typename T> class CountingAllocator
{
public:
    using value_type = T;

    CountingAllocator()
        : allocation_count_(std::make_shared<size_t>(0)), deallocation_count_(std::make_shared<size_t>(0))
    {
    }

    template <typename U>
    CountingAllocator(const CountingAllocator<U>& other) noexcept
        : allocation_count_(other.allocation_count_), deallocation_count_(other.deallocation_count_)
    {
    }

    T* allocate(std::size_t n)
    {
        (*allocation_count_)++;
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* ptr, std::size_t n) noexcept
    {
        (*deallocation_count_)++;
        ::operator delete(ptr);
    }

    size_t allocation_count() const
    {
        return *allocation_count_;
    }

    size_t deallocation_count() const
    {
        return *deallocation_count_;
    }

    template <typename U> friend class CountingAllocator;

private:
    std::shared_ptr<size_t> allocation_count_;
    std::shared_ptr<size_t> deallocation_count_;
};

template <typename T, typename U> bool operator==(const CountingAllocator<T>& lhs, const CountingAllocator<U>& rhs)
{
    return lhs.allocation_count_ == rhs.allocation_count_;
}

template <typename T, typename U> bool operator!=(const CountingAllocator<T>& lhs, const CountingAllocator<U>& rhs)
{
    return !(lhs == rhs);
}

TEST_F(CustomAllocatorsTest, StatefulAllocator)
{
    CountingAllocator<int> alloc;

    // Q: Why does CountingAllocator use shared_ptr for its counters?
    // A:
    // R:

    std::vector<int, CountingAllocator<int>> vec(alloc);

    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);

    // Q: How many allocations occurred during these 5 push_back calls?
    // A:
    // R:

    EXPECT_GE(alloc.allocation_count(), 1);

    // Q: Why might allocation_count be greater than 1?
    // A:
    // R:

    size_t alloc_count = alloc.allocation_count();
    size_t dealloc_count = alloc.deallocation_count();

    // Q: What is the relationship between allocation_count and deallocation_count before vec is destroyed?
    // A:
    // R:

    EXPECT_EQ(alloc_count - dealloc_count, 1);
}
