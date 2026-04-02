#include "move_instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <utility>
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
    auto ptr1 = std::make_unique<MoveTracked>("Unique");
    auto ptr2 = std::move(ptr1);

    // Q: Why is unique_ptr move-only and what would break if it were copyable?
    // A:
    // R:

    // Q: After the move, what does ptr1 contain and what EventLog entries confirm the MoveTracked object was not
    // destroyed? A: R:

    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");

    EXPECT_EQ(ctor_count, 1);
}

TEST_F(MoveOnlyTypesTest, MoveOnlyResourceClass)
{
    Resource res1("MoveOnly");
    Resource res2(std::move(res1));

    // Q: What special member functions must be deleted or defaulted to make a class move-only?
    // A:
    // R:

    // Q: After the move, what operations on res1 are well-defined and what would be undefined behavior?
    // A:
    // R:

    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");

    EXPECT_EQ(move_ctor, 1);
}

TEST_F(MoveOnlyTypesTest, MoveOnlyInContainer)
{
    std::vector<Resource> vec;
    Resource resource("InVector");

    vec.push_back(std::move(resource));

    // Q: What prevents push_back(resource) without std::move from compiling?
    // A:
    // R:

    // Q: When the vector resizes, what operation does it use to relocate move-only elements?
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

    Resource result = create_resource();

    // Q: What EventLog entries show how many constructor calls occurred? What optimization eliminated move operations?
    // A:
    // R:

    // Q: What conditions must be met for RVO to apply?
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

    Resource result = create_conditional(true);

    // Q: What EventLog entries show how many constructors and moves occurred? Can RVO apply with multiple return paths?
    // A:
    // R:

    // Q: What happens to the Resource object in the non-taken branch?
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

    Resource result = wrong_return_move();

    // Q: What EventLog entries show whether RVO was applied? How many move operations occurred?
    // A:
    // R:

    // Q: Why does `return std::move(local);` prevent RVO and what performance cost does this introduce?
    // A:
    // R:

    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");

    EXPECT_EQ(move_ctor, 1);
}

struct MoveOnlyWrapper
{
    explicit MoveOnlyWrapper(const std::string& name) : resource_(name)
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
    MoveOnlyWrapper wrapper1("Wrapper1");
    MoveOnlyWrapper wrapper2(std::move(wrapper1));

    // Q: What does = default generate for the move constructor and what EventLog entries confirm the member-wise move?
    // A:
    // R:

    // Q: After the move, what state is wrapper1.resource_ in?
    // A:
    // R:

    auto events = EventLog::instance().events();
    size_t move_ctor = EventLog::instance().count_events("move_ctor");

    EXPECT_GE(move_ctor, 1);
}

TEST_F(MoveOnlyTypesTest, UniquePtrOwnershipTransfer)
{
    auto ptr1 = std::make_unique<MoveTracked>("UniqueOwner");
    auto ptr2 = std::move(ptr1);

    // Q: What is the value of ptr1 after the transfer and what happens if you dereference it?
    // A:
    // R:

    // Q: What EventLog entries confirm the MoveTracked object was not destroyed during the transfer?
    // A:
    // R:

    bool ptr1_is_null = (ptr1 == nullptr);

    EXPECT_TRUE(ptr1_is_null);
}

std::unique_ptr<MoveTracked> factory_pattern(const std::string& name)
{
    return std::make_unique<MoveTracked>(name);
}

TEST_F(MoveOnlyTypesTest, FactoryWithUniquePtr)
{
    EventLog::instance().clear();

    auto ptr = factory_pattern("Factory");

    // Q: What EventLog entries show how many moves occurred? Why is returning unique_ptr efficient?
    // A:
    // R:

    // Q: Why is the factory pattern common with unique_ptr and what ownership semantics does it establish?
    // A:
    // R:

    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");

    EXPECT_GE(ctor_count, 0);
}

TEST_F(MoveOnlyTypesTest, MoveOnlyInVector)
{
    std::vector<std::unique_ptr<MoveTracked>> vec;

    vec.push_back(std::make_unique<MoveTracked>("First"));
    vec.push_back(std::make_unique<MoveTracked>("Second"));

    // Q: What prevents vector<unique_ptr<T>> from being copyable?
    // A:
    // R:

    // Q: When you move a vector<unique_ptr<T>>, what happens to the unique_ptrs and their managed objects?
    // A:
    // R:

    size_t vec_size = vec.size();
    EXPECT_GE(vec_size, 0);
}

TEST_F(MoveOnlyTypesTest, TemporaryMoveOnly)
{
    std::vector<Resource> vec;

    vec.push_back(Resource("Temporary"));

    // Q: Why is std::move not needed when pushing back a temporary?
    // A:
    // R:

    // Q: What value category is Resource("Temporary") and how does this enable move operations?
    // A:
    // R:

    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");

    EXPECT_GE(ctor_count, 1);
}

class MoveCounter
{
public:
    MoveCounter() : move_count_(0)
    {
    }

    MoveCounter(const MoveCounter&) = delete;
    MoveCounter& operator=(const MoveCounter&) = delete;

    MoveCounter(MoveCounter&& other) noexcept : move_count_(other.move_count_ + 1)
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
    MoveCounter c2(std::move(counter));
    MoveCounter c3(std::move(c2));

    // Q: What is c3.move_count() and how does the MoveCounter implementation track the number of moves?
    // A:
    // R:

    // Q: After multiple moves, which object holds the final state and what happened to the previous objects?
    // A:
    // R:

    EXPECT_TRUE(true);
}
