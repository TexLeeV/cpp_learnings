// Test Suite: Practical Template Metaprogramming
// Estimated Time: 4 hours
// Difficulty: Hard

#include "instrumentation.h"

#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

class PracticalMetaprogrammingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Compile-Time Fibonacci - Moderate
// ============================================================================

template <int N> struct Fibonacci
{
    static constexpr int value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};

template <> struct Fibonacci<0>
{
    static constexpr int value = 0;
};

template <> struct Fibonacci<1>
{
    static constexpr int value = 1;
};

TEST_F(PracticalMetaprogrammingTest, CompileTimeFibonacci)
{
    constexpr int fib10 = Fibonacci<10>::value;
    constexpr int fib15 = Fibonacci<15>::value;

    static_assert(Fibonacci<10>::value == 55, "fib(10) should be 55");
    static_assert(Fibonacci<15>::value == 610, "fib(15) should be 610");

    EXPECT_EQ(fib10, 55);
    EXPECT_EQ(fib15, 610);

    // Q: Fibonacci is computed at compile time using template recursion. What is the
    // Q: runtime cost of accessing fib10?
    // A:
    // R:

    // Q: Template metaprogramming is Turing-complete. What are the limitations compared
    // Q: to runtime computation?
    // A:
    // R:
}

// ============================================================================
// TEST 2: Type List Manipulation - Hard
// ============================================================================

template <typename... Types> struct TypeList
{
    static constexpr size_t size = sizeof...(Types);
};

template <typename List> struct TypeListSize;

template <typename... Types> struct TypeListSize<TypeList<Types...>>
{
    static constexpr size_t value = sizeof...(Types);
};

template <typename T, typename List> struct PushFront;

template <typename T, typename... Types> struct PushFront<T, TypeList<Types...>>
{
    using type = TypeList<T, Types...>;
};

TEST_F(PracticalMetaprogrammingTest, TypeListManipulation)
{
    using List1 = TypeList<int, double, std::string>;
    using List2 = PushFront<float, List1>::type;

    EXPECT_EQ(TypeListSize<List1>::value, 3);
    EXPECT_EQ(TypeListSize<List2>::value, 4);

    // Q: TypeList is a compile-time container of types. How does this differ from
    // Q: std::tuple?
    // A:
    // R:

    // Q: PushFront adds a type to the front of the list. How would you implement
    // Q: PushBack, PopFront, or Concatenate?
    // A:
    // R:
}

// ============================================================================
// TEST 3: Tuple Utilities with Metaprogramming - Hard
// ============================================================================

template <typename Tuple, size_t... Is>
auto tuple_to_string_impl(const Tuple& t, std::index_sequence<Is...>) -> std::string
{
    std::string result;
    ((result += std::to_string(std::get<Is>(t)) + " "), ...);
    return result;
}

template <typename... Args> auto tuple_to_string(const std::tuple<Args...>& t) -> std::string
{
    return tuple_to_string_impl(t, std::index_sequence_for<Args...>{});
}

TEST_F(PracticalMetaprogrammingTest, TupleUtilitiesWithMetaprogramming)
{
    std::tuple<int, double, int> t(42, 3.14, 100);

    std::string result = tuple_to_string(t);

    EventLog::instance().record("Tuple string: " + result);

    // Q: std::index_sequence_for generates 0, 1, 2, ... How is this used to access
    // Q: tuple elements at compile time?
    // A:
    // R:

    // Q: The fold expression ((result += ...), ...) processes all elements. What is
    // Q: the expansion order?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Tuple string:"), 1);
}

// ============================================================================
// TEST 4: Tag Dispatching - Moderate
// ============================================================================

template <typename T> void advance_impl(T& iter, int n, std::random_access_iterator_tag)
{
    EventLog::instance().record("advance: random_access");
    iter += n;
}

template <typename T> void advance_impl(T& iter, int n, std::bidirectional_iterator_tag)
{
    EventLog::instance().record("advance: bidirectional");
    if (n >= 0)
    {
        for (int i = 0; i < n; ++i)
            ++iter;
    }
    else
    {
        for (int i = 0; i > n; --i)
            --iter;
    }
}

template <typename T> void advance_impl(T& iter, int n, std::forward_iterator_tag)
{
    EventLog::instance().record("advance: forward");
    for (int i = 0; i < n; ++i)
        ++iter;
}

template <typename T> void advance_custom(T& iter, int n)
{
    advance_impl(iter, n, typename std::iterator_traits<T>::iterator_category{});
}

