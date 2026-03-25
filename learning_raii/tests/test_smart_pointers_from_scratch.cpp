// Test Suite: Building Smart Pointers from Scratch
// Estimated Time: 4 hours
// Difficulty: Moderate

#include "instrumentation.h"

#include <atomic>
#include <gtest/gtest.h>
#include <utility>

class SmartPointersFromScratchTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Custom unique_ptr Implementation
// ============================================================================

template <typename T> class UniquePtr
{
public:
    explicit UniquePtr(T* ptr = nullptr) : ptr_(ptr)
    {
        EventLog::instance().record("UniquePtr::ctor");
    }

    ~UniquePtr()
    {
        if (ptr_)
        {
            EventLog::instance().record("UniquePtr::dtor - deleting");
            delete ptr_;
        }
        else
        {
            EventLog::instance().record("UniquePtr::dtor - empty");
        }
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
            if (ptr_)
            {
                delete ptr_;
            }
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
            EventLog::instance().record("UniquePtr::move_assign");
        }
        return *this;
    }

    T* get() const
    {
        return ptr_;
    }

    T& operator*() const
    {
        return *ptr_;
    }

    T* operator->() const
    {
        return ptr_;
    }

    explicit operator bool() const
    {
        return ptr_ != nullptr;
    }

    T* release()
    {
        T* temp = ptr_;
        ptr_ = nullptr;
        EventLog::instance().record("UniquePtr::release");
        return temp;
    }

    void reset(T* ptr = nullptr)
    {
        if (ptr_)
        {
            delete ptr_;
        }
        ptr_ = ptr;
        EventLog::instance().record("UniquePtr::reset");
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

private:
    T* ptr_;
};

// ============================================================================
// Scenario 1: Custom unique_ptr Basics (Easy)
// ============================================================================

TEST_F(SmartPointersFromScratchTest, CustomUniquePtrBasics)
{
    // Q: What ownership semantics does unique_ptr provide?
    // A:
    // R:

    {
        UniquePtr<Tracked> ptr(new Tracked("Unique"));

        EXPECT_TRUE(ptr);
        EXPECT_EQ(ptr->name(), "Unique");
        EXPECT_EQ(EventLog::instance().count_events("UniquePtr::ctor"), 1);

        // Q: When will the Tracked object be deleted?
        // A:
        // R:
    }

    // Q: What observable signal confirms deletion occurred?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("UniquePtr::dtor - deleting"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Unique)::dtor"), 1);
}

// ============================================================================
// Scenario 2: unique_ptr Move Semantics (Moderate)
// ============================================================================

TEST_F(SmartPointersFromScratchTest, UniquePtrMoveSemantics)
{
    UniquePtr<Tracked> ptr1(new Tracked("Movable"));

    EventLog::instance().clear();

    // Q: Why can't we copy a unique_ptr?
    // A:
    // R:

    UniquePtr<Tracked> ptr2(std::move(ptr1));

    // Q: What is the state of ptr1 after the move?
    // A:
    // R:

    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2->name(), "Movable");
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);

    // Q: How many Tracked objects will be deleted?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: unique_ptr Release and Reset (Moderate)
// ============================================================================

TEST_F(SmartPointersFromScratchTest, UniquePtrReleaseAndReset)
{
    UniquePtr<Tracked> ptr(new Tracked("Original"));

    EventLog::instance().clear();

    // Q: What does release() do?
    // A:
    // R:

    Tracked* raw_ptr = ptr.release();

    EXPECT_FALSE(ptr);
    EXPECT_EQ(raw_ptr->name(), "Original");
    EXPECT_EQ(EventLog::instance().count_events("UniquePtr::release"), 1);

    // Q: Who is responsible for deleting raw_ptr now?
    // A:
    // R:

    delete raw_ptr;

    EventLog::instance().clear();

    // Q: What does reset() do?
    // A:
    // R:

    ptr.reset(new Tracked("Reset"));

    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr->name(), "Reset");
    EXPECT_EQ(EventLog::instance().count_events("UniquePtr::reset"), 1);
}

// ============================================================================
// Custom shared_ptr Implementation (Simplified)
// ============================================================================

