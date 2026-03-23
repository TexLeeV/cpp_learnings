// Test Suite: Variadic Templates
// Estimated Time: 4 hours
// Difficulty: Hard


#include <gtest/gtest.h>
#include "instrumentation.h"
#include <string>
#include <sstream>
#include <tuple>
#include <type_traits>

class VariadicTemplatesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic Variadic Templates - Moderate
// ============================================================================

template<typename... Args>
void log_args(Args... args)
{
    EventLog::instance().record("log_args: " + std::to_string(sizeof...(Args)) + " arguments");
}

TEST_F(VariadicTemplatesTest, BasicVariadicTemplates)
{
    log_args();
    log_args(1);
    log_args(1, 2.0);
    log_args(1, 2.0, "three");

    // Q: sizeof...(Args) returns the number of template arguments. What is the count
    // Q: for each call?
    // A:
    // R:

    // Q: Variadic templates accept any number of arguments. How does this differ from
    // Q: function overloading?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("log_args: 0 arguments"), 1);
    EXPECT_EQ(EventLog::instance().count_events("log_args: 1 arguments"), 1);
    EXPECT_EQ(EventLog::instance().count_events("log_args: 2 arguments"), 1);
    EXPECT_EQ(EventLog::instance().count_events("log_args: 3 arguments"), 1);
}

// ============================================================================
// TEST 2: Recursive Variadic Template Expansion - Hard
// ============================================================================

void print_impl()
{
    EventLog::instance().record("print_impl: base case");
}

template<typename T, typename... Args>
void print_impl(T first, Args... rest)
{
    EventLog::instance().record("print_impl: recursive");
    print_impl(rest...);
}

TEST_F(VariadicTemplatesTest, RecursiveVariadicExpansion)
{
    print_impl(1, 2.0, "three", 4);

    // Q: How many times is print_impl called recursively?
    // A:
    // R:

    // Q: The base case print_impl() has no template parameters. Why is this necessary?
    // A:
    // R:

    // Q: Each recursive call peels off one argument. What happens to the parameter pack
    // Q: at each level?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("print_impl: recursive"), 4);
    EXPECT_EQ(EventLog::instance().count_events("print_impl: base case"), 1);
}

// ============================================================================
// TEST 3: Fold Expressions (C++17) - Moderate
// ============================================================================

template<typename... Args>
auto sum_fold(Args... args)
{
    EventLog::instance().record("sum_fold");
    return (args + ...);
}

template<typename... Args>
auto sum_recursive(Args... args);

template<>
auto sum_recursive()
{
    return 0;
}

template<typename T, typename... Args>
auto sum_recursive(T first, Args... rest)
{
    return first + sum_recursive(rest...);
}

TEST_F(VariadicTemplatesTest, FoldExpressions)
{
    int result_fold = sum_fold(1, 2, 3, 4, 5);
    int result_recursive = sum_recursive(1, 2, 3, 4, 5);

    EXPECT_EQ(result_fold, 15);
    EXPECT_EQ(result_recursive, 15);

    // Q: Fold expressions (args + ...) expand to (arg1 + (arg2 + (arg3 + ...))).
    // Q: How does this simplify variadic template code?
    // A:
    // R:

    // Q: What are the four types of fold expressions? (unary left, unary right, binary
    // Q: left, binary right)
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("sum_fold"), 1);
}

// ============================================================================
// TEST 4: Variadic Class Templates - Moderate
// ============================================================================

template<typename... Types>
class Tuple
{
public:
    Tuple()
    {
        EventLog::instance().record("Tuple::ctor " + std::to_string(sizeof...(Types)) + " types");
    }

    static constexpr size_t size() { return sizeof...(Types); }
};

TEST_F(VariadicTemplatesTest, VariadicClassTemplates)
{
    Tuple<> empty;
    Tuple<int> single;
    Tuple<int, double, std::string> triple;

    EXPECT_EQ(empty.size(), 0);
    EXPECT_EQ(single.size(), 1);
    EXPECT_EQ(triple.size(), 3);

    // Q: Tuple stores types but not values. How does this differ from std::tuple?
    // A:
    // R:

    // Q: How would you implement actual storage for the values?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Tuple::ctor 0 types"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tuple::ctor 3 types"), 1);
}

