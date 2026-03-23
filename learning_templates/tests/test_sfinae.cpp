// Test Suite: SFINAE (Substitution Failure Is Not An Error)
// Estimated Time: 4 hours
// Difficulty: Hard


#include <gtest/gtest.h>
#include "instrumentation.h"
#include <type_traits>
#include <vector>
#include <string>

class SFINAETest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic SFINAE with enable_if - Moderate
// ============================================================================

template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process(T value)
{
    EventLog::instance().record("process: integral");
    return value * 2;
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
process(T value)
{
    EventLog::instance().record("process: floating_point");
    return value * 3.0;
}

TEST_F(SFINAETest, BasicSFINAEWithEnableIf)
{
    int result_int = process(10);
    double result_double = process(3.14);

    EXPECT_EQ(result_int, 20);
    EXPECT_DOUBLE_EQ(result_double, 9.42);

    // Q: When calling process(10), the floating_point overload fails substitution.
    // Q: What happens to that overload?
    // A:
    // R:

    // Q: SFINAE means "Substitution Failure Is Not An Error". What would happen without
    // Q: SFINAE when substitution fails?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("process: integral"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process: floating_point"), 1);
}

// ============================================================================
// TEST 2: SFINAE with enable_if_t (C++14) - Moderate
// ============================================================================

template<typename T, std::enable_if_t<std::is_pointer<T>::value, int> = 0>
void handle(T value)
{
    EventLog::instance().record("handle: pointer");
}

template<typename T, std::enable_if_t<!std::is_pointer<T>::value, int> = 0>
void handle(T value)
{
    EventLog::instance().record("handle: non-pointer");
}

TEST_F(SFINAETest, SFINAEWithEnableIfT)
{
    int x = 42;
    int* ptr = &x;

    handle(x);
    handle(ptr);

    // Q: enable_if_t is a C++14 alias for enable_if<...>::type. How does this improve
    // Q: readability?
    // A:
    // R:

    // Q: The second template parameter is a non-type parameter with default value 0.
    // Q: Why is this pattern used for SFINAE?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("handle: non-pointer"), 1);
    EXPECT_EQ(EventLog::instance().count_events("handle: pointer"), 1);
}

// ============================================================================
// TEST 3: SFINAE for Member Detection - Hard
// ============================================================================

template<typename T, typename = void>
struct has_size : std::false_type {};

template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

class WithSize
{
public:
    size_t size() const { return 42; }
};

class WithoutSize
{
};

TEST_F(SFINAETest, SFINAEForMemberDetection)
{
    EXPECT_TRUE(has_size<std::vector<int>>::value);
    EXPECT_TRUE(has_size<std::string>::value);
    EXPECT_TRUE(has_size<WithSize>::value);
    EXPECT_FALSE(has_size<WithoutSize>::value);
    EXPECT_FALSE(has_size<int>::value);

    // Q: has_size uses std::void_t and decltype to detect the size() member. How does
    // Q: this work?
    // A:
    // R:

    // Q: std::declval<T>() creates a "fake" T without constructing it. Why is this
    // Q: necessary for SFINAE?
    // A:
    // R:

    // Q: If T doesn't have size(), substitution fails and the second specialization
    // Q: is discarded. Which specialization is selected?
    // A:
    // R:
}

// ============================================================================
// TEST 4: SFINAE with Return Type Deduction - Hard
// ============================================================================

template<typename T>
auto get_size(T& container) -> decltype(container.size())
{
    EventLog::instance().record("get_size: has_size");
    return container.size();
}

template<typename T, size_t N>
auto get_size(T (&arr)[N]) -> size_t
{
    EventLog::instance().record("get_size: array");
    return N;
}

TEST_F(SFINAETest, SFINAEWithReturnTypeDeduction)
{
    std::vector<int> vec = {1, 2, 3};
    int arr[] = {4, 5, 6};

    size_t vec_size = get_size(vec);
    size_t arr_size = get_size(arr);

    EXPECT_EQ(vec_size, 3);
    EXPECT_EQ(arr_size, 3);

    // Q: std::vector has size() member. What happens to the array overload during
    // Q: substitution?
    // A:
    // R:

    // Q: C arrays don't have size() member. What happens to the first overload during
    // Q: substitution?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("get_size: has_size"), 1);
    EXPECT_EQ(EventLog::instance().count_events("get_size: array"), 1);
}

// ============================================================================
// TEST 5: TODO - Implement SFINAE for Iterator Detection - Hard
// ============================================================================

// TODO: Implement has_iterator trait that detects if a type has:
// TODO: 1. begin() and end() member functions
// TODO: 2. Use SFINAE with std::void_t
// TODO: 3. Test with std::vector, std::string, int

TEST_F(SFINAETest, DISABLED_SFINAEForIteratorDetection)
{
    // TODO: Implement has_iterator trait
    // TODO: Test with various types

    // Q: How would you extend this to detect const_iterator?
    // A:
    // R:
}

// ============================================================================
// TEST 6: SFINAE vs if constexpr - Moderate
// ============================================================================

template<typename T>
void print_size_sfinae(const T& container,
                       typename std::enable_if<std::is_same<T, std::vector<int>>::value>::type* = nullptr)
{
    EventLog::instance().record("print_size: SFINAE vector");
}

template<typename T>
void print_size_sfinae(const T& container,
                       typename std::enable_if<std::is_same<T, std::string>::value>::type* = nullptr)
{
    EventLog::instance().record("print_size: SFINAE string");
}

template<typename T>
void print_size_if_constexpr(const T& container)
{
    if constexpr (std::is_same_v<T, std::vector<int>>)
    {
        EventLog::instance().record("print_size: if constexpr vector");
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        EventLog::instance().record("print_size: if constexpr string");
    }
}

TEST_F(SFINAETest, SFINAEVsIfConstexpr)
{
    std::vector<int> vec = {1, 2, 3};
    std::string str = "hello";

    print_size_sfinae(vec);
    print_size_sfinae(str);

    EventLog::instance().clear();

    print_size_if_constexpr(vec);
    print_size_if_constexpr(str);

    // Q: SFINAE uses overload resolution, if constexpr uses compile-time branching.
    // Q: Which is more readable?
    // A:
    // R:

    // Q: SFINAE can participate in overload resolution, if constexpr cannot. When is
    // Q: SFINAE still necessary in C++17?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("print_size: if constexpr vector"), 1);
    EXPECT_EQ(EventLog::instance().count_events("print_size: if constexpr string"), 1);
}

// ============================================================================
// TEST 7: SFINAE with Expression SFINAE - Hard
// ============================================================================

template<typename T>
auto add_if_possible(T a, T b) -> decltype(a + b)
{
    EventLog::instance().record("add_if_possible: success");
    return a + b;
}

struct NonAddable
{
};

TEST_F(SFINAETest, SFINAEWithExpressionSFINAE)
{
    int result_int = add_if_possible(10, 20);
    std::string result_str = add_if_possible(std::string("hello"), std::string(" world"));

    EXPECT_EQ(result_int, 30);
    EXPECT_EQ(result_str, "hello world");

    // NonAddable na1, na2;
    // auto result = add_if_possible(na1, na2);  // Compile error: no matching function

    // Q: decltype(a + b) checks if a + b is valid. What happens if T doesn't support
    // Q: operator+?
    // A:
    // R:

    // Q: Expression SFINAE allows template selection based on valid expressions. How
    // Q: does this differ from type-based SFINAE?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("add_if_possible: success"), 2);
}
