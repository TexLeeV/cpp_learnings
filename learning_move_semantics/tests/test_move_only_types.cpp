#include "instrumentation.h"
#include <gtest/gtest.h>
#include <utility>
#include <memory>
#include <vector>

class MoveOnlyTypesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(MoveOnlyTypesTest, UniquePtrBasics)
{
    // TODO: Create unique_ptr to MoveTracked with name "Unique"
    // YOUR CODE HERE
    
    // Q: Can you copy a unique_ptr?
    // A: 
    // R: 
    
    // Q: Why is unique_ptr a move-only type?
    // A: 
    // R: 
    
    // TODO: Move the unique_ptr to ptr2
    // YOUR CODE HERE
    
    // Q: After moving, what does ptr1 contain?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");
    
    EXPECT_EQ(ctor_count, 1);
}

TEST_F(MoveOnlyTypesTest, MoveOnlyResourceClass)
{
    // TODO: Create Resource object with name "MoveOnly"
    // YOUR CODE HERE
    
    // Q: How do you make a class move-only?
    // A: 
    // R: 
    
    // TODO: Try to create res2 by moving from res1
    // YOUR CODE HERE
    
    // Q: After moving, is res1 still valid?
    // A: 
    // R: 
    
    // Q: Can you call res1.is_valid()?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(move_ctor, 1);
}

TEST_F(MoveOnlyTypesTest, MoveOnlyInContainer)
{
    std::vector<Resource> vec;
    
    // TODO: Create resource with name "InVector"
    // YOUR CODE HERE
    
    // TODO: Move resource into vector
    // YOUR CODE HERE
    
    // Q: Can you push_back a move-only type without std::move?
    // A: 
    // R: 
    
    // Q: What happens if the vector needs to resize?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_GE(move_ctor, 1);
}

Resource create_resource()
{
    Resource res("Created");
    
    // Q: Do you need std::move when returning a local variable?
    // A: 
    // R: 
    
    // Q: What is automatic move from local variables?
    // A: 
    // R: 
    
    return res;
}

TEST_F(MoveOnlyTypesTest, ReturnValueOptimization)
{
    EventLog::instance().clear();
    
    // TODO: Call create_resource and capture result
    // YOUR CODE HERE
    
    // Q: Was there a move constructor call?
    // A: 
    // R: 
    
    // Q: What is copy elision / RVO?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(ctor_count, 1);
    EXPECT_EQ(move_ctor, 0);
}

Resource create_conditional(bool condition)
{
    if (condition)
    {
        Resource res1("Branch1");
        return res1;
    }
    else
    {
        Resource res2("Branch2");
        return res2;
    }
    
    // Q: Can RVO apply when there are multiple return paths?
    // A: 
    // R: 
    
    // Q: What happens to the non-returned object?
    // A: 
    // R: 
}

TEST_F(MoveOnlyTypesTest, ConditionalReturn)
{
    EventLog::instance().clear();
    
    // TODO: Call create_conditional(true)
    // YOUR CODE HERE
    
    // Q: How many Resource constructors were called?
    // A: 
    // R: 
    
    // Q: How many moves occurred?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");
    
    EXPECT_EQ(ctor_count, 1);
}

Resource wrong_return_move()
{
    Resource res("WrongMove");
    
    // Q: Should you use std::move(res) when returning?
    // A: 
    // R: 
    
    // Q: How does std::move affect RVO?
    // A: 
    // R: 
    
    return std::move(res);
}

TEST_F(MoveOnlyTypesTest, ReturnMoveAntiPattern)
{
    EventLog::instance().clear();
    
    // TODO: Call wrong_return_move and capture result
    // YOUR CODE HERE
    
    // Q: Was RVO applied when using std::move in return?
    // A: 
    // R: 
    
    // Q: Why is return std::move(local) an anti-pattern?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(move_ctor, 1);
}

struct MoveOnlyWrapper
{
    explicit MoveOnlyWrapper(const std::string& name)
    : resource_(name)
    {
    }
    