TEST_F(PracticalMetaprogrammingTest, TagDispatching)
{
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto vec_iter = vec.begin();
    advance_custom(vec_iter, 2);

    EXPECT_EQ(*vec_iter, 3);

    // Q: Tag dispatching uses iterator category tags to select the optimal algorithm.
    // Q: How does this differ from SFINAE?
    // A:
    // R:

    // Q: Random access iterators can use += for O(1) advance. What is the complexity
    // Q: for forward iterators?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("advance: random_access"), 1);
}

// ============================================================================
// TEST 5: TODO - Implement Compile-Time String Hashing - Hard
// ============================================================================

// TODO: Implement a constexpr string hash function that:
// TODO: 1. Computes hash at compile time for string literals
// TODO: 2. Use template recursion or constexpr function
// TODO: 3. Compare with runtime hashing

TEST_F(PracticalMetaprogrammingTest, DISABLED_CompileTimeStringHashing)
{
    // TODO: Implement constexpr hash function
    // TODO: Test with string literals
    // TODO: Verify compile-time evaluation

    // Q: Compile-time hashing enables switch statements on strings. How?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Perfect Forwarding with Variadic Templates - Hard
// ============================================================================

template <typename T, typename... Args> std::unique_ptr<T> make_unique_custom(Args&&... args)
{
    EventLog::instance().record("make_unique_custom: " + std::to_string(sizeof...(Args)) + " args");
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Widget
{
public:
    Widget()
    {
        EventLog::instance().record("Widget::ctor()");
    }

    Widget(int x)
    {
        EventLog::instance().record("Widget::ctor(int)");
    }

    Widget(int x, double y)
    {
        EventLog::instance().record("Widget::ctor(int, double)");
    }
};

TEST_F(PracticalMetaprogrammingTest, PerfectForwardingWithVariadicTemplates)
{
    auto w1 = make_unique_custom<Widget>();
    auto w2 = make_unique_custom<Widget>(42);
    auto w3 = make_unique_custom<Widget>(42, 3.14);

    // Q: std::forward<Args>(args)... forwards each argument perfectly. What does
    // Q: "perfectly" mean?
    // A:
    // R:

    // Q: Without std::forward, what would happen to rvalue arguments?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Widget::ctor()"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Widget::ctor(int)"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Widget::ctor(int, double)"), 1);
}

// ============================================================================
// TEST 7: Compile-Time Conditional Execution - Moderate
// ============================================================================

template <typename T> void process_type()
{
    if constexpr (std::is_integral_v<T>)
    {
        EventLog::instance().record("process_type: integral");
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        EventLog::instance().record("process_type: floating_point");
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        EventLog::instance().record("process_type: pointer");
    }
    else
    {
        EventLog::instance().record("process_type: other");
    }
}

TEST_F(PracticalMetaprogrammingTest, CompileTimeConditionalExecution)
{
    process_type<int>();
    process_type<double>();
    process_type<int*>();
    process_type<std::string>();

    // Q: if constexpr discards branches at compile time. What happens to the code in
    // Q: discarded branches?
    // A:
    // R:

    // Q: How does if constexpr differ from runtime if for template metaprogramming?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("process_type: integral"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_type: floating_point"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_type: pointer"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_type: other"), 1);
}

// ============================================================================
// TEST 8: Type-Based Function Dispatch - Hard
// ============================================================================

template <typename T> void serialize(const T& value, std::string& output)
{
    if constexpr (std::is_arithmetic_v<T>)
    {
        EventLog::instance().record("serialize: arithmetic");
        output += std::to_string(value);
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        EventLog::instance().record("serialize: string");
        output += value;
    }
    else if constexpr (std::is_same_v<T, std::vector<int>>)
    {
        EventLog::instance().record("serialize: vector");
        output += "[";
        for (size_t i = 0; i < value.size(); ++i)
        {
            if (i > 0)
                output += ",";
            output += std::to_string(value[i]);
        }
        output += "]";
    }
}

TEST_F(PracticalMetaprogrammingTest, TypeBasedFunctionDispatch)
{
    std::string result;

    serialize(42, result);
    result += " ";
    serialize(3.14, result);
    result += " ";
    serialize(std::string("hello"), result);
    result += " ";
    serialize(std::vector<int>{1, 2, 3}, result);

    EXPECT_EQ(result, "42 3.140000 hello [1,2,3]");

    // Q: Type-based dispatch allows different behavior for different types. How does
    // Q: this enable generic programming?
    // A:
    // R:

    // Q: What happens if you call serialize with a type that doesn't match any branch?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("serialize: arithmetic"), 2);
    EXPECT_EQ(EventLog::instance().count_events("serialize: string"), 1);
    EXPECT_EQ(EventLog::instance().count_events("serialize: vector"), 1);
}
