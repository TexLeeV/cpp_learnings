// Test Suite: Template Specialization
// Estimated Time: 3 hours
// Difficulty: Moderate to Hard


#include <gtest/gtest.h>
#include "instrumentation.h"
#include <string>
#include <vector>
#include <type_traits>

class TemplateSpecializationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Full Template Specialization - Easy
// ============================================================================

template<typename T>
class TypeName
{
public:
    static std::string name()
    {
        EventLog::instance().record("TypeName::name (generic)");
        return "unknown";
    }
};

template<>
class TypeName<int>
{
public:
    static std::string name()
    {
        EventLog::instance().record("TypeName::name (int)");
        return "int";
    }
};

template<>
class TypeName<double>
{
public:
    static std::string name()
    {
        EventLog::instance().record("TypeName::name (double)");
        return "double";
    }
};

TEST_F(TemplateSpecializationTest, FullTemplateSpecialization)
{
    std::string int_name = TypeName<int>::name();
    std::string double_name = TypeName<double>::name();
    std::string float_name = TypeName<float>::name();

    EXPECT_EQ(int_name, "int");
    EXPECT_EQ(double_name, "double");
    EXPECT_EQ(float_name, "unknown");

    // Q: Full specialization provides a completely different implementation. How many
    // Q: TypeName classes are instantiated?
    // A:
    // R:

    // Q: What happens if you don't provide a specialization for a type?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("TypeName::name (int)"), 1);
    EXPECT_EQ(EventLog::instance().count_events("TypeName::name (generic)"), 1);
}

// ============================================================================
// TEST 2: Partial Template Specialization - Moderate
// ============================================================================

template<typename T>
class Container
{
public:
    void add(T value)
    {
        EventLog::instance().record("Container::add (generic)");
        data_.push_back(value);
    }

    size_t size() const { return data_.size(); }

private:
    std::vector<T> data_;
};

template<typename T>
class Container<T*>
{
public:
    void add(T* value)
    {
        EventLog::instance().record("Container::add (pointer)");
        data_.push_back(value);
    }

    size_t size() const { return data_.size(); }

private:
    std::vector<T*> data_;
};

TEST_F(TemplateSpecializationTest, PartialTemplateSpecialization)
{
    Container<int> int_container;
    int_container.add(42);
    int_container.add(100);

    int x = 10, y = 20;
    Container<int*> ptr_container;
    ptr_container.add(&x);
    ptr_container.add(&y);

    EXPECT_EQ(int_container.size(), 2);
    EXPECT_EQ(ptr_container.size(), 2);

    // Q: Container<int*> uses the pointer specialization. What pattern does the compiler
    // Q: match to select this specialization?
    // A:
    // R:

    // Q: Partial specialization allows specializing on patterns (e.g., T*, const T).
    // Q: Can you partially specialize function templates?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Container::add (generic)"), 2);
    EXPECT_EQ(EventLog::instance().count_events("Container::add (pointer)"), 2);
}

// ============================================================================
// TEST 3: Specialization for const and reference types - Moderate
// ============================================================================

template<typename T>
class TypeTraits
{
public:
    static constexpr bool is_const = false;
    static constexpr bool is_reference = false;
    static constexpr bool is_pointer = false;
};

template<typename T>
class TypeTraits<const T>
{
public:
    static constexpr bool is_const = true;
    static constexpr bool is_reference = false;
    static constexpr bool is_pointer = false;
};

template<typename T>
class TypeTraits<T&>
{
public:
    static constexpr bool is_const = false;
    static constexpr bool is_reference = true;
    static constexpr bool is_pointer = false;
};

template<typename T>
class TypeTraits<T*>
{
public:
    static constexpr bool is_const = false;
    static constexpr bool is_reference = false;
    static constexpr bool is_pointer = true;
};

TEST_F(TemplateSpecializationTest, SpecializationForConstAndReference)
{
    EXPECT_FALSE(TypeTraits<int>::is_const);
    EXPECT_TRUE(TypeTraits<const int>::is_const);
    EXPECT_TRUE(TypeTraits<int&>::is_reference);
    EXPECT_TRUE(TypeTraits<int*>::is_pointer);

    // Q: TypeTraits<const int> matches the const T specialization. What is T in this case?
    // A:
    // R:

    // Q: Can you specialize for const T& (both const and reference)?
    // A:
    // R:

    // Q: How does this relate to std::is_const, std::is_reference from <type_traits>?
    // A:
    // R:
}