// ============================================================================
// TEST 5: Parameter Pack Expansion Patterns - Hard
// ============================================================================

template<typename... Args>
void process_all(Args... args)
{
    EventLog::instance().record("process_all: start");
    (EventLog::instance().record("arg: " + std::to_string(args)), ...);
    EventLog::instance().record("process_all: end");
}

TEST_F(VariadicTemplatesTest, ParameterPackExpansionPatterns)
{
    process_all(1, 2, 3, 4);

    // Q: The fold expression (EventLog::record(...), ...) expands to a comma-separated
    // Q: sequence. How many times is record called?
    // A:
    // R:

    // Q: Comma operator evaluates left-to-right and returns the right operand. What
    // Q: does the fold expression expand to?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("process_all: start"), 1);
    EXPECT_EQ(EventLog::instance().count_events("arg:"), 4);
    EXPECT_EQ(EventLog::instance().count_events("process_all: end"), 1);
}

// ============================================================================
// TEST 6: TODO - Implement Variadic make_unique - Hard
// ============================================================================

// TODO: Implement a variadic make_unique_variadic that:
// TODO: 1. Accepts any number of constructor arguments
// TODO: 2. Forwards them to the constructor using perfect forwarding
// TODO: 3. Returns std::unique_ptr<T>

TEST_F(VariadicTemplatesTest, DISABLED_VariadicMakeUnique)
{
    // TODO: Implement make_unique_variadic
    // TODO: Test with different argument counts

    // Q: How does perfect forwarding interact with variadic templates?
    // A:
    // R:
}

// ============================================================================
// TEST 7: Variadic Template with Type Constraints - Hard
// ============================================================================

template<typename... Args>
auto sum_if_numeric(Args... args)
{
    static_assert((std::is_arithmetic_v<Args> && ...), "All arguments must be numeric");
    return (args + ...);
}

TEST_F(VariadicTemplatesTest, VariadicTemplateWithTypeConstraints)
{
    int result1 = sum_if_numeric(1, 2, 3);
    double result2 = sum_if_numeric(1.5, 2.5, 3.0);

    EXPECT_EQ(result1, 6);
    EXPECT_DOUBLE_EQ(result2, 7.0);

    // sum_if_numeric(1, "two", 3);  // Compile error: static_assert fails

    // Q: The fold expression (std::is_arithmetic_v<Args> && ...) checks all types.
    // Q: What does it expand to?
    // A:
    // R:

    // Q: static_assert provides compile-time type checking. How does this improve
    // Q: error messages compared to SFINAE?
    // A:
    // R:
}

// ============================================================================
// TEST 8: Index Sequence and Parameter Pack Indexing - Hard
// ============================================================================

template<typename Tuple, size_t... Is>
void print_tuple_impl(const Tuple& t, std::index_sequence<Is...>)
{
    ((EventLog::instance().record("tuple[" + std::to_string(Is) + "]")), ...);
}

template<typename... Args>
void print_tuple(const std::tuple<Args...>& t)
{
    print_tuple_impl(t, std::index_sequence_for<Args...>{});
}

TEST_F(VariadicTemplatesTest, IndexSequenceAndParameterPackIndexing)
{
    std::tuple<int, double, std::string> t(42, 3.14, "hello");

    print_tuple(t);

    // Q: std::index_sequence_for generates 0, 1, 2, ... for each type. How is this
    // Q: used to index into the tuple?
    // A:
    // R:

    // Q: std::get<Is>(t) retrieves the Is-th element. How does the fold expression
    // Q: expand this for all indices?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("tuple[0]"), 1);
    EXPECT_EQ(EventLog::instance().count_events("tuple[1]"), 1);
    EXPECT_EQ(EventLog::instance().count_events("tuple[2]"), 1);
}
