#include "move_instrumentation.h"
#include <gtest/gtest.h>
#include <utility>
#include <vector>

class StdMoveTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(StdMoveTest, StdMoveCast)
{
    // TODO: Create obj with name "ToCast"
    // YOUR CODE HERE
    
    // Q: What does std::move actually do?
    // A: 
    // R: 
    
    // Q: Does std::move perform any operations on the object itself?
    // A: 
    // R: 
    
    // Q: What type does std::move(obj) return?
    // A: 
    // R: 
    
    // TODO: Use std::move(obj) to construct obj2
    // YOUR CODE HERE
    
    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(move_ctor, 1);
}

template<typename T>
void forward_lvalue(T&& param)
{
    // TODO: Forward param to another function using std::forward
    // YOUR CODE HERE
    
    // Q: If param binds to an lvalue, what does std::forward<T>(param) return?
    // A: 
    // R: 
    
    // Q: If param binds to an rvalue, what does std::forward<T>(param) return?
    // A: 
    // R: 
}

template<typename T>
void forward_rvalue(T&& param)
{
    // TODO: Forward param using std::forward<T>
    // YOUR CODE HERE
    
    // Q: Why do we use std::forward in template functions instead of std::move?
    // A: 
    // R: 
}

TEST_F(StdMoveTest, StdMoveVsStdForward)
{
    // TODO: Create obj with name "Test"
    // YOUR CODE HERE
    
    // Q: What is the key difference between std::move and std::forward?
    // A: 
    // R: 
    
    // Q: When should you use std::move vs std::forward?
    // A: 
    // R: 
    
    // Q: Can std::forward preserve lvalue-ness?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

void consume_by_value(MoveTracked obj)
{
    EventLog::instance().record("consume_by_value called");
}

void consume_by_rvalue(MoveTracked&& obj)
{
    EventLog::instance().record("consume_by_rvalue called");
}

TEST_F(StdMoveTest, StdMoveInFunctionCall)
{
    // TODO: Create obj with name "ToConsume"
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Call consume_by_value(std::move(obj))
    // YOUR CODE HERE
    
    // Q: How many moves occurred when passing std::move(obj) by value?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(move_ctor, 1);
}

TEST_F(StdMoveTest, StdMoveInReturn)
{
    auto create_object = []() -> MoveTracked
    {
        MoveTracked local("Local");
        
        // Q: Should you use std::move when returning local?
        // A: 
        // R: 
        
        // Q: What is Return Value Optimization (RVO)?
        // A: 
        // R: 
        
        return local;
    };
    
    EventLog::instance().clear();
    
    // TODO: Call create_object and capture result
    // YOUR CODE HERE
    
    // Q: How many copy/move operations occurred?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t copy_ctor = EventLog::instance().count_events("copy_ctor");
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(copy_ctor, 0);
}

TEST_F(StdMoveTest, StdMoveOnConst)
{
    const MoveTracked obj("Const");
    
    std::vector<MoveTracked> vec;
    
    EventLog::instance().clear();
    
    // TODO: Try to move the const object into vector
    // YOUR CODE HERE
    
    // Q: Can you move from a const object?
    // A: 
    // R: 
    
    // Q: What happens when you call std::move on a const object?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t copy_ctor = EventLog::instance().count_events("copy_ctor");
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_GE(copy_ctor, 1);
    EXPECT_EQ(move_ctor, 0);
}

TEST_F(StdMoveTest, RepeatedStdMove)
{
    // TODO: Create obj with name "Multi"
    // YOUR CODE HERE
    
    // TODO: Call std::move multiple times: auto ref = std::move(std::move(std::move(obj)))
    // YOUR CODE HERE
    
    // Q: Does calling std::move multiple times have any additional effect?
    // A: 
    // R: 
    
    // Q: After multiple std::move calls, what type is ref?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

TEST_F(StdMoveTest, StdMoveWithReferenceCollapse)
{
    MoveTracked obj("Reference");
    
    // Q: What is MoveTracked&& & (rvalue reference to lvalue reference)?
    // A: 
    // R: 
    
    // Q: What is MoveTracked&& && (rvalue reference to rvalue reference)?
    // A: 
    // R: 
    
    // Q: What is MoveTracked& && (lvalue reference to rvalue reference)?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

template<typename T>
void wrapper_move(T&& param)
{
    std::vector<MoveTracked> vec;
    
    // TODO: Move param into vector (incorrect - always moves)
    // vec.push_back(std::move(param));
    
    // TODO: Forward param into vector (correct - preserves value category)
    // vec.push_back(std::forward<T>(param));
}

TEST_F(StdMoveTest, ForwardingAndValueCategory)
{
    // TODO: Create obj with name "Forward"
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Call wrapper_move with obj (lvalue)
    // YOUR CODE HERE
    
    // Q: If wrapper_move uses std::move, what happens to lvalues?
    // A: 
    // R: 
    
    // Q: If wrapper_move uses std::forward, what happens to lvalues?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    
    EXPECT_GE(EventLog::instance().count_events("::ctor"), 0);
}

TEST_F(StdMoveTest, MoveSemanticOptimization)
{
    std::vector<MoveTracked> vec1;
    
    // TODO: Create 3 objects and push_back with std::move
    // YOUR CODE HERE
    
    std::vector<MoveTracked> vec2;
    
    // TODO: Create 3 objects and push_back without std::move
    // YOUR CODE HERE
    
    // Q: What is the difference in copy constructor calls?
    // A: 
    // R: 
    
    // Q: For large objects, what is the performance impact?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t copy_count = EventLog::instance().count_events("copy_ctor");
    size_t move_count = EventLog::instance().count_events("move_ctor");
    
    EXPECT_GE(copy_count, 3);
    EXPECT_GE(move_count, 3);
}

TEST_F(StdMoveTest, StdMoveWithUniquePtrAnalogy)
{
    // Q: How is std::move similar to transferring unique_ptr ownership?
    // A: 
    // R: 
    
    // Q: After moving, why is the moved-from object still valid but unspecified?
    // A: 
    // R: 
    
    // Q: Can you compare a moved-from object for equality?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}