// ============================================================================
// TEST 4: Function Template Specialization - Moderate
// ============================================================================

template<typename T>
void print_type(T value)
{
    EventLog::instance().record("print_type: generic");
}

template<>
void print_type<int>(int value)
{
    EventLog::instance().record("print_type: int specialization");
}

template<>
void print_type<const char*>(const char* value)
{
    EventLog::instance().record("print_type: const char* specialization");
}

TEST_F(TemplateSpecializationTest, FunctionTemplateSpecialization)
{
    print_type(42);
    print_type(3.14);
    print_type("hello");

    // Q: Function templates can be fully specialized but not partially specialized.
    // Q: Why does this limitation exist?
    // A:
    // R:

    // Q: What alternative to partial specialization can you use for functions?
    // Q: (Hint: overloading, SFINAE)
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("print_type: int specialization"), 1);
    EXPECT_EQ(EventLog::instance().count_events("print_type: const char* specialization"), 1);
    EXPECT_EQ(EventLog::instance().count_events("print_type: generic"), 1);
}

// ============================================================================
// TEST 5: TODO - Implement Specialization for std::vector - Moderate
// ============================================================================

// TODO: Implement a Serializer template that:
// TODO: 1. Generic version serializes to string using std::to_string
// TODO: 2. Specialization for std::vector serializes each element
// TODO: 3. Specialization for std::string returns the string as-is

TEST_F(TemplateSpecializationTest, DISABLED_SpecializationForStdVector)
{
    // TODO: Implement Serializer template
    // TODO: Test with int, vector<int>, string

    // Q: How does specialization for std::vector<T> differ from full specialization?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Specialization Resolution Order - Hard
// ============================================================================

template<typename T>
class Selector
{
public:
    static std::string select()
    {
        EventLog::instance().record("Selector: primary template");
        return "primary";
    }
};

template<typename T>
class Selector<T*>
{
public:
    static std::string select()
    {
        EventLog::instance().record("Selector: pointer specialization");
        return "pointer";
    }
};

template<typename T>
class Selector<const T*>
{
public:
    static std::string select()
    {
        EventLog::instance().record("Selector: const pointer specialization");
        return "const_pointer";
    }
};

TEST_F(TemplateSpecializationTest, SpecializationResolutionOrder)
{
    std::string result1 = Selector<int>::select();
    std::string result2 = Selector<int*>::select();
    std::string result3 = Selector<const int*>::select();

    EXPECT_EQ(result1, "primary");
    EXPECT_EQ(result2, "pointer");
    EXPECT_EQ(result3, "const_pointer");

    // Q: When multiple specializations match, which one is selected? What is the
    // Q: "most specialized" rule?
    // A:
    // R:

    // Q: If you call Selector<const int>, which specialization is used?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Selector: primary template"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Selector: pointer specialization"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Selector: const pointer specialization"), 1);
}

// ============================================================================
// TEST 7: Specialization for Multiple Parameters - Hard
// ============================================================================

template<typename T1, typename T2>
class Pair
{
public:
    static std::string type()
    {
        EventLog::instance().record("Pair: generic");
        return "generic";
    }
};

template<typename T>
class Pair<T, T>
{
public:
    static std::string type()
    {
        EventLog::instance().record("Pair: same type");
        return "same_type";
    }
};

template<typename T>
class Pair<T, int>
{
public:
    static std::string type()
    {
        EventLog::instance().record("Pair: second is int");
        return "second_int";
    }
};

template<>
class Pair<int, int>
{
public:
    static std::string type()
    {
        EventLog::instance().record("Pair: full specialization int, int");
        return "full_int_int";
    }
};

TEST_F(TemplateSpecializationTest, SpecializationForMultipleParameters)
{
    std::string result1 = Pair<int, double>::type();
    std::string result2 = Pair<int, int>::type();
    std::string result3 = Pair<double, int>::type();
    std::string result4 = Pair<float, float>::type();

    EXPECT_EQ(result1, "generic");
    EXPECT_EQ(result2, "full_int_int");
    EXPECT_EQ(result3, "second_int");
    EXPECT_EQ(result4, "same_type");

    // Q: Pair<int, int> matches "same type", "second is int", and full specialization.
    // Q: Which one is selected and why?
    // A:
    // R:

    // Q: Full specialization has higher priority than partial specialization. What is
    // Q: the resolution order: primary -> partial -> full?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Pair: generic"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Pair: same type"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Pair: second is int"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Pair: full specialization int, int"), 1);
}
