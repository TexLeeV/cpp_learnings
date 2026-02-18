#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class SmartPointerContrastTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(SmartPointerContrastTest, UniquePtrVsSharedPtrOwnership)
{
    long shared_count = 0;
    
    {
        std::unique_ptr<Tracked> unique = std::make_unique<Tracked>("Unique");
        std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("Shared");
        
        shared_count = shared.use_count();
        std::shared_ptr<Tracked> shared_copy = shared;
        
        // Q: Can you copy unique_ptr? Why or why not?
        // A:
        // R:
        
        // Q: What is the fundamental ownership difference between unique_ptr and shared_ptr?
        // A:
        // R:
    }
    
    EXPECT_EQ(shared_count, 1);
}

TEST_F(SmartPointerContrastTest, UniquePtrMoveSemantics)
{
    // TODO: Create u1
    // YOUR CODE HERE
    
    // TODO: Move u1 to u2
    // YOUR CODE HERE
    
    bool u1_is_null = false;
    bool u2_is_valid = false;
    // TODO: Check if u1 is null and u2 is valid
    // u1_is_null = ???
    // u2_is_valid = ???
    
    EXPECT_TRUE(u1_is_null);
    EXPECT_TRUE(u2_is_valid);
}

TEST_F(SmartPointerContrastTest, SharedPtrCopyVsUniquePtrMove)
{
    EventLog::instance().clear();
    
    {
        // TODO: Create s1 and copy to s2
        // YOUR CODE HERE
    }
    
    size_t shared_events = EventLog::instance().events().size();
    
    EventLog::instance().clear();
    
    {
        // TODO: Create u1 and move to u2
        // YOUR CODE HERE
    }
    
    size_t unique_events = EventLog::instance().events().size();
    
    // Question: Which has more events? Why?
    EXPECT_GT(shared_events, 0);
    EXPECT_GT(unique_events, 0);
}

TEST_F(SmartPointerContrastTest, RawPointerDangerVsSmartPointer)
{
    Tracked* raw = new Tracked("Raw");
    
    {
        // TODO: Create shared_ptr
        // YOUR CODE HERE
    }
    
    // shared_ptr automatically cleaned up
    // But raw pointer is still allocated!
    
    bool raw_still_allocated = true;
    
    delete raw;  // Manual cleanup required
    
    EXPECT_TRUE(raw_still_allocated);
}

TEST_F(SmartPointerContrastTest, CustomDeleterComparison)
{
    {
        // TODO: Create unique_ptr with LoggingDeleter
        // Hint: std::unique_ptr<Tracked, LoggingDeleter<Tracked>>
        // YOUR CODE HERE
    }
    
    auto unique_events = EventLog::instance().events();
    
    EventLog::instance().clear();
    
    {
        // TODO: Create shared_ptr with LoggingDeleter
        // YOUR CODE HERE
    }
    
    auto shared_events = EventLog::instance().events();
    
    // Question: What's the difference in deleter storage?
    EXPECT_GT(unique_events.size(), 0);
    EXPECT_GT(shared_events.size(), 0);
}

TEST_F(SmartPointerContrastTest, SharedPtrOverheadVsUnique)
{
    long shared_count = 0;
    
    // TODO: Create shared_ptr
    // YOUR CODE HERE
    
    // TODO: Get use_count
    // shared_count = ???
    
    // TODO: Create unique_ptr
    // YOUR CODE HERE
    
    // Question: What's the size difference?
    // shared_ptr: pointer + control block pointer
    // unique_ptr: just pointer (usually)
    
    EXPECT_EQ(shared_count, 1);
}

TEST_F(SmartPointerContrastTest, AliasingUniqueVsShared)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };
    
    // TODO: Create unique_ptr to Container
    // YOUR CODE HERE
    
    // TODO: Get raw pointer to member
    // YOUR CODE HERE
    
    // TODO: Create shared_ptr to Container
    // YOUR CODE HERE
    
    // TODO: Create aliased shared_ptr to member
    // YOUR CODE HERE
    
    long alias_count = 0;
    // TODO: Get use_count of aliased_member
    // alias_count = ???
    
    // Question: Can unique_ptr do aliasing? Why or why not?
    EXPECT_EQ(alias_count, 2);
}
