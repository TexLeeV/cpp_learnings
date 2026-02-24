#include "move_instrumentation.h"
#include <gtest/gtest.h>
#include <utility>

class MoveAssignmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(MoveAssignmentTest, BasicMoveAssignment)
{
    // TODO: Create obj1 with name "Source"
    // YOUR CODE HERE
    
    // TODO: Create obj2 with name "Destination"
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Move assign obj1 to obj2 using std::move
    // YOUR CODE HERE
    
    // Q: Which object's move assignment operator was called?
    // A: 
    // R: 
    
    // Q: After the move assignment, what happens to obj2's original state?
    // A: 
    // R: 
    
    // Q: What is the key difference between move constructor and move assignment?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_assign_count = EventLog::instance().count_events("move_assign");
    
    EXPECT_EQ(move_assign_count, 1);
}

TEST_F(MoveAssignmentTest, SelfMoveAssignment)
{
    // TODO: Create obj with name "SelfMove"
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Move assign obj to itself: obj = std::move(obj)
    // YOUR CODE HERE
    
    // Q: What should happen when an object is move-assigned to itself?
    // A: 
    // R: 
    
    // Q: Should the move assignment operator check for self-assignment?
    // A: 
    // R: 
    
    // Q: What would happen if there's no self-assignment check?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_assign_count = EventLog::instance().count_events("move_assign");
    
    EXPECT_EQ(move_assign_count, 1);
}

class RuleOfFive
{
public:
    explicit RuleOfFive(const std::string& name)
    : tracked_(name)
    {
    }
    
    // TODO: Implement copy constructor
    RuleOfFive(const RuleOfFive& other)
    : tracked_(other.tracked_)
    {
    }
    
    // TODO: Implement move constructor
    RuleOfFive(RuleOfFive&& other) noexcept
    : tracked_(std::move(other.tracked_))
    {
    }
    
    // TODO: Implement copy assignment operator
    RuleOfFive& operator=(const RuleOfFive& other)
    {
        if (this != &other)
        {
            tracked_ = other.tracked_;
        }
        return *this;
    }
    
    // TODO: Implement move assignment operator
    RuleOfFive& operator=(RuleOfFive&& other) noexcept
    {
        if (this != &other)
        {
            tracked_ = std::move(other.tracked_);
        }
        return *this;
    }
    
    // TODO: Implement destructor
    ~RuleOfFive()
    {
    }
    
    std::string name() const
    {
        return tracked_.name();
    }
    
private:
    MoveTracked tracked_;
};

TEST_F(MoveAssignmentTest, RuleOfFiveComplete)
{
    // TODO: Create obj1 with "First"
    // YOUR CODE HERE
    
    // TODO: Create obj2 by copying obj1
    // YOUR CODE HERE
    
    // TODO: Create obj3 by moving from obj1
    // YOUR CODE HERE
    
    // Q: How many copy constructors were called in total?
    // A: 
    // R: 
    
    // Q: How many move constructors were called in total?
    // A: 
    // R: 
    
    // Q: Why do we need all five special member functions (Rule of Five)?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t copy_ctor = EventLog::instance().count_events("copy_ctor");
    size_t move_ctor = EventLog::instance().count_events("move_ctor");
    
    EXPECT_EQ(copy_ctor, 2);
    EXPECT_EQ(move_ctor, 1);
}

TEST_F(MoveAssignmentTest, MoveAssignmentChain)
{
    // TODO: Create obj1, obj2, obj3 with names "First", "Second", "Third"
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Chain move assignments: obj3 = std::move(obj2) = std::move(obj1)
    // Note: This should NOT compile correctly - assignment doesn't return rvalue ref
    // Instead, do: obj2 = std::move(obj1); obj3 = std::move(obj2);
    // YOUR CODE HERE
    
    // Q: After obj2 = std::move(obj1), what state is obj1 in?
    // A: 
    // R: 
    
    // Q: After obj3 = std::move(obj2), can you still move from obj2 again?
    // A: 
    // R: 
    
    // Q: How many objects are in valid (not moved-from) state?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_assign_count = EventLog::instance().count_events("move_assign");
    
    EXPECT_EQ(move_assign_count, 2);
}

