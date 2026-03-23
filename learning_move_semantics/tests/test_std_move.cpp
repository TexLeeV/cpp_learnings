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
    MoveTracked obj("ToCast");
    
    // Q: What does std::move do to obj and what type does it return?
    // A: 
    // R: 
    
    MoveTracked obj2(std::move(obj));
    
    // Q: What EventLog entry confirms the move constructor was invoked?
    // A: 
    // R: 
    
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
    MoveTracked obj("Test");
    
    // Q: What is the key difference between std::move and std::forward in terms of what they preserve?
    // A: 
    // R: 
    
    // Q: In what context would using std::move instead of std::forward cause incorrect behavior?
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
    MoveTracked obj("ToConsume");
    
    EventLog::instance().clear();
    
    consume_by_value(std::move(obj));
    
    // Q: What EventLog entries show the move operation? How many moves occurred?
    // A: 
    // R: 
    
    // Q: If you called consume_by_rvalue(std::move(obj)) instead, would there be a move constructor call?
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
        return local;
    };
    
    EventLog::instance().clear();
    
    MoveTracked result = create_object();
    
    // Q: What EventLog entries show how many copy/move operations occurred? What optimization eliminated them?
    // A: 
    // R: 
    
    // Q: If you changed the return to `return std::move(local);`, what would happen to RVO?
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
    
    vec.push_back(std::move(obj));
    
    // Q: What EventLog entries show which constructor was called? Why wasn't the move constructor invoked?
    // A: 
    // R: 
    
    // Q: What type does std::move(obj) return when obj is const?
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
    MoveTracked obj("Multi");
    auto&& ref = std::move(std::move(std::move(obj)));
    
    // Q: Does calling std::move multiple times have any additional effect beyond the first call?
    // A: 
    // R: 
    
    // Q: What is the type of ref after the nested std::move calls?
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
    MoveTracked obj("Forward");
    
    EventLog::instance().clear();
    
    wrapper_move(obj);
    
    // Q: If wrapper_move uses std::move(param), what EventLog entries would show for an lvalue argument?
    // A: 
    // R: 
    
    // Q: If wrapper_move uses std::forward<T>(param), what EventLog entries would show for an lvalue argument?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    
    EXPECT_GE(EventLog::instance().count_events("::ctor"), 0);
}

TEST_F(StdMoveTest, MoveSemanticOptimization)
{
    std::vector<MoveTracked> vec1;
    MoveTracked a("A"), b("B"), c("C");
    vec1.push_back(std::move(a));
    vec1.push_back(std::move(b));
    vec1.push_back(std::move(c));
    
    std::vector<MoveTracked> vec2;
    MoveTracked x("X"), y("Y"), z("Z");
    vec2.push_back(x);
    vec2.push_back(y);
    vec2.push_back(z);
    
    // Q: What EventLog entries distinguish the operations on vec1 versus vec2?
    // A: 
    // R: 
    
    // Q: For a type managing heap-allocated memory, what resource operations does move avoid compared to copy?
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
