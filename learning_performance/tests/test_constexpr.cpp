// Test Suite: Compile-Time Computation with constexpr
// Estimated Time: 3 hours
// Difficulty: Moderate to Hard
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <array>
#include <cmath>

class ConstexprTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic constexpr Functions - Easy
// ============================================================================

constexpr int factorial(int n)
{
    return (n <= 1) ? 1 : n * factorial(n - 1);
}

constexpr int fibonacci(int n)
{
    return (n <= 1) ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

TEST_F(ConstexprTest, BasicConstexprFunctions)
{
    constexpr int fact5 = factorial(5);
    constexpr int fib10 = fibonacci(10);

    static_assert(factorial(5) == 120, "factorial(5) should be 120");
    static_assert(fibonacci(10) == 55, "fibonacci(10) should be 55");

    EXPECT_EQ(fact5, 120);
    EXPECT_EQ(fib10, 55);

    // Q: constexpr functions are evaluated at compile time when used in constant
    // Q: expressions. What is the runtime cost of computing fact5?
    // A:
    // R:

    // Q: Can you call factorial(n) with a runtime variable? What happens?
    // A:
    // R:

    int runtime_n = 6;
    int runtime_fact = factorial(runtime_n);
    EXPECT_EQ(runtime_fact, 720);
}

// ============================================================================
// TEST 2: constexpr vs const - Moderate
// ============================================================================

constexpr int compute_at_compile_time(int x)
{
    return x * x + 2 * x + 1;
}

int compute_at_runtime(int x)
{
    return x * x + 2 * x + 1;
}

TEST_F(ConstexprTest, ConstexprVsConst)
{
    constexpr int compile_time = compute_at_compile_time(10);
    const int runtime_const = compute_at_runtime(10);

    std::array<int, compile_time> arr1;
    // std::array<int, runtime_const> arr2;  // Compile error: runtime_const is not constexpr

    EXPECT_EQ(compile_time, 121);
    EXPECT_EQ(runtime_const, 121);

    // Q: compile_time can be used as an array size, but runtime_const cannot. Why?
    // A:
    // R:

    // Q: What is the difference between const and constexpr for variables?
    // A:
    // R:
}

// ============================================================================
// TEST 3: constexpr Constructors and Objects - Moderate
// ============================================================================

class Point
{
public:
    constexpr Point(int x, int y)
    : x_(x), y_(y)
    {
    }

    constexpr int x() const { return x_; }
    constexpr int y() const { return y_; }

    constexpr int distance_squared() const
    {
        return x_ * x_ + y_ * y_;
    }

private:
    int x_, y_;
};

TEST_F(ConstexprTest, ConstexprConstructorsAndObjects)
{
    constexpr Point p1(3, 4);
    constexpr int dist_sq = p1.distance_squared();

    static_assert(p1.x() == 3, "x should be 3");
    static_assert(p1.y() == 4, "y should be 4");
    static_assert(p1.distance_squared() == 25, "distance squared should be 25");

    EXPECT_EQ(dist_sq, 25);

    // Q: Point p1 is a constexpr object. Where is it stored: stack, heap, or data segment?
    // A:
    // R:

    // Q: Can you create a constexpr Point at runtime with non-const arguments?
    // A:
    // R:

    int runtime_x = 5;
    Point p2(runtime_x, 12);
    EXPECT_EQ(p2.distance_squared(), 169);
}

// ============================================================================
// TEST 4: constexpr if in C++17 - Moderate
// ============================================================================

template<typename T>
constexpr T absolute_value(T value)
{
    if constexpr (std::is_unsigned_v<T>)
    {
        return value;
    }
    else
    {
        return value < 0 ? -value : value;
    }
}

TEST_F(ConstexprTest, ConstexprIfC17)
{
    constexpr int neg = absolute_value(-5);
    constexpr unsigned int pos = absolute_value(5u);

    EXPECT_EQ(neg, 5);
    EXPECT_EQ(pos, 5u);

    // Q: if constexpr discards branches at compile time. What happens to the
    // Q: "value < 0" branch when T is unsigned?
    // A:
    // R:

    // Q: How is if constexpr different from a regular if statement in a template?
    // A:
    // R:
}

// ============================================================================
// TEST 5: constexpr Limitations - Moderate
// ============================================================================

constexpr int valid_constexpr(int x)
{
    int result = 0;
    for (int i = 0; i < x; ++i)
    {
        result += i;
    }
    return result;
}

TEST_F(ConstexprTest, ConstexprLimitations)
{
    constexpr int sum = valid_constexpr(10);

    static_assert(valid_constexpr(10) == 45, "sum should be 45");

    EXPECT_EQ(sum, 45);

    // Q: This constexpr function uses a loop and local variables. What C++ standard
    // Q: allows this? (C++11 had stricter constexpr rules)
    // A:
    // R:

    // Q: What operations are still forbidden in constexpr functions even in C++17?
    // Q: (Hint: dynamic allocation, I/O, exceptions)
    // A:
    // R:
}

// ============================================================================
// TEST 6: TODO - Implement Compile-Time Lookup Table - Hard
// ============================================================================

// TODO: Implement a constexpr function that generates a lookup table at compile time:
// TODO: 1. Create constexpr function to compute sin approximation
// TODO: 2. Generate std::array<float, 360> with precomputed values
// TODO: 3. Verify table is generated at compile time using static_assert

TEST_F(ConstexprTest, DISABLED_CompileTimeLookupTable)
{
    // TODO: Implement constexpr sin_table generator
    // TODO: Use static_assert to verify compile-time evaluation

    // Q: Lookup tables trade memory for speed. What is the cache vs computation tradeoff?
    // A:
    // R:
}

// ============================================================================
// TEST 7: constexpr std::array vs C Array - Easy
// ============================================================================

constexpr std::array<int, 5> create_array()
{
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    return arr;
}

TEST_F(ConstexprTest, ConstexprStdArray)
{
    constexpr auto arr = create_array();

    static_assert(arr[0] == 1, "first element should be 1");
    static_assert(arr.size() == 5, "size should be 5");

    EXPECT_EQ(arr[2], 3);

    // Q: std::array is a constexpr-friendly container. What makes it suitable for
    // Q: compile-time computation compared to std::vector?
    // A:
    // R:

    // Q: Can you use std::array in a constexpr context with runtime size?
    // A:
    // R:
}

// ============================================================================
// TEST 8: Performance Impact of constexpr - Moderate
// ============================================================================

constexpr int power(int base, int exp)
{
    int result = 1;
    for (int i = 0; i < exp; ++i)
    {
        result *= base;
    }
    return result;
}

TEST_F(ConstexprTest, PerformanceImpactOfConstexpr)
{
    constexpr int compile_time_power = power(2, 20);

    int runtime_base = 2;
    int runtime_exp = 20;
    int runtime_power = power(runtime_base, runtime_exp);

    EXPECT_EQ(compile_time_power, 1048576);
    EXPECT_EQ(runtime_power, 1048576);

    // Q: compile_time_power is computed at compile time. What is the assembly code
    // Q: generated for this variable? (Hint: it's a constant)
    // A:
    // R:

    // Q: runtime_power is computed at runtime. What is the performance difference
    // Q: between the two?
    // A:
    // R:

    // Q: When should you prefer constexpr over runtime computation?
    // A:
    // R:
}
