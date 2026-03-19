// Test Suite: Type Traits and Metaprogramming
// Estimated Time: 3 hours
// Difficulty: Moderate to Hard
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <type_traits>
#include <vector>
#include <string>
#include <cstring>

class TypeTraitsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic Type Traits - Easy
// ============================================================================

TEST_F(TypeTraitsTest, BasicTypeTraits)
{
    EXPECT_TRUE(std::is_integral<int>::value);
    EXPECT_TRUE(std::is_floating_point<double>::value);
    EXPECT_TRUE(std::is_pointer<int*>::value);
    EXPECT_TRUE(std::is_reference<int&>::value);

    EXPECT_FALSE(std::is_integral<double>::value);
    EXPECT_FALSE(std::is_pointer<int>::value);

    // Q: Type traits are compile-time predicates. What is the runtime cost of checking
    // Q: std::is_integral<int>::value?
    // A:
    // R:

    // Q: How are type traits implemented? (Hint: template specialization)
    // A:
    // R:
}

// ============================================================================
// TEST 2: Type Transformations - Moderate
// ============================================================================

TEST_F(TypeTraitsTest, TypeTransformations)
{
    using plain_int = int;
    using const_int = std::add_const_t<int>;
    using int_ptr = std::add_pointer_t<int>;
    using int_ref = std::add_lvalue_reference_t<int>;

    using removed_const = std::remove_const_t<const int>;
    using removed_ptr = std::remove_pointer_t<int*>;
    using removed_ref = std::remove_reference_t<int&>;

    EXPECT_TRUE((std::is_same_v<const_int, const int>));
    EXPECT_TRUE((std::is_same_v<int_ptr, int*>));
    EXPECT_TRUE((std::is_same_v<int_ref, int&>));

    EXPECT_TRUE((std::is_same_v<removed_const, int>));
    EXPECT_TRUE((std::is_same_v<removed_ptr, int>));
    EXPECT_TRUE((std::is_same_v<removed_ref, int>));

    // Q: Type transformations like add_const_t return new types. How are these
    // Q: implemented? (Hint: template specialization, type aliases)
    // A:
    // R:

    // Q: std::remove_reference_t is useful for perfect forwarding. Why?
    // A:
    // R:
}

// ============================================================================
// TEST 3: Custom Type Traits - Moderate
// ============================================================================

template<typename T, typename = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),
    typename T::value_type
>> : std::true_type {};

TEST_F(TypeTraitsTest, CustomTypeTraits)
{
    EXPECT_TRUE(is_container<std::vector<int>>::value);
    EXPECT_TRUE(is_container<std::string>::value);
    EXPECT_FALSE(is_container<int>::value);
    EXPECT_FALSE(is_container<int*>::value);

    // Q: is_container checks for begin(), end(), and value_type. How does std::void_t
    // Q: enable this detection?
    // A:
    // R:

    // Q: If T doesn't have begin(), substitution fails. Which specialization is selected?
    // A:
    // R:

    // Q: How would you extend this to detect iterators vs containers?
    // A:
    // R:
}

// ============================================================================
// TEST 4: Type Traits for Function Selection - Moderate
// ============================================================================

template<typename T>
std::enable_if_t<std::is_trivially_copyable_v<T>, void>
copy_data(T* dest, const T* src, size_t count)
{
    EventLog::instance().record("copy_data: memcpy");
    std::memcpy(dest, src, count * sizeof(T));
}

template<typename T>
std::enable_if_t<!std::is_trivially_copyable_v<T>, void>
copy_data(T* dest, const T* src, size_t count)
{
    EventLog::instance().record("copy_data: element-wise");
    for (size_t i = 0; i < count; ++i)
    {
        dest[i] = src[i];
    }
}

TEST_F(TypeTraitsTest, TypeTraitsForFunctionSelection)
{
    int int_src[] = {1, 2, 3};
    int int_dest[3];
    copy_data(int_dest, int_src, 3);

    std::string str_src[] = {"a", "b", "c"};
    std::string str_dest[3];
    copy_data(str_dest, str_src, 3);

    EXPECT_EQ(int_dest[0], 1);
    EXPECT_EQ(str_dest[0], "a");

    // Q: Trivially copyable types can use memcpy. What types are trivially copyable?
    // A:
    // R:

    // Q: std::string is not trivially copyable. Why must it use element-wise copy?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("copy_data: memcpy"), 1);
    EXPECT_EQ(EventLog::instance().count_events("copy_data: element-wise"), 1);
}

// ============================================================================
// TEST 5: TODO - Implement is_callable Trait - Hard
// ============================================================================

// TODO: Implement is_callable trait that detects if a type can be called with
// TODO: specific argument types:
// TODO: 1. Use std::void_t and decltype
// TODO: 2. Test with functions, lambdas, functors
// TODO: 3. Test with different argument types

TEST_F(TypeTraitsTest, DISABLED_IsCallableTrait)
{
    // TODO: Implement is_callable trait
    // TODO: Test with various callable types

    // Q: How does is_callable differ from std::is_invocable?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Compile-Time Type Selection - Moderate
// ============================================================================

template<bool Condition, typename T, typename F>
struct conditional_type
{
    using type = T;
};

template<typename T, typename F>
struct conditional_type<false, T, F>
{
    using type = F;
};

TEST_F(TypeTraitsTest, CompileTimeTypeSelection)
{
    using type1 = conditional_type<true, int, double>::type;
    using type2 = conditional_type<false, int, double>::type;

    EXPECT_TRUE((std::is_same_v<type1, int>));
    EXPECT_TRUE((std::is_same_v<type2, double>));

    // Q: conditional_type selects between two types at compile time. How does this
    // Q: relate to std::conditional?
    // A:
    // R:

    // Q: When is compile-time type selection useful? (Hint: policy-based design,
    // Q: optimization)
    // A:
    // R:
}

// ============================================================================
// TEST 7: Type Traits Composition - Hard
// ============================================================================

template<typename T>
struct is_const_pointer : std::false_type {};

template<typename T>
struct is_const_pointer<const T*> : std::true_type {};

template<typename T>
constexpr bool is_const_pointer_v = is_const_pointer<T>::value;

TEST_F(TypeTraitsTest, TypeTraitsComposition)
{
    EXPECT_TRUE(is_const_pointer_v<const int*>);
    EXPECT_FALSE(is_const_pointer_v<int*>);
    EXPECT_FALSE(is_const_pointer_v<const int>);

    using T1 = const int*;
    using T2 = std::remove_const_t<std::remove_pointer_t<T1>>*;

    EXPECT_TRUE((std::is_same_v<T2, int*>));

    // Q: is_const_pointer combines two checks: is_pointer and is_const. How would you
    // Q: implement this using std::conjunction?
    // A:
    // R:

    // Q: Type trait composition allows building complex predicates. What is the
    // Q: performance cost of composing multiple traits?
    // A:
    // R:
}
