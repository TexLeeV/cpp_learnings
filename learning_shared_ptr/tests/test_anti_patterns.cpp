#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class AntiPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(AntiPatternsTest, SharedPtrAsGlobal)
{
    // ANTI-PATTERN: Using shared_ptr as a global
    // Problem: Unclear ownership, initialization order issues
    
    // TODO: Create a shared_ptr (simulating global)
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    // Question: Who owns this? When is it destroyed?
    EXPECT_EQ(use_count, 1);
}

TEST_F(AntiPatternsTest, PassingByValueUnnecessarily)
{
    auto process = [](std::shared_ptr<Tracked> item)
    {
        // ANTI-PATTERN: Passing by value when not needed
        // Problem: Unnecessary reference count increment/decrement
    };
    
    // TODO: Create shared_ptr
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Call process (passes by value)
    // YOUR CODE HERE
    
    auto events = EventLog::instance().events();
    
    // Question: How many ref count operations occurred?
    EXPECT_GT(events.size(), 0);
}

TEST_F(AntiPatternsTest, CreatingTwoSharedPtrsFromSameRaw)
{
    // ANTI-PATTERN: Creating multiple shared_ptrs from same raw pointer
    // Problem: Multiple control blocks, double deletion
    
    Tracked* raw = new Tracked("Dangerous");
    
    // TODO: Create p1 from raw
    // YOUR CODE HERE
    
    // WARNING: DO NOT DO THIS - creates second control block
    // std::shared_ptr<Tracked> p2(raw);  // DOUBLE DELETE!
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(AntiPatternsTest, HoldingSharedPtrToThis)
{
    // ANTI-PATTERN: Storing shared_ptr to this without enable_shared_from_this
    // Problem: Circular reference, memory leak
    
    class BadSelfReference
    {
    public:
        explicit BadSelfReference(const std::string& name)
        : tracked_(name)
        {
        }
        
        void set_self(std::shared_ptr<BadSelfReference> self)
        {
            self_ = self;  // Creates circular reference!
        }
        
    private:
        Tracked tracked_;
        std::shared_ptr<BadSelfReference> self_;
    };
    
    {
        // TODO: Create obj
        // YOUR CODE HERE
        
        // TODO: Set self reference (BAD!)
        // YOUR CODE HERE
        
        long use_count = 0;
        // TODO: Get use_count
        // use_count = ???
        
        EXPECT_EQ(use_count, 2);
    }
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    EXPECT_EQ(dtor_count, 0);  // Memory leak!
}

TEST_F(AntiPatternsTest, UsingSharedPtrForUniqueOwnership)
{
    // ANTI-PATTERN: Using shared_ptr when unique_ptr would suffice
    // Problem: Unnecessary overhead, unclear ownership semantics
    
    // TODO: Create shared_ptr (but ownership is actually unique)
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    // Question: If use_count is always 1, why use shared_ptr?
    EXPECT_EQ(use_count, 1);
}

TEST_F(AntiPatternsTest, NotUsingMakeShared)
{
    // ANTI-PATTERN: Using new instead of make_shared
    // Problem: Two allocations instead of one, exception safety issues
    
    // TODO: Create p1 using new (ANTI-PATTERN)
    // YOUR CODE HERE
    
    // TODO: Create p2 using make_shared (BETTER)
    // YOUR CODE HERE
    
    // Both work, but make_shared is more efficient
    EXPECT_NE(nullptr, nullptr);
}

TEST_F(AntiPatternsTest, StoringWeakPtrWhenSharedPtrNeeded)
{
    // ANTI-PATTERN: Using weak_ptr when shared ownership is actually needed
    // Problem: Object can be destroyed unexpectedly
    
    // TODO: Create weak_ptr
    // YOUR CODE HERE
    
    {
        // TODO: Create shared_ptr and assign to weak
        // YOUR CODE HERE
    }
    
    bool expired = false;
    // TODO: Check if expired
    // expired = ???
    
    // Question: Did you want the object to be destroyed?
    EXPECT_TRUE(expired);
}

TEST_F(AntiPatternsTest, IgnoringWeakPtrLockResult)
{
    // ANTI-PATTERN: Not checking if weak_ptr::lock() succeeded
    // Problem: Dereferencing nullptr
    
    // TODO: Create weak_ptr
    // YOUR CODE HERE
    
    {
        // TODO: Create shared_ptr and assign to weak
        // YOUR CODE HERE
    }
    
    // TODO: Try to lock
    // YOUR CODE HERE
    
    bool is_null = false;
    // TODO: Check if locked is nullptr
    // is_null = ???
    
    // Always check lock result before using!
    EXPECT_TRUE(is_null);
}

TEST_F(AntiPatternsTest, CyclicReferencesWithoutWeakPtr)
{
    // ANTI-PATTERN: Creating cycles without breaking them with weak_ptr
    // Problem: Memory leak
    
    class Node
    {
    public:
        explicit Node(const std::string& name)
        : tracked_(name)
        {
        }
        
        void set_next(std::shared_ptr<Node> next)
        {
            next_ = next;
        }
        
    private:
        Tracked tracked_;
        std::shared_ptr<Node> next_;  // Should be weak_ptr!
    };
    
    {
        // TODO: Create two nodes and create cycle
        // YOUR CODE HERE
    }
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    EXPECT_EQ(dtor_count, 0);  // Memory leak!
}

TEST_F(AntiPatternsTest, UsingGetToCreateNewSharedPtr)
{
    // ANTI-PATTERN: Creating new shared_ptr from get()
    // Problem: Multiple control blocks, double deletion
    
    // TODO: Create original
    // YOUR CODE HERE
    
    Tracked* raw = nullptr;
    // TODO: Get raw pointer
    // raw = ???
    
    // WARNING: DO NOT DO THIS
    // std::shared_ptr<Tracked> another(raw);  // DOUBLE DELETE!
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(AntiPatternsTest, HoldingSharedPtrInUnstableContainer)
{
    // ANTI-PATTERN: Storing shared_ptr when weak_ptr would be safer
    // Problem: Prevents cleanup, unclear ownership
    
    std::vector<std::shared_ptr<Tracked>> cache;
    
    {
        // TODO: Create resource
        // YOUR CODE HERE
        
        // TODO: Store in cache
        // YOUR CODE HERE
    }
    
    // Resource is still alive because cache holds shared_ptr
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    EXPECT_EQ(dtor_count, 0);
}
