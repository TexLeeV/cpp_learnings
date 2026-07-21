// Test Suite: Custom unique_ptr from Scratch
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <utility>

class UniquePtrFromScratchTest : public ::testing::Test
{
protected:
    void SetUp() override { EventLog::instance().clear(); }
};

template <typename T> class UniquePtr
{
public:
    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr)
    {
        EventLog::instance().record("UniquePtr::ctor");
    }

    ~UniquePtr()
    {
        EventLog::instance().record(ptr_ ? "UniquePtr::dtor - deleting" : "UniquePtr::dtor - empty");
        delete ptr_;
    }

    UniquePtr(UniquePtr&& other) noexcept : ptr_(other.ptr_)
    {
        other.ptr_ = nullptr;
        EventLog::instance().record("UniquePtr::move_ctor");
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept
    {
        if (this != &other)
        {
            delete ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
            EventLog::instance().record("UniquePtr::move_assign");
        }
        return *this;
    }

    T* get() const { return ptr_; }
    T& operator*() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    explicit operator bool() const { return ptr_ != nullptr; }

    T* release()
    {
        T* temp = ptr_;
        ptr_ = nullptr;
        EventLog::instance().record("UniquePtr::release");
        return temp;
    }

    void reset(T* ptr = nullptr)
    {
        delete ptr_;
        ptr_ = ptr;
        EventLog::instance().record("UniquePtr::reset");
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

private:
    T* ptr_;
};

// ============================================================================
// Scenario 1: Basics (Easy)
// ============================================================================

TEST_F(UniquePtrFromScratchTest, BasicsOwnAndDeleteOnScopeExit)
{
    {
        UniquePtr<Tracked> ptr(new Tracked("Unique"));

        // Q: What ownership rule does UniquePtr encode, and when does Tracked die?
        // A:
        // R:

        EXPECT_TRUE(ptr);
        EXPECT_EQ(ptr->name(), "Unique");
        EXPECT_EQ(EventLog::instance().count_events("UniquePtr::ctor"), 1);
    }

    // Q: Which two EventLog substrings confirm UniquePtr deleted Tracked?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("UniquePtr::dtor - deleting"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Unique)::dtor"), 1);
}

// ============================================================================
// Scenario 2: Move Transfers Ownership (Moderate)
// ============================================================================

TEST_F(UniquePtrFromScratchTest, MoveTransfersOwnership)
{
    UniquePtr<Tracked> ptr1(new Tracked("Movable"));
    EventLog::instance().clear();
    UniquePtr<Tracked> ptr2(std::move(ptr1));

    // Q: After the move, what are `ptr1` and `ptr2`, and why is Tracked still alive?
    // A:
    // R:

    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2->name(), "Movable");
    EXPECT_EQ(EventLog::instance().count_events("UniquePtr::move_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);

    // Q: Why would a copy constructor for UniquePtr break the ownership rule?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: release and reset (Moderate)
// ============================================================================

TEST_F(UniquePtrFromScratchTest, ReleaseAndReset)
{
    UniquePtr<Tracked> ptr(new Tracked("Original"));
    EventLog::instance().clear();
    Tracked* raw = ptr.release();

    // Q: After `release()`, who owns `raw`, and what is `ptr`'s state?
    // A:
    // R:

    EXPECT_FALSE(ptr);
    EXPECT_EQ(raw->name(), "Original");
    EXPECT_EQ(EventLog::instance().count_events("UniquePtr::release"), 1);

    delete raw;
    EventLog::instance().clear();
    ptr.reset(new Tracked("Reset"));

    // Q: What does `reset` delete (if anything) before taking the new pointer?
    // A:
    // R:

    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr->name(), "Reset");
    EXPECT_EQ(EventLog::instance().count_events("UniquePtr::reset"), 1);
}
