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
    MoveTracked obj1("Source");
    MoveTracked obj2("Destination");
    
    EventLog::instance().clear();
    
    obj2 = std::move(obj1);
    
    // Q: What EventLog entries confirm the move assignment and what happened to obj2's original resource?
    // A: 
    // R: 
    
    // Q: Move constructor initializes a new object; move assignment replaces an existing one. What resource management must move assignment handle that move constructor does not?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_assign_count = EventLog::instance().count_events("move_assign");
    
    EXPECT_EQ(move_assign_count, 1);
}

TEST_F(MoveAssignmentTest, SelfMoveAssignment)
{
    MoveTracked obj("SelfMove");
    
    EventLog::instance().clear();
    
    obj = std::move(obj);
    
    // Q: What observable behavior results from self-move-assignment? What EventLog entries appear?
    // A: 
    // R: 
    
    // Q: If the move assignment operator lacked a self-assignment check and deleted its resource before stealing from 'other', what would happen when this == &other?
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
    
    RuleOfFive(const RuleOfFive& other)
    : tracked_(other.tracked_)
    {
    }
    
    RuleOfFive(RuleOfFive&& other) noexcept
    : tracked_(std::move(other.tracked_))
    {
    }
    
    RuleOfFive& operator=(const RuleOfFive& other)
    {
        if (this != &other)
        {
            tracked_ = other.tracked_;
        }
        return *this;
    }
    
    RuleOfFive& operator=(RuleOfFive&& other) noexcept
    {
        if (this != &other)
        {
            tracked_ = std::move(other.tracked_);
        }
        return *this;
    }
    
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
    RuleOfFive obj1("First");
    RuleOfFive obj2(obj1);
    RuleOfFive obj3(obj2);
    RuleOfFive obj4(std::move(obj1));
    
    // Q: What EventLog entries confirm the copy and move operations? How many of each occurred?
    // A: 
    // R: 
    
    // Q: If you define a move constructor but not a move assignment operator, what happens when you attempt move assignment?
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
    MoveTracked obj1("First");
    MoveTracked obj2("Second");
    MoveTracked obj3("Third");
    
    EventLog::instance().clear();
    
    obj2 = std::move(obj1);
    obj3 = std::move(obj2);
    
    // Q: After the two move assignments, which objects are in moved-from state and what EventLog entries confirm this?
    // A: 
    // R: 
    
    // Q: If you attempted obj4 = std::move(obj1) after obj1 is already moved-from, what guarantees does the standard provide about this operation?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t move_assign_count = EventLog::instance().count_events("move_assign");
    
    EXPECT_EQ(move_assign_count, 2);
}

TEST_F(MoveAssignmentTest, MoveFromTemporary)
{
    MoveTracked obj("Target");
    
    EventLog::instance().clear();
    
    obj = MoveTracked("Temporary");
    
    // Q: What EventLog entries appear from this assignment? Does std::move appear anywhere in the code?
    // A: 
    // R: 
    
    // Q: What value category is MoveTracked("Temporary") and why does this determine which assignment operator is called?
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
    MoveTracked obj1("Safe1");
    MoveTracked obj2("Safe2");
    
    // Q: Why should move assignment be marked noexcept and what observable consequence occurs in std::vector if it's not?
    // A: 
    // R: 
    
    // Q: If move assignment can throw, what fallback does std::vector use during reallocation and why does this impact performance?
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
    ResourceWrapper wrapper1("Resource1");
    ResourceWrapper wrapper2("Resource2");
    
    EventLog::instance().clear();
    
    wrapper2 = std::move(wrapper1);
    
    // Q: What EventLog entries confirm wrapper2's original resource was destroyed and wrapper1's pointer was nulled?
    // A: 
    // R: 
    
    // Q: If the move assignment operator did not null out other.resource_, what would happen when wrapper1's destructor runs?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t dtor_count = EventLog::instance().count_events("::dtor");
    
    EXPECT_EQ(dtor_count, 1);
}

TEST_F(MoveAssignmentTest, MovedFromStateAccess)
{
    MoveTracked obj1("Original");
    MoveTracked obj2(std::move(obj1));
    
    // Q: What guarantees does the standard provide about calling obj1.name() after the move? What could it return?
    // A: 
    // R: 
    
    // Q: Which operations on moved-from obj1 are well-defined and which would be undefined behavior?
    // A: 
    // R: 
    
    obj1 = MoveTracked("Reassigned");
    
    bool obj1_valid = !obj1.name().empty();
    
    // Q: After reassignment, what EventLog entries confirm obj1 is no longer in a moved-from state?
    // A: 
    // R: 
    
    EXPECT_TRUE(obj1_valid);
}
