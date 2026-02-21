#include "instrumentation.h"
#include <gtest/gtest.h>
#include <utility>
#include <vector>

class RvalueReferencesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(RvalueReferencesTest, LvalueVsRvalue)
{
    // TODO: Create an lvalue MoveTracked object named obj with "Lvalue"
    // YOUR CODE HERE
    
    // TODO: Create a vector and push_back obj
    // YOUR CODE HERE
    
    // Q: When you called push_back(obj), which constructor was invoked—copy or move?
    // A: 
    // R: 
    
    // Q: Why was the copy constructor chosen?
    // A: 
    // R: 
    
    // TODO: Now push_back an rvalue using MoveTracked("Rvalue")
    // YOUR CODE HERE
    
    // Q: When you called push_back(MoveTracked("Rvalue")), which constructor was invoked?
    // A: 
    // R: 
    
    // Q: What is the difference between obj and MoveTracked("Rvalue") in terms of value category?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t copy_ctor_count = EventLog::instance().count_events("copy_ctor");
    size_t move_ctor_count = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(copy_ctor_count, 1);
    EXPECT_GE(move_ctor_count, 1);
}

TEST_F(RvalueReferencesTest, StdMoveBasics)
{
    // TODO: Create obj1 with name "Original"
    // YOUR CODE HERE
    
    // TODO: Create obj2 by moving from obj1 using std::move
    // YOUR CODE HERE
    
    // Q: What does std::move(obj1) actually do to obj1?
    // A: 
    // R: 
    
    // Q: After the move, is obj1 still in a valid state?
    // A: 
    // R: 
    
    // Q: What observable signal in the event log confirms a move occurred?
    // A: 
    // R: 
    
    // TODO: Check if obj1 is moved-from
    bool obj1_moved = false;
    // obj1_moved = ???
    
    auto events = EventLog::instance().events();
    size_t move_ctor_count = EventLog::instance().count_events("move_ctor");
    
    EXPECT_TRUE(obj1_moved);
    EXPECT_EQ(move_ctor_count, 1);
}

TEST_F(RvalueReferencesTest, TemporaryLifetime)
{
    {
        // TODO: Create an lvalue reference that binds to a temporary
        // Hint: This is illegal - const MoveTracked& ref = MoveTracked("Temp"); is legal
        // YOUR CODE HERE
        
        // Q: Can you bind a non-const lvalue reference to a temporary?
        // A: 
        // R: 
        
        // Q: Why does const lvalue reference extend the temporary's lifetime?
        // A: 
        // R: 
        
        // TODO: Create an rvalue reference that binds to a temporary
        // YOUR CODE HERE
        
        // Q: Does the rvalue reference extend the temporary's lifetime?
        // A: 
        // R: 
    }
    
    // Q: At what point in the code was the temporary destroyed?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor");
    size_t dtor_count = EventLog::instance().count_events("::dtor");
    
    EXPECT_GE(ctor_count, 1);
    EXPECT_GE(dtor_count, 1);
}

TEST_F(RvalueReferencesTest, MoveConstructorElision)
{
    EventLog::instance().clear();
    
    // TODO: Create a vector and reserve space for 2 elements
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Push back MoveTracked("First")
    // YOUR CODE HERE
    
    // Q: How many MoveTracked constructors were called for this push_back?
    // A: 
    // R: 
    
    // Q: Was there a move constructor call, or was the object constructed in-place?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");
    size_t move_ctor_count = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(ctor_count, 1);
    EXPECT_EQ(move_ctor_count, 0);
}

TEST_F(RvalueReferencesTest, RvalueReferenceFunctionOverloading)
{
    auto process_lvalue = [](MoveTracked& obj)
    {
        EventLog::instance().record("process_lvalue called");
    };
    
    auto process_rvalue = [](MoveTracked&& obj)
    {
        EventLog::instance().record("process_rvalue called");
    };
    
    // TODO: Create an lvalue object
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Call process_lvalue with the lvalue
    // YOUR CODE HERE
    
    // TODO: Call process_rvalue with std::move(obj)
    // YOUR CODE HERE
    
    // Q: Why does std::move(obj) bind to the rvalue reference overload?
    // A: 
    // R: 
    
    // Q: After calling process_rvalue(std::move(obj)), is obj still valid?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t lvalue_calls = EventLog::instance().count_events("process_lvalue");
    size_t rvalue_calls = EventLog::instance().count_events("process_rvalue");
    
    EXPECT_EQ(lvalue_calls, 1);
    EXPECT_EQ(rvalue_calls, 1);
}

TEST_F(RvalueReferencesTest, MoveIntoContainer)
{
    std::vector<MoveTracked> vec;
    
    // TODO: Create obj with name "ToMove"
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Move obj into vector
    // YOUR CODE HERE
    
    // Q: How many copy constructors were called?
    // A: 
    // R: 
    
    // Q: How many move constructors were called?
    // A: 
    // R: 
    
    // Q: What is the performance benefit of moving instead of copying?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t copy_count = EventLog::instance().count_events("copy_ctor");
    size_t move_count = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(copy_count, 0);
    EXPECT_GE(move_count, 1);
}

TEST_F(RvalueReferencesTest, ValueCategoryInExpression)
{
    // TODO: Create obj with name "Value"
    // YOUR CODE HERE
    
    // Q: What is the value category of obj?
    // A: 
    // R: 
    
    // Q: What is the value category of std::move(obj)?
    // A: 
    // R: 
    
    // Q: What is the value category of MoveTracked("Temp")?
    // A: 
    // R: 
    
    // Q: Does calling std::move on obj change obj's type?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}
