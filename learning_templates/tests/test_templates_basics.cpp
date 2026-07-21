// Test Suite: Templates Basics
// Estimated Time: 1-2 hours
// Difficulty: Easy to Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <string>
#include <type_traits>

class TemplatesBasicsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Function Template Instantiation (Easy)
// ============================================================================

template <typename T> T max_value(T a, T b)
{
    EventLog::instance().record("max_value");
    return (a > b) ? a : b;
}

TEST_F(TemplatesBasicsTest, FunctionTemplateInstantiatesPerType)
{
    EXPECT_EQ(max_value(10, 20), 20);
    EXPECT_EQ(max_value(3.14, 2.71), 3.14);
    EXPECT_EQ(max_value(std::string("apple"), std::string("banana")), "banana");

    // Q: How many distinct `max_value` instantiations does this test produce, and what
    //    EventLog count confirms each call ran?
    // A:
    // R:

    // Q: Hypothetical: would `max_value(10, 3.14)` compile with this single-parameter
    //    template, and why?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("max_value"), 3);
}

// ============================================================================
// Scenario 2: Class Template Distinct Types (Easy)
// ============================================================================

template <typename T> class Box
{
public:
    explicit Box(T value) : value_(value)
    {
        EventLog::instance().record("Box::ctor");
    }

    T get() const
    {
        return value_;
    }

private:
    T value_;
};

TEST_F(TemplatesBasicsTest, ClassTemplateYieldsDistinctTypes)
{
    Box<int> int_box(42);
    Box<std::string> str_box("hello");

    EXPECT_EQ(int_box.get(), 42);
    EXPECT_EQ(str_box.get(), "hello");

    // Q: Why are `Box<int>` and `Box<std::string>` different types, and what would
    //    fail if you tried to assign one to the other?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Box::ctor"), 2);
}

// ============================================================================
// Scenario 3: Type Deduction Strips Top-Level Qualifiers (Moderate)
// ============================================================================

template <typename T> void classify(T)
{
    if constexpr (std::is_pointer_v<T>)
    {
        EventLog::instance().record("classify: pointer");
    }
    else
    {
        EventLog::instance().record("classify: value");
    }
}

TEST_F(TemplatesBasicsTest, TypeDeductionSelectsPointerOrValue)
{
    int x = 42;
    int* ptr = &x;

    classify(x);
    classify(ptr);
    classify(&x);

    // Q: When calling `classify(x)`, what is `T`? When calling `classify(ptr)`, what
    //    is `T`, and which EventLog tags confirm the split?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("classify: value"), 1);
    EXPECT_EQ(EventLog::instance().count_events("classify: pointer"), 2);
}

// ============================================================================
// Scenario 4: Non-Type Template Parameter (Moderate)
// ============================================================================

template <typename T, std::size_t N> class FixedArray
{
public:
    FixedArray()
    {
        EventLog::instance().record("FixedArray::ctor N=" + std::to_string(N));
    }

    constexpr std::size_t size() const
    {
        return N;
    }

    T& operator[](std::size_t i)
    {
        return data_[i];
    }

private:
    T data_[N];
};

TEST_F(TemplatesBasicsTest, NonTypeParameterIsPartOfTheType)
{
    FixedArray<int, 5> a5;
    FixedArray<int, 10> a10;
    a5[0] = 42;
    a10[0] = 100;

    EXPECT_EQ(a5.size(), 5u);
    EXPECT_EQ(a10.size(), 10u);
    EXPECT_EQ(a5[0], 42);

    // Q: Why are `FixedArray<int, 5>` and `FixedArray<int, 10>` different types, and
    //    what compile-time guarantee does baking `N` into the type buy you?
    // A:
    // R:

    // Q: Which EventLog strings prove both sizes were recorded at construction?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("FixedArray::ctor N=5"), 1);
    EXPECT_EQ(EventLog::instance().count_events("FixedArray::ctor N=10"), 1);
}
