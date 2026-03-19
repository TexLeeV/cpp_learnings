// Test Suite: std::optional, std::variant, std::any
// Estimated Time: 3 hours
// Difficulty: Moderate
// C++17

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <optional>
#include <variant>
#include <any>
#include <string>
#include <memory>

class OptionalVariantAnyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: std::optional Basics (Easy)
// ============================================================================

std::optional<int> find_value(bool should_find)
{
    if (should_find)
    {
        return 42;
    }
    return std::nullopt;
}

TEST_F(OptionalVariantAnyTest, OptionalBasics)
{
    // Q: What does std::optional represent?
    // A:
    // R:
    
    auto result = find_value(true);
    
    // Q: How do we check if an optional has a value?
    // A:
    // R:
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
    EXPECT_EQ(result.value(), 42);
    
    auto empty = find_value(false);
    
    EXPECT_FALSE(empty.has_value());
    
    // Q: What happens if we call value() on an empty optional?
    // A:
    // R:
    
    // Q: What is the difference between *opt and opt.value()?
    // A:
    // R:
    
    int default_val = empty.value_or(100);
    EXPECT_EQ(default_val, 100);
}

// ============================================================================
// Scenario 2: std::optional with Tracked Objects (Moderate)
// ============================================================================

std::optional<Tracked> make_optional_tracked(bool should_create)
{
    if (should_create)
    {
        return Tracked("Optional");
    }
    return std::nullopt;
}

TEST_F(OptionalVariantAnyTest, OptionalWithTracked)
{
    // Q: How many Tracked objects are constructed when returning from make_optional_tracked?
    // A:
    // R:
    
    auto opt = make_optional_tracked(true);
    
    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(opt->name(), "Optional");
    
    // Q: What observable signal shows copy vs move construction?
    // A:
    // R:
    
    EventLog::instance().clear();
    
    // TODO: Reset the optional
    opt.reset();
    
    // Q: What happens to the Tracked object when reset() is called?
    // A:
    // R:
    
    EXPECT_FALSE(opt.has_value());
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}

// ============================================================================
// Scenario 3: std::variant Basics (Moderate)
// ============================================================================

TEST_F(OptionalVariantAnyTest, VariantBasics)
{
    // Q: What is std::variant?
    // A:
    // R:
    
    std::variant<int, double, std::string> var;
    
    // Q: What is the default value of var?
    // A:
    // R:
    
    EXPECT_EQ(var.index(), 0);
    EXPECT_EQ(std::get<int>(var), 0);
    
    var = 3.14;
    
    // Q: What is var.index() now?
    // A:
    // R:
    
    EXPECT_EQ(var.index(), 1);
    EXPECT_EQ(std::get<double>(var), 3.14);
    
    var = "hello";
    
    EXPECT_EQ(var.index(), 2);
    EXPECT_EQ(std::get<std::string>(var), "hello");
    
    // Q: What happens if we call std::get<int>(var) when var holds a string?
    // A:
    // R:
}

// ============================================================================
// Scenario 4: std::variant with std::visit (Hard)
// ============================================================================

TEST_F(OptionalVariantAnyTest, VariantWithVisit)
{
    std::variant<int, double, std::string> var = 42;
    
    // Q: What is std::visit?
    // A:
    // R:
    
    auto visitor = [](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>)
        {
            EventLog::instance().record("Visited int: " + std::to_string(arg));
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            EventLog::instance().record("Visited double: " + std::to_string(arg));
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            EventLog::instance().record("Visited string: " + arg);
        }
    };
    
    std::visit(visitor, var);
    
    EXPECT_EQ(EventLog::instance().count_events("Visited int"), 1);
    
    var = 3.14;
    std::visit(visitor, var);
    
    EXPECT_EQ(EventLog::instance().count_events("Visited double"), 1);
    
    // Q: What advantage does std::visit have over std::get?
    // A:
    // R:
}

// ============================================================================
// Scenario 5: std::any Basics (Moderate)
// ============================================================================

TEST_F(OptionalVariantAnyTest, AnyBasics)
{
    // Q: What is the difference between std::variant and std::any?
    // A:
    // R:
    
    std::any a = 42;
    
    EXPECT_TRUE(a.has_value());
    
    // Q: How do we extract the value from std::any?
    // A:
    // R:
    
    int value = std::any_cast<int>(a);
    EXPECT_EQ(value, 42);
    
    a = std::string("hello");
    
    // Q: What happens if we call std::any_cast<int>(a) when a holds a string?
    // A:
    // R:
    
    std::string str = std::any_cast<std::string>(a);
    EXPECT_EQ(str, "hello");
    
    // Q: What is the performance cost of std::any compared to std::variant?
    // A:
    // R:
}

// ============================================================================
// Scenario 6: std::any with Tracked Objects (Hard)
// ============================================================================

TEST_F(OptionalVariantAnyTest, AnyWithTracked)
{
    std::any a = Tracked("AnyTracked");
    
    // Q: How many Tracked objects exist at this point?
    // A:
    // R:
    
    EXPECT_TRUE(a.has_value());
    
    // TODO: Extract the Tracked object
    Tracked extracted = std::any_cast<Tracked>(a);
    
    // Q: How many copy operations occurred during any_cast?
    // A:
    // R:
    
    EXPECT_EQ(extracted.name(), "AnyTracked");
    
    EventLog::instance().clear();
    
    a.reset();
    
    // Q: What happens to the Tracked object when any is reset?
    // A:
    // R:
    
    EXPECT_FALSE(a.has_value());
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}
