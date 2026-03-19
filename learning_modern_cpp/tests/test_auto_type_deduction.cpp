// Test Suite: auto and Type Deduction Rules
// Estimated Time: 2 hours
// Difficulty: Easy
// C++11/14

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <vector>
#include <map>
#include <memory>
#include <type_traits>

class AutoTypeDeductionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Basic auto Deduction (Easy)
// ============================================================================

TEST_F(AutoTypeDeductionTest, BasicAutoDeduction)
{
    // Q: What type is x?
    // A:
    // R:
    
    auto x = 42;
    static_assert(std::is_same<decltype(x), int>::value, "x should be int");
    
    // Q: What type is y?
    // A:
    // R:
    
    auto y = 3.14;
    static_assert(std::is_same<decltype(y), double>::value, "y should be double");
    
    // Q: What type is ptr?
    // A:
    // R:
    
    auto ptr = std::make_shared<Tracked>("Auto");
    static_assert(std::is_same<decltype(ptr), std::shared_ptr<Tracked>>::value, 
                  "ptr should be shared_ptr<Tracked>");
    
    EXPECT_EQ(ptr->name(), "Auto");
}

// ============================================================================
// Scenario 2: auto with References (Moderate)
// ============================================================================

TEST_F(AutoTypeDeductionTest, AutoWithReferences)
{
    int value = 10;
    int& ref = value;
    
    // Q: What type is x (auto from reference)?
    // A:
    // R:
    
    auto x = ref;
    static_assert(std::is_same<decltype(x), int>::value, "x should be int, not int&");
    
    x = 20;
    
    // Q: What is the value of 'value' after x = 20?
    // A:
    // R:
    
    EXPECT_EQ(value, 10);
    EXPECT_EQ(x, 20);
    
    // Q: How do we preserve the reference with auto?
    // A:
    // R:
    
    auto& y = ref;
    static_assert(std::is_same<decltype(y), int&>::value, "y should be int&");
    
    y = 30;
    
    EXPECT_EQ(value, 30);
}

// ============================================================================
// Scenario 3: auto with const (Moderate)
// ============================================================================

TEST_F(AutoTypeDeductionTest, AutoWithConst)
{
    const int const_value = 42;
    
    // Q: What type is x?
    // A:
    // R:
    
    auto x = const_value;
    static_assert(std::is_same<decltype(x), int>::value, "x should be int, not const int");
    
    x = 100;  // This compiles
    
    // Q: Why does auto drop const?
    // A:
    // R:
    
    // Q: How do we preserve const with auto?
    // A:
    // R:
    
    const auto y = const_value;
    static_assert(std::is_same<decltype(y), const int>::value, "y should be const int");
    
    // y = 100;  // This would fail to compile
    
    // Q: What type is z?
    // A:
    // R:
    
    auto& z = const_value;
    static_assert(std::is_same<decltype(z), const int&>::value, "z should be const int&");
}

// ============================================================================
// Scenario 4: decltype Basics (Moderate)
// ============================================================================

int global_func()
{
    return 42;
}

TEST_F(AutoTypeDeductionTest, DecltypeBasics)
{
    int x = 10;
    
    // Q: What is the difference between auto and decltype?
    // A:
    // R:
    
    decltype(x) y = 20;
    static_assert(std::is_same<decltype(y), int>::value, "y should be int");
    
    // Q: What type is result?
    // A:
    // R:
    
    decltype(global_func()) result = global_func();
    static_assert(std::is_same<decltype(result), int>::value, "result should be int");
    
    // Q: What type is ref_type?
    // A:
    // R:
    
    int& ref = x;
    decltype(ref) ref_type = x;
    static_assert(std::is_same<decltype(ref_type), int&>::value, "ref_type should be int&");
    
    ref_type = 30;
    EXPECT_EQ(x, 30);
}

// ============================================================================
// Scenario 5: decltype with Expressions (Hard)
// ============================================================================

TEST_F(AutoTypeDeductionTest, DecltypeWithExpressions)
{
    int x = 10;
    
    // Q: What type is decltype(x)?
    // A:
    // R:
    
    static_assert(std::is_same<decltype(x), int>::value, "decltype(x) is int");
    
    // Q: What type is decltype((x))?
    // A:
    // R:
    
    static_assert(std::is_same<decltype((x)), int&>::value, "decltype((x)) is int&");
    
    // Q: Why does adding parentheses change the deduced type?
    // A:
    // R:
    
    // Q: What type is decltype(x + 1)?
    // A:
    // R:
    
    static_assert(std::is_same<decltype(x + 1), int>::value, "decltype(x + 1) is int");
}

// ============================================================================
// Scenario 6: auto with Iterators (Easy)
// ============================================================================

TEST_F(AutoTypeDeductionTest, AutoWithIterators)
{
    std::vector<Tracked> vec;
    vec.emplace_back("V1");
    vec.emplace_back("V2");
    vec.emplace_back("V3");
    
    EventLog::instance().clear();
    
    // Q: What is the advantage of auto for iterator types?
    // A:
    // R:
    
    // TODO: Use auto to iterate through vec
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        EventLog::instance().record("Iterator: " + it->name());
    }
    
    EXPECT_EQ(EventLog::instance().count_events("Iterator:"), 3);
    
    // Q: What type is elem in this range-based for loop?
    // A:
    // R:
    
    EventLog::instance().clear();
    
    for (auto elem : vec)
    {
        EventLog::instance().record("Element: " + elem.name());
    }
    
    // Q: How many copy constructions occurred in the range-based for loop?
    // A:
    // R:
    
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 3);
    
    // Q: How can we avoid the copies?
    // A:
    // R:
    
    EventLog::instance().clear();
    
    for (const auto& elem : vec)
    {
        EventLog::instance().record("Ref: " + elem.name());
    }
    
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 0);
}

// ============================================================================
// Scenario 7: auto Return Type Deduction (C++14) (Moderate)
// ============================================================================

auto make_tracked(const std::string& name)
{
    return std::make_shared<Tracked>(name);
}

TEST_F(AutoTypeDeductionTest, AutoReturnTypeDeduction)
{
    // Q: What is the return type of make_tracked?
    // A:
    // R:
    
    auto ptr = make_tracked("AutoReturn");
    
    static_assert(std::is_same<decltype(ptr), std::shared_ptr<Tracked>>::value, 
                  "Should be shared_ptr<Tracked>");
    
    EXPECT_EQ(ptr->name(), "AutoReturn");
    
    // Q: What limitation does auto return type have with multiple return statements?
    // A:
    // R:
}
