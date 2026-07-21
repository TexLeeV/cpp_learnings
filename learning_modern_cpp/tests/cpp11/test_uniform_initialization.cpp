// Test Suite: Uniform Initialization and Initializer Lists
// Estimated Time: 2 hours
// Difficulty: Easy
// Introduced in: C++11
// C++ Standard: C++20 (project build)

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <initializer_list>
#include <map>
#include <vector>

class UniformInitializationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Uniform Initialization Basics (Easy)
// ============================================================================

TEST_F(UniformInitializationTest, BasicUniformInitialization)
{
    // Q: What is uniform initialization (brace initialization)?
    // A:
    // R:

    int x{42};
    int y = {42};
    int z(42);
    int w = 42;

    EXPECT_EQ(x, 42);
    EXPECT_EQ(y, 42);
    EXPECT_EQ(z, 42);
    EXPECT_EQ(w, 42);

    // Q: What advantage does {} have over () for initialization?
    // A:
    // R:

    std::vector<int> vec{1, 2, 3, 4, 5};

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec[0], 1);

    // Q: What would std::vector<int> vec(5) create?
    // A:
    // R:

    std::vector<int> vec2(5);
    EXPECT_EQ(vec2.size(), 5);
    EXPECT_EQ(vec2[0], 0);
}

// ============================================================================
// Scenario 2: Narrowing Conversions (Moderate)
// ============================================================================

TEST_F(UniformInitializationTest, NarrowingConversions)
{
    // Q: What is a narrowing conversion?
    // A:
    // R:

    double d = 3.14;

    // This compiles (narrowing allowed)
    int x = d;
    EXPECT_EQ(x, 3);

    // Q: What happens with brace initialization and narrowing?
    // A:
    // R:

    // int y{d};  // This would fail to compile (narrowing prevented)

    int safe = static_cast<int>(d);
    EXPECT_EQ(safe, 3);

    // Q: Why does C++11 prevent narrowing with {}?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Initializer Lists (Moderate)
// ============================================================================

class Container
{
public:
    Container(std::initializer_list<int> list)
    {
        for (int val : list)
        {
            values_.push_back(val);
        }
        EventLog::instance().record("Container::ctor with initializer_list (size=" + std::to_string(list.size()) + ")");
    }

    Container(int count, int value)
    {
        values_.resize(count, value);
        EventLog::instance().record("Container::ctor with count=" + std::to_string(count) +
                                    ", value=" + std::to_string(value));
    }

    size_t size() const
    {
        return values_.size();
    }

    int operator[](size_t idx) const
    {
        return values_[idx];
    }

private:
    std::vector<int> values_;
};

TEST_F(UniformInitializationTest, InitializerListPriority)
{
    // Q: Which constructor is called?
    // A:
    // R:

    Container c1{1, 2, 3, 4, 5};

    EXPECT_EQ(c1.size(), 5);
    EXPECT_EQ(EventLog::instance().count_events("initializer_list"), 1);

    // Q: Which constructor is called here?
    // A:
    // R:

    EventLog::instance().clear();
    Container c2(3, 10);

    EXPECT_EQ(c2.size(), 3);
    EXPECT_EQ(c2[0], 10);
    EXPECT_EQ(EventLog::instance().count_events("count="), 1);

    // Q: How would you call the count/value constructor using brace initialization?
    // A:
    // R:
}

// ============================================================================
// Scenario 4: Most Vexing Parse (Easy)
// ============================================================================

class Widget
{
public:
    Widget()
    {
        EventLog::instance().record("Widget::default_ctor");
    }

    explicit Widget(int value)
    {
        EventLog::instance().record("Widget::ctor(int=" + std::to_string(value) + ")");
    }
};

TEST_F(UniformInitializationTest, MostVexingParse)
{
    // Q: What does this declare?
    // A:
    // R:

    // Widget w();  // This is a function declaration, not an object!

    // Q: How does uniform initialization solve this?
    // A:
    // R:

    Widget w1{};

    EXPECT_EQ(EventLog::instance().count_events("default_ctor"), 1);

    Widget w2{42};

    EXPECT_EQ(EventLog::instance().count_events("ctor(int="), 1);
}

// ============================================================================
// Scenario 5: Aggregate Initialization (Moderate)
// ============================================================================

struct Point
{
    int x;
    int y;
    int z;
};

TEST_F(UniformInitializationTest, AggregateInitialization)
{
    // Q: What is an aggregate type?
    // A:
    // R:

    Point p1{1, 2, 3};

    EXPECT_EQ(p1.x, 1);
    EXPECT_EQ(p1.y, 2);
    EXPECT_EQ(p1.z, 3);

    // Q: What happens if we provide fewer initializers than members?
    // A:
    // R:

    Point p2{10, 20};

    EXPECT_EQ(p2.x, 10);
    EXPECT_EQ(p2.y, 20);
    EXPECT_EQ(p2.z, 0);

    // Q: Can we use designated initializers in C++17?
    // A:
    // R:
}
