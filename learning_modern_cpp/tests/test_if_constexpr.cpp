// Test Suite: if constexpr and Fold Expressions
// Estimated Time: 3 hours
// Difficulty: Moderate

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <vector>

class IfConstexprTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: if constexpr Basics (Moderate)
// ============================================================================

template <typename T> std::string describe_type(T value)
{
    // Q: What is the difference between if constexpr and regular if?
    // A:
    // R:

    if constexpr (std::is_integral_v<T>)
    {
        return "integral: " + std::to_string(value);
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        return "floating: " + std::to_string(value);
    }
    else
    {
        return "other type";
    }
}

TEST_F(IfConstexprTest, BasicIfConstexpr)
{
    // Q: When is the condition in if constexpr evaluated?
    // A:
    // R:

    auto result_int = describe_type(42);
    EXPECT_EQ(result_int, "integral: 42");

    auto result_double = describe_type(3.14);
    EXPECT_EQ(result_double, "floating: 3.140000");

    auto result_string = describe_type(std::string("test"));
    EXPECT_EQ(result_string, "other type");

    // Q: What code is generated for describe_type<int>?
    // A:
    // R:
}

// ============================================================================
// Scenario 2: if constexpr for Type-Specific Behavior (Hard)
// ============================================================================

template <typename T> void process_value(const T& value)
{
    // Q: Why does this compile even though not all branches are valid for all types?
    // A:
    // R:

    if constexpr (std::is_pointer_v<T>)
    {
        EventLog::instance().record("Processing pointer");
        if (value != nullptr)
        {
            EventLog::instance().record("Pointer is valid");
        }
    }
    else if constexpr (std::is_integral_v<T>)
    {
        EventLog::instance().record("Processing integral: " + std::to_string(value));
    }
    else
    {
        EventLog::instance().record("Processing other type");
    }
}

TEST_F(IfConstexprTest, IfConstexprTypeSpecific)
{
    int* ptr = nullptr;
    process_value(ptr);

    EXPECT_EQ(EventLog::instance().count_events("Processing pointer"), 1);

    EventLog::instance().clear();

    int value = 42;
    process_value(value);

    EXPECT_EQ(EventLog::instance().count_events("Processing integral"), 1);

    // Q: What advantage does if constexpr have over function overloading?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Fold Expressions - Unary Folds (Moderate)
// ============================================================================

template <typename... Args> auto sum(Args... args)
{
    // Q: What does (args + ...) do?
    // A:
    // R:

    return (args + ...);
}

template <typename... Args> void log_all(Args... args)
{
    // Q: What does (EventLog::instance().record(args), ...) do?
    // A:
    // R:

    (EventLog::instance().record(std::to_string(args)), ...);
}

TEST_F(IfConstexprTest, UnaryFoldExpressions)
{
    // Q: How does the fold expression expand?
    // A:
    // R:

    auto result = sum(1, 2, 3, 4, 5);
    EXPECT_EQ(result, 15);

    auto result2 = sum(10, 20);
    EXPECT_EQ(result2, 30);

    log_all(1, 2, 3);

    EXPECT_EQ(EventLog::instance().count_events("1"), 1);
    EXPECT_EQ(EventLog::instance().count_events("2"), 1);
    EXPECT_EQ(EventLog::instance().count_events("3"), 1);
}

// ============================================================================
// Scenario 4: Fold Expressions - Binary Folds (Hard)
// ============================================================================

template <typename... Args> auto sum_with_init(Args... args)
{
    // Q: What does (0 + ... + args) do?
    // A:
    // R:

    return (0 + ... + args);
}

template <typename... Args> bool all_positive(Args... args)
{
    // Q: What does (... && (args > 0)) do?
    // A:
    // R:

    return (... && (args > 0));
}

TEST_F(IfConstexprTest, BinaryFoldExpressions)
{
    auto result = sum_with_init(1, 2, 3);
    EXPECT_EQ(result, 6);

    auto empty = sum_with_init();
    EXPECT_EQ(empty, 0);

    // Q: Why is the initial value important for empty parameter packs?
    // A:
    // R:

    EXPECT_TRUE(all_positive(1, 2, 3, 4));
    EXPECT_FALSE(all_positive(1, -2, 3));
    EXPECT_TRUE(all_positive(5));
}

// ============================================================================
// Scenario 5: Compile-Time Branching with Tracked (Hard)
// ============================================================================

template <typename T> auto make_container(const std::string& name)
{
    if constexpr (std::is_same_v<T, Tracked>)
    {
        EventLog::instance().record("make_container: creating Tracked");
        return Tracked(name);
    }
    else if constexpr (std::is_same_v<T, std::shared_ptr<Tracked>>)
    {
        EventLog::instance().record("make_container: creating shared_ptr<Tracked>");
        return std::make_shared<Tracked>(name);
    }
    else
    {
        EventLog::instance().record("make_container: creating default T");
        return T{};
    }
}

TEST_F(IfConstexprTest, CompileTimeBranchingWithTracked)
{
    // Q: How many instantiations of make_container exist?
    // A:
    // R:

    auto tracked = make_container<Tracked>("Direct");
    EXPECT_EQ(tracked.name(), "Direct");
    EXPECT_EQ(EventLog::instance().count_events("creating Tracked"), 1);

    EventLog::instance().clear();

    auto ptr = make_container<std::shared_ptr<Tracked>>("Shared");
    EXPECT_EQ(ptr->name(), "Shared");
    EXPECT_EQ(EventLog::instance().count_events("creating shared_ptr"), 1);

    // Q: What branches are compiled for make_container<Tracked>?
    // A:
    // R:
}

// ============================================================================
// Scenario 6: Variadic Fold with Tracked (Hard)
// ============================================================================

template <typename... Args> void construct_all(std::vector<Tracked>& vec, Args&&... args)
{
    // Q: What does (vec.emplace_back(std::forward<Args>(args)), ...) do?
    // A:
    // R:

    (vec.emplace_back(std::forward<Args>(args)), ...);
}

TEST_F(IfConstexprTest, VariadicFoldWithTracked)
{
    std::vector<Tracked> vec;

    construct_all(vec, "First", "Second", "Third");

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0].name(), "First");
    EXPECT_EQ(vec[1].name(), "Second");
    EXPECT_EQ(vec[2].name(), "Third");

    // Q: How many Tracked constructions occurred?
    // A:
    // R:
}
