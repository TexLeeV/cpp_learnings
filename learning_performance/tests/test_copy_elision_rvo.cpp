// Test Suite: Copy Elision and Return Value Optimization
// Estimated Time: 3 hours
// Difficulty: Moderate
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <vector>
#include <string>

class CopyElisionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Named Return Value Optimization (NRVO) - Easy
// ============================================================================

class Widget
{
public:
    Widget()
    {
        EventLog::instance().record("Widget::ctor");
    }

    Widget(const Widget& other)
    {
        EventLog::instance().record("Widget::copy_ctor");
    }

    Widget(Widget&& other) noexcept
    {
        EventLog::instance().record("Widget::move_ctor");
    }

    ~Widget()
    {
        EventLog::instance().record("Widget::dtor");
    }
};

Widget create_widget_nrvo()
{
    Widget w;
    return w;
}

TEST_F(CopyElisionTest, NamedReturnValueOptimization)
{
    {
        Widget result = create_widget_nrvo();
    }

    // Q: How many Widget objects are constructed? Check EventLog.
    // A:
    // R:

    // Q: NRVO elides the copy/move when returning a named local variable. What observable
    // Q: signal confirms NRVO occurred?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("::copy_ctor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("::move_ctor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}

// ============================================================================
// TEST 2: Return Value Optimization (RVO) with Temporaries - Easy
// ============================================================================

Widget create_widget_rvo()
{
    return Widget();
}

TEST_F(CopyElisionTest, ReturnValueOptimizationTemporary)
{
    {
        Widget result = create_widget_rvo();
    }

    // Q: When returning a temporary (prvalue), is copy elision mandatory or optional
    // Q: in C++17?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("::copy_ctor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("::move_ctor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}

// ============================================================================
// TEST 3: NRVO Blocked by Multiple Return Paths - Moderate
// ============================================================================

Widget create_widget_conditional(bool flag)
{
    Widget w1;
    Widget w2;

    if (flag)
    {
        return w1;
    }
    else
    {
        return w2;
    }
}

TEST_F(CopyElisionTest, NRVOBlockedByMultipleReturns)
{
    {
        Widget result = create_widget_conditional(true);
    }

    // Q: This function has two possible return values (w1 or w2). Can the compiler
    // Q: apply NRVO?
    // A:
    // R:

    // Q: What observable signal in EventLog shows that NRVO was blocked?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 2);
    EXPECT_EQ(EventLog::instance().count_events("::move_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 3);
}

// ============================================================================
// TEST 4: Pass-by-Value with Move Semantics - Moderate
// ============================================================================

class Container
{
public:
    Container()
    {
        EventLog::instance().record("Container::ctor");
    }

    explicit Container(std::vector<int> data)
    : data_(std::move(data))
    {
        EventLog::instance().record("Container::ctor(vector)");
    }

    Container(const Container& other)
    : data_(other.data_)
    {
        EventLog::instance().record("Container::copy_ctor");
    }

    Container(Container&& other) noexcept
    : data_(std::move(other.data_))
    {
        EventLog::instance().record("Container::move_ctor");
    }

    size_t size() const { return data_.size(); }

private:
    std::vector<int> data_;
};

TEST_F(CopyElisionTest, PassByValueWithMove)
{
    std::vector<int> data = {1, 2, 3, 4, 5};

    Container c1(data);

    EventLog::instance().clear();

    Container c2(std::move(data));

    // Q: c1 is constructed from an lvalue (data). How many vector copies occur?
    // A:
    // R:

    // Q: c2 is constructed from an rvalue (std::move(data)). How many vector moves occur?
    // A:
    // R:

    // Q: Pass-by-value with move is a common optimization. When is it preferable to
    // Q: pass-by-const-reference?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Container::ctor(vector)"), 1);
}

// ============================================================================
// TEST 5: TODO - Implement Copy Elision in Factory Pattern - Moderate
// ============================================================================

// TODO: Implement a factory function that:
// TODO: 1. Returns different Widget types based on input
// TODO: 2. Measure whether NRVO applies
// TODO: 3. Compare with a version that uses std::unique_ptr

TEST_F(CopyElisionTest, DISABLED_CopyElisionInFactory)
{
    // TODO: Implement factory with multiple return paths
    // TODO: Measure copy/move operations
    // TODO: Compare with unique_ptr factory

    // Q: Factory functions often have multiple return paths. How does this affect NRVO?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Mandatory Copy Elision in C++17 - Easy
// ============================================================================

class NonMovable
{
public:
    NonMovable()
    {
        EventLog::instance().record("NonMovable::ctor");
    }

    NonMovable(const NonMovable&) = delete;
    NonMovable(NonMovable&&) = delete;

    ~NonMovable()
    {
        EventLog::instance().record("NonMovable::dtor");
    }
};

NonMovable create_nonmovable()
{
    return NonMovable();
}

TEST_F(CopyElisionTest, MandatoryCopyElisionC17)
{
    {
        NonMovable obj = create_nonmovable();
    }

    // Q: NonMovable has deleted copy and move constructors. How can this code compile
    // Q: in C++17?
    // A:
    // R:

    // Q: C++17 mandates copy elision for prvalues. What is the difference between
    // Q: mandatory and optional copy elision?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("NonMovable::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("NonMovable::dtor"), 1);
}

// ============================================================================
// TEST 7: Copy Elision with std::move - Moderate
// ============================================================================

Widget create_with_explicit_move()
{
    Widget w;
    return std::move(w);
}

TEST_F(CopyElisionTest, ExplicitMoveBlocksNRVO)
{
    {
        Widget result = create_with_explicit_move();
    }

    // Q: The function explicitly uses std::move on the return value. Does this enable
    // Q: or block NRVO?
    // A:
    // R:

    // Q: What observable signal shows that std::move blocked NRVO?
    // A:
    // R:

    // Q: When should you use std::move on return values?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("::move_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 2);
}

// ============================================================================
// TEST 8: Copy Elision in Initialization - Easy
// ============================================================================

TEST_F(CopyElisionTest, CopyElisionInInitialization)
{
    {
        Widget w1 = Widget();
        Widget w2{Widget()};
    }

    // Q: How many Widget constructors are called for w1 and w2 combined?
    // A:
    // R:

    // Q: C++17 guaranteed copy elision applies to both initializations. What is
    // Q: the observable difference from C++11/14?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 2);
    EXPECT_EQ(EventLog::instance().count_events("::copy_ctor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("::move_ctor"), 0);
}
