#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <functional>
#include <vector>

class ScopeLifetimePatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(ScopeLifetimePatternsTest, WeakPtrLockBasic)
{
    // TODO: Create weak_ptr
    // YOUR CODE HERE
    
    bool expired_before = false;
    // TODO: Check if expired
    // expired_before = ???
    
    {
        // TODO: Create shared_ptr and assign to weak
        // YOUR CODE HERE
        
        bool expired_during = false;
        // TODO: Check if expired during
        // expired_during = ???
        
        // TODO: Lock weak_ptr
        // YOUR CODE HERE
        
        long use_count = 0;
        // TODO: Get use_count of locked ptr
        // use_count = ???
        
        EXPECT_FALSE(expired_before);
        EXPECT_FALSE(expired_during);
        EXPECT_EQ(use_count, 2);
    }
    
    bool expired_after = false;
    // TODO: Check if expired after
    // expired_after = ???
    
    // TODO: Try to lock after shared is destroyed
    // YOUR CODE HERE
    
    bool locked_after_null = false;
    // TODO: Check if locked_after is nullptr
    // locked_after_null = ???
    
    EXPECT_TRUE(expired_after);
    EXPECT_TRUE(locked_after_null);
}

TEST_F(ScopeLifetimePatternsTest, WeakPtrLockMultipleTimes)
{
    // TODO: Create shared_ptr and weak_ptr
    // YOUR CODE HERE
    
    // TODO: Lock weak_ptr three times
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get use_count of shared
    // use_count = ???
    
    EXPECT_EQ(use_count, 4);
}

TEST_F(ScopeLifetimePatternsTest, LambdaCaptureSharedPtr)
{
    long captured_use_count = 0;
    
    // TODO: Declare callback function
    // YOUR CODE HERE
    
    {
        // TODO: Create shared_ptr
        // YOUR CODE HERE
        
        // TODO: Create lambda that captures shared by value
        // YOUR CODE HERE
        
        long use_count_before_call = 0;
        // TODO: Get use_count before calling callback
        // use_count_before_call = ???
        
        EXPECT_EQ(use_count_before_call, 2);
    }
    
    // TODO: Call callback after shared goes out of scope
    // YOUR CODE HERE
    
    EXPECT_EQ(captured_use_count, 1);
}

TEST_F(ScopeLifetimePatternsTest, LambdaCaptureWeakPtr)
{
    bool was_valid = false;
    
    // TODO: Declare callback function
    // YOUR CODE HERE
    
    {
        // TODO: Create shared_ptr and weak_ptr
        // YOUR CODE HERE
        
        // TODO: Create lambda that captures weak by value
        // YOUR CODE HERE
        
        long use_count = 0;
        // TODO: Get use_count of shared
        // use_count = ???
        
        EXPECT_EQ(use_count, 1);
    }
    
    // TODO: Call callback after shared is destroyed
    // YOUR CODE HERE
    
    EXPECT_FALSE(was_valid);
}

TEST_F(ScopeLifetimePatternsTest, LambdaCaptureWeakPtrStillAlive)
{
    bool was_valid = false;
    long captured_use_count = 0;
    
    // TODO: Create shared_ptr and weak_ptr
    // YOUR CODE HERE
    
    // TODO: Create lambda that captures weak
    // YOUR CODE HERE
    
    // TODO: Call callback while shared is still alive
    // YOUR CODE HERE
    
    EXPECT_TRUE(was_valid);
    EXPECT_EQ(captured_use_count, 2);
}

class CallbackManager
{
public:
    void register_callback(std::function<void()> callback)
    {
        callbacks_.push_back(callback);
    }
    
    void invoke_all()
    {
        for (auto& callback : callbacks_)
        {
            callback();
        }
    }
    
private:
    std::vector<std::function<void()>> callbacks_;
};

TEST_F(ScopeLifetimePatternsTest, CallbackWithSharedPtrLifetimeExtension)
{
    CallbackManager manager;
    
    {
        // TODO: Create shared_ptr
        // YOUR CODE HERE
        
        // TODO: Register callback that captures shared
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
    
    EXPECT_EQ(dtor_count, 0);
}

TEST_F(ScopeLifetimePatternsTest, CallbackWithWeakPtrNoLifetimeExtension)
{
    CallbackManager manager;
    
    {
        // TODO: Create shared_ptr and weak_ptr
        // YOUR CODE HERE
        
        // TODO: Register callback that captures weak
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
    
    EXPECT_EQ(dtor_count, 1);
}

TEST_F(ScopeLifetimePatternsTest, NestedLambdaCapture)
{
    long outer_use_count = 0;
    long inner_use_count = 0;
    
    // TODO: Create shared_ptr
    // YOUR CODE HERE
    
    // TODO: Create outer lambda that captures shared
    // TODO: Inside outer lambda, create inner lambda that also captures shared
    // YOUR CODE HERE
    
    // TODO: Call outer lambda
    // YOUR CODE HERE
    
    EXPECT_EQ(outer_use_count, 2);
    EXPECT_EQ(inner_use_count, 3);
}

TEST_F(ScopeLifetimePatternsTest, WeakPtrExpiredCheck)
{
    // TODO: Create weak_ptr
    // YOUR CODE HERE
    
    bool expired_initial = false;
    // TODO: Check if expired initially
    // expired_initial = ???
    
    {
        // TODO: Create shared_ptr and assign to weak
        // YOUR CODE HERE
        
        bool expired_alive = false;
        // TODO: Check if expired while alive
        // expired_alive = ???
        
        EXPECT_TRUE(expired_initial);
        EXPECT_FALSE(expired_alive);
    }
    
    bool expired_final = false;
    // TODO: Check if expired after shared is destroyed
    // expired_final = ???
    
    EXPECT_TRUE(expired_final);
}

TEST_F(ScopeLifetimePatternsTest, ConditionalLockInCallback)
{
    int invocation_count = 0;
    
    // TODO: Declare callback function
    // YOUR CODE HERE
    
    {
        // TODO: Create shared_ptr and weak_ptr
        // YOUR CODE HERE
        
        // TODO: Create callback with conditional lock
        // YOUR CODE HERE
        
        // TODO: Call callback while shared is alive
        // YOUR CODE HERE
    }
    
    // TODO: Call callback after shared is destroyed
    // YOUR CODE HERE
    
    EXPECT_EQ(invocation_count, 1);
}

TEST_F(ScopeLifetimePatternsTest, MutableLambdaWithSharedPtr)
{
    // TODO: Create shared_ptr
    // YOUR CODE HERE
    
    long initial_count = 0;
    // TODO: Get initial use_count
    // initial_count = ???
    
    // TODO: Create mutable lambda that captures shared and resets it
    // YOUR CODE HERE
    
    long before_call = 0;
    // TODO: Get use_count before calling callback
    // before_call = ???
    
    // TODO: Call callback
    // YOUR CODE HERE
    
    long after_call = 0;
    // TODO: Get use_count after calling callback
    // after_call = ???
    
    EXPECT_EQ(initial_count, 1);
    EXPECT_EQ(before_call, 2);
    EXPECT_EQ(after_call, 1);
}
