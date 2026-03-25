#include "move_instrumentation.h"

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
    MoveTracked obj("Lvalue");
    std::vector<MoveTracked> vec;

    vec.push_back(obj);
    vec.push_back(MoveTracked("Rvalue"));

    // Q: What EventLog entries distinguish the push_back(obj) from push_back(MoveTracked("Rvalue"))?
    // A:
    // R:

    // Q: What property of obj versus MoveTracked("Rvalue") determines which constructor overload is selected?
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
    MoveTracked obj1("Original");
    MoveTracked obj2(std::move(obj1));

    // Q: What does std::move(obj1) do and what state is obj1 in after the move constructor completes?
    // A:
    // R:

    // Q: What EventLog entry confirms a move occurred rather than a copy?
    // A:
    // R:

    bool obj1_moved = obj1.name().empty();

    auto events = EventLog::instance().events();
    size_t move_ctor_count = EventLog::instance().count_events("move_ctor");

    EXPECT_TRUE(obj1_moved);
    EXPECT_EQ(move_ctor_count, 1);
}

TEST_F(RvalueReferencesTest, TemporaryLifetime)
{
    {
        const MoveTracked& ref = MoveTracked("Temp");
        MoveTracked&& rref = MoveTracked("RTemp");

        // Q: What EventLog entries show when each temporary was constructed and when will each be destroyed?
        // A:
        // R:

        // Q: If you removed the const from the lvalue reference, would the code compile?
        // A:
        // R:
    }

    // Q: At this point, what EventLog entries confirm both temporaries were destroyed?
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
    std::vector<MoveTracked> vec;
    vec.reserve(2);

    EventLog::instance().clear();

    vec.emplace_back("First");

    // Q: What EventLog entries appear from this emplace_back? How many MoveTracked objects were constructed?
    // A:
    // R:

    // Q: How does emplace_back differ from push_back in terms of construction location?
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
    auto process_lvalue = [](MoveTracked& obj) { EventLog::instance().record("process_lvalue called"); };

    auto process_rvalue = [](MoveTracked&& obj) { EventLog::instance().record("process_rvalue called"); };

    MoveTracked obj("Overload");

    EventLog::instance().clear();

    process_lvalue(obj);
    process_rvalue(std::move(obj));

    // Q: What determines which overload is selected and what EventLog entries confirm the selections?
    // A:
    // R:

    // Q: After process_rvalue(std::move(obj)), obj is still valid. What operations on obj would be well-defined?
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
    MoveTracked obj("ToMove");

    EventLog::instance().clear();

    vec.push_back(std::move(obj));

    // Q: What EventLog entries confirm zero copies and at least one move?
    // A:
    // R:

    // Q: For a type with expensive copy operations, what resource operations does move avoid?
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
    MoveTracked obj("Value");

    // Q: What are the value categories of: obj, std::move(obj), and MoveTracked("Temp")?
    // A:
    // R:

    // Q: Does std::move change obj's type or just cast it? What is the return type of std::move(obj)?
    // A:
    // R:

    EXPECT_TRUE(true);
}
