// Test Suite: SFINAE and Type Traits
// Estimated Time: 1-2 hours
// Difficulty: Moderate to Hard
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <vector>

class SFINAETraitsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: enable_if vs if constexpr Selection (Moderate)
// ============================================================================

template <typename T> std::enable_if_t<std::is_integral_v<T>, T> scale(T value)
{
    EventLog::instance().record("scale: enable_if integral");
    return value * 2;
}

template <typename T> std::enable_if_t<std::is_floating_point_v<T>, T> scale(T value)
{
    EventLog::instance().record("scale: enable_if floating");
    return value * 3.0;
}

template <typename T> auto scale_constexpr(T value)
{
    if constexpr (std::is_integral_v<T>)
    {
        EventLog::instance().record("scale: if constexpr integral");
        return value * 2;
    }
    else
    {
        EventLog::instance().record("scale: if constexpr other");
        return value;
    }
}

TEST_F(SFINAETraitsTest, EnableIfAndIfConstexprSelectOverloads)
{
    EXPECT_EQ(scale(10), 20);
    EXPECT_DOUBLE_EQ(scale(2.0), 6.0);
    EXPECT_EQ(scale_constexpr(7), 14);
    EXPECT_EQ(scale_constexpr(std::string("x")), "x");

    // Q: When `scale(10)` is called, what happens to the floating-point overload during
    //    substitution, and which EventLog tag proves the integral path won?
    // A:
    // R:

    // Q: How does `if constexpr` discard the unused branch without relying on SFINAE
    //    overload sets?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("scale: enable_if integral"), 1);
    EXPECT_EQ(EventLog::instance().count_events("scale: enable_if floating"), 1);
    EXPECT_EQ(EventLog::instance().count_events("scale: if constexpr integral"), 1);
    EXPECT_EQ(EventLog::instance().count_events("scale: if constexpr other"), 1);
}

// ============================================================================
// Scenario 2: Basic std::is_* Traits (Easy)
// ============================================================================

TEST_F(SFINAETraitsTest, BasicIsTraitsClassifyTypes)
{
    EXPECT_TRUE(std::is_integral_v<int>);
    EXPECT_TRUE(std::is_floating_point_v<double>);
    EXPECT_TRUE(std::is_pointer_v<int*>);
    EXPECT_FALSE(std::is_integral_v<double>);
    EXPECT_FALSE(std::is_pointer_v<int>);

    EventLog::instance().record(std::is_integral_v<int> ? "trait: int_is_integral" : "trait: unexpected");

    // Q: What is the runtime cost of evaluating `std::is_integral_v<int>`, and what
    //    EventLog string confirms the compile-time result was observed?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("trait: int_is_integral"), 1);
}

// ============================================================================
// Scenario 3: remove_const / conditional_t Selection (Moderate)
// ============================================================================

template <typename T> using StorageT = std::conditional_t<std::is_const_v<T>, std::remove_const_t<T>, T>;

TEST_F(SFINAETraitsTest, RemoveConstAndConditionalSelectStorage)
{
    using FromConst = StorageT<const int>;
    using FromPlain = StorageT<int>;

    EXPECT_TRUE((std::is_same_v<FromConst, int>));
    EXPECT_TRUE((std::is_same_v<FromPlain, int>));
    EXPECT_TRUE((std::is_same_v<std::remove_const_t<const double>, double>));

    EventLog::instance().record(std::is_same_v<FromConst, int> ? "storage: stripped_const" : "storage: kept_const");

    // Q: For `StorageT<const int>`, which branch of `conditional_t` applies, and what
    //    type does `remove_const_t` produce?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("storage: stripped_const"), 1);
}

// ============================================================================
// Scenario 4: Member Detection via void_t / decltype (Hard)
// ============================================================================

template <typename T, typename = void> struct has_size : std::false_type
{
};

template <typename T> struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type
{
};

struct WithSize
{
    std::size_t size() const
    {
        return 42;
    }
};

struct WithoutSize
{
};

TEST_F(SFINAETraitsTest, MemberDetectionViaVoidT)
{
    EXPECT_TRUE(has_size<std::vector<int>>::value);
    EXPECT_TRUE(has_size<WithSize>::value);
    EXPECT_FALSE(has_size<WithoutSize>::value);
    EXPECT_FALSE(has_size<int>::value);

    EventLog::instance().record(has_size<WithSize>::value ? "detect: has_size" : "detect: missing");
    EventLog::instance().record(has_size<WithoutSize>::value ? "detect: unexpected" : "detect: no_size");

    // Q: When `T` has no `size()`, which specialization of `has_size` is selected, and
    //    why is that failure not a hard error?
    // A:
    // R:

    // Q: Why does `std::declval<T>()` appear inside `decltype` instead of constructing
    //    a real `T`?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("detect: has_size"), 1);
    EXPECT_EQ(EventLog::instance().count_events("detect: no_size"), 1);
}