    MoveOnlyWrapper(const MoveOnlyWrapper&) = delete;
    MoveOnlyWrapper& operator=(const MoveOnlyWrapper&) = delete;
    
    MoveOnlyWrapper(MoveOnlyWrapper&&) = default;
    MoveOnlyWrapper& operator=(MoveOnlyWrapper&&) = default;
    
    Resource resource_;
};

TEST_F(MoveOnlyTypesTest, DefaultedMoveOperations)
{
    // TODO: Create wrapper1 with "Wrapper1"
    // YOUR CODE HERE
    
    // TODO: Move wrapper1 to wrapper2
    // YOUR CODE HERE
    
    // Q: What does = default do for move constructor?
    // A: 
    // R: 
    
    // Q: What happens to wrapper1.resource_ after the move?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_GE(move_ctor, 1);
}

TEST_F(MoveOnlyTypesTest, UniquePtrOwnershipTransfer)
{
    auto ptr1 = std::make_unique<MoveTracked>("UniqueOwner");
    
    // TODO: Transfer ownership to ptr2
    // YOUR CODE HERE
    
    // Q: Can you access ptr1 after the transfer?
    // A: 
    // R: 
    
    // Q: What happens if you dereference ptr1 after transfer?
    // A: 
    // R: 
    
    bool ptr1_is_null = false;
    // TODO: Check if ptr1 is nullptr
    // ptr1_is_null = ???
    
    EXPECT_TRUE(ptr1_is_null);
}

std::unique_ptr<MoveTracked> factory_pattern(const std::string& name)
{
    // TODO: Return a unique_ptr created with make_unique
    // YOUR CODE HERE
    
    // Q: Why is factory pattern common with unique_ptr?
    // A: 
    // R: 
    
    return nullptr;
}

TEST_F(MoveOnlyTypesTest, FactoryWithUniquePtr)
{
    EventLog::instance().clear();
    
    // TODO: Call factory_pattern and capture result
    // YOUR CODE HERE
    
    // Q: How many moves occurred in the return?
    // A: 
    // R: 
    
    // Q: Does returning unique_ptr guarantee no copies?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");
    
    EXPECT_GE(ctor_count, 0);
}

TEST_F(MoveOnlyTypesTest, MoveOnlyInVector)
{
    std::vector<std::unique_ptr<MoveTracked>> vec;
    
    // TODO: Create unique_ptr and move into vector
    // YOUR CODE HERE
    
    // TODO: Move another unique_ptr into vector
    // YOUR CODE HERE
    
    // Q: Can vector<unique_ptr> be copied?
    // A: 
    // R: 
    
    // Q: Can vector<unique_ptr> be moved?
    // A: 
    // R: 
    
    size_t vec_size = vec.size();
    EXPECT_GE(vec_size, 0);
}

TEST_F(MoveOnlyTypesTest, TemporaryMoveOnly)
{
    std::vector<Resource> vec;
    
    // TODO: Push back a temporary Resource
    // YOUR CODE HERE
    
    // Q: Is std::move needed for temporaries?
    // A: 
    // R: 
    
    // Q: How does the compiler treat temporary move-only objects?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");
    
    EXPECT_GE(ctor_count, 1);
}

class MoveCounter
{
public:
    MoveCounter()
    : move_count_(0)
    {
    }
    
    MoveCounter(const MoveCounter&) = delete;
    MoveCounter& operator=(const MoveCounter&) = delete;
    
    MoveCounter(MoveCounter&& other) noexcept
    : move_count_(other.move_count_ + 1)
    {
    }
    
    MoveCounter& operator=(MoveCounter&& other) noexcept
    {
        move_count_ = other.move_count_ + 1;
        return *this;
    }
    
    int move_count() const
    {
        return move_count_;
    }
    
private:
    int move_count_;
};

TEST_F(MoveOnlyTypesTest, TrackingMoveOperations)
{
    MoveCounter counter;
    
    // TODO: Move counter multiple times
    // YOUR CODE HERE
    
    // Q: How many times was the object moved?
    // A: 
    // R: 
    
    // Q: What's the relationship between move count and object identity?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}