TEST_F(MoveAssignmentTest, MoveFromTemporary)
{
    // TODO: Create obj with name "Target"
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Move assign a temporary to obj: obj = MoveTracked("Temporary")
    // YOUR CODE HERE
    
    // Q: Is std::move needed when assigning from a temporary?
    // A: 
    // R: 
    
    // Q: Why does the compiler automatically treat temporaries as rvalues?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");
    size_t move_assign_count = EventLog::instance().count_events("move_assign");
    
    EXPECT_GE(ctor_count, 1);
    EXPECT_EQ(move_assign_count, 1);
}

TEST_F(MoveAssignmentTest, MoveAssignmentExceptionSafety)
{
    // TODO: Create obj1 and obj2 with names "Safe1" and "Safe2"
    // YOUR CODE HERE
    
    // Q: Why should move assignment be marked noexcept?
    // A: 
    // R: 
    
    // Q: What happens in std::vector::resize if move assignment can throw?
    // A: 
    // R: 
    
    // Q: How does noexcept affect move semantics in standard containers?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

class ResourceWrapper
{
public:
    explicit ResourceWrapper(const std::string& name)
    : resource_(new MoveTracked(name))
    {
    }
    
    ~ResourceWrapper()
    {
        delete resource_;
    }
    
    ResourceWrapper(const ResourceWrapper& other)
    : resource_(new MoveTracked(*other.resource_))
    {
    }
    
    ResourceWrapper(ResourceWrapper&& other) noexcept
    : resource_(other.resource_)
    {
        other.resource_ = nullptr;
    }
    
    ResourceWrapper& operator=(const ResourceWrapper& other)
    {
        if (this != &other)
        {
            delete resource_;
            resource_ = new MoveTracked(*other.resource_);
        }
        return *this;
    }
    
    ResourceWrapper& operator=(ResourceWrapper&& other) noexcept
    {
        if (this != &other)
        {
            delete resource_;
            resource_ = other.resource_;
            other.resource_ = nullptr;
        }
        return *this;
    }
    
    std::string name() const
    {
        return resource_ ? resource_->name() : "null";
    }
    
private:
    MoveTracked* resource_;
};

TEST_F(MoveAssignmentTest, RawPointerMoveSemantics)
{
    // TODO: Create wrapper1 with "Resource1"
    // YOUR CODE HERE
    
    // TODO: Create wrapper2 with "Resource2"
    // YOUR CODE HERE
    
    EventLog::instance().clear();
    
    // TODO: Move assign wrapper1 to wrapper2
    // YOUR CODE HERE
    
    // Q: After the move assignment, what happened to wrapper2's original resource?
    // A: 
    // R: 
    
    // Q: What happened to wrapper1's resource pointer?
    // A: 
    // R: 
    
    // Q: Why is it critical to set other.resource_ = nullptr in the move constructor?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t dtor_count = EventLog::instance().count_events("::dtor");
    
    EXPECT_EQ(dtor_count, 1);
}

TEST_F(MoveAssignmentTest, MovedFromStateAccess)
{
    // TODO: Create obj1 with name "Original"
    // YOUR CODE HERE
    
    // TODO: Create obj2 by moving from obj1
    // YOUR CODE HERE
    
    // Q: After moving from obj1, can you safely call obj1.name()?
    // A: 
    // R: 
    
    // Q: What guarantees does the C++ standard provide about moved-from objects?
    // A: 
    // R: 
    
    // Q: Can you reassign a value to obj1 after it's been moved from?
    // A: 
    // R: 
    
    // TODO: Reassign obj1 with a new value
    // YOUR CODE HERE
    
    bool obj1_valid = false;
    // TODO: Check if obj1 is no longer moved-from
    // obj1_valid = ???
    
    EXPECT_TRUE(obj1_valid);
}