template <typename T> class SharedPtr
{
public:
    explicit SharedPtr(T* ptr = nullptr) : ptr_(ptr), ref_count_(ptr ? new std::atomic<long>(1) : nullptr)
    {
        EventLog::instance().record("SharedPtr::ctor use_count=" + std::to_string(use_count()));
    }

    ~SharedPtr()
    {
        release();
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), ref_count_(other.ref_count_)
    {
        if (ref_count_)
        {
            (*ref_count_)++;
        }
        EventLog::instance().record("SharedPtr::copy_ctor use_count=" + std::to_string(use_count()));
    }

    SharedPtr& operator=(const SharedPtr& other)
    {
        if (this != &other)
        {
            release();
            ptr_ = other.ptr_;
            ref_count_ = other.ref_count_;
            if (ref_count_)
            {
                (*ref_count_)++;
            }
            EventLog::instance().record("SharedPtr::copy_assign use_count=" + std::to_string(use_count()));
        }
        return *this;
    }

    T* get() const
    {
        return ptr_;
    }

    T& operator*() const
    {
        return *ptr_;
    }

    T* operator->() const
    {
        return ptr_;
    }

    long use_count() const
    {
        return ref_count_ ? ref_count_->load() : 0;
    }

    explicit operator bool() const
    {
        return ptr_ != nullptr;
    }

private:
    void release()
    {
        if (ref_count_)
        {
            long count = --(*ref_count_);
            EventLog::instance().record("SharedPtr::release use_count=" + std::to_string(count));

            if (count == 0)
            {
                delete ptr_;
                delete ref_count_;
                EventLog::instance().record("SharedPtr::deleted_object");
            }
        }
    }

    T* ptr_;
    std::atomic<long>* ref_count_;
};

// ============================================================================
// Scenario 4: Custom shared_ptr Basics (Moderate)
// ============================================================================

TEST_F(SmartPointersFromScratchTest, CustomSharedPtrBasics)
{
    // Q: What is the key difference between unique_ptr and shared_ptr?
    // A:
    // R:

    SharedPtr<Tracked> ptr1(new Tracked("Shared"));

    EXPECT_EQ(ptr1.use_count(), 1);

    // Q: What does the control block (ref_count_) store?
    // A:
    // R:

    SharedPtr<Tracked> ptr2 = ptr1;

    // Q: What happened to use_count during the copy?
    // A:
    // R:

    EXPECT_EQ(ptr1.use_count(), 2);
    EXPECT_EQ(ptr2.use_count(), 2);

    EXPECT_EQ(ptr1.get(), ptr2.get());
}

// ============================================================================
// Scenario 5: shared_ptr Reference Counting (Hard)
// ============================================================================

TEST_F(SmartPointersFromScratchTest, SharedPtrReferenceCounting)
{
    SharedPtr<Tracked> ptr1(new Tracked("RefCount"));

    EventLog::instance().clear();

    {
        SharedPtr<Tracked> ptr2 = ptr1;
        EXPECT_EQ(ptr1.use_count(), 2);

        {
            SharedPtr<Tracked> ptr3 = ptr1;
            EXPECT_EQ(ptr1.use_count(), 3);

            // Q: What happens when ptr3 goes out of scope?
            // A:
            // R:
        }

        // Q: What is use_count now?
        // A:
        // R:

        EXPECT_EQ(ptr1.use_count(), 2);
    }

    // Q: What is use_count after ptr2 is destroyed?
    // A:
    // R:

    EXPECT_EQ(ptr1.use_count(), 1);

    // Q: Walk through the EventLog: how many release events occurred?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("SharedPtr::release"), 2);
}

// ============================================================================
// Scenario 6: shared_ptr Final Deletion (Hard)
// ============================================================================

TEST_F(SmartPointersFromScratchTest, SharedPtrFinalDeletion)
{
    {
        SharedPtr<Tracked> ptr1(new Tracked("Final"));
        SharedPtr<Tracked> ptr2 = ptr1;
        SharedPtr<Tracked> ptr3 = ptr1;

        EXPECT_EQ(ptr1.use_count(), 3);

        EventLog::instance().clear();

        // Q: When will the Tracked object be deleted?
        // A:
        // R:
    }

    // Q: How many release events occurred?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("SharedPtr::release"), 3);

    // Q: What observable signal confirms the object was deleted?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("deleted_object"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Final)::dtor"), 1);
}
