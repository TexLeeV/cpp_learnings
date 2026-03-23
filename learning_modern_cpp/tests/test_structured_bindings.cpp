// Test Suite: Structured Bindings
// Estimated Time: 2 hours
// Difficulty: Easy


#include "instrumentation.h"
#include <gtest/gtest.h>
#include <tuple>
#include <map>
#include <utility>
#include <string>

class StructuredBindingsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Structured Bindings with Pairs (Easy)
// ============================================================================

std::pair<int, std::string> get_pair()
{
    return {42, "answer"};
}

TEST_F(StructuredBindingsTest, BindingsWithPairs)
{
    // Q: What does auto [a, b] do?
    // A:
    // R:
    
    auto [number, text] = get_pair();
    
    EXPECT_EQ(number, 42);
    EXPECT_EQ(text, "answer");
    
    // Q: What are the types of number and text?
    // A:
    // R:
    
    static_assert(std::is_same<decltype(number), int>::value, "number is int");
    static_assert(std::is_same<decltype(text), std::string>::value, "text is string");
    
    // Q: Are number and text references or copies?
    // A:
    // R:
    
    number = 100;
    auto [n2, t2] = get_pair();
    EXPECT_EQ(n2, 42);
}

// ============================================================================
// Scenario 2: Structured Bindings with References (Moderate)
// ============================================================================

TEST_F(StructuredBindingsTest, BindingsWithReferences)
{
    std::pair<int, std::string> p{10, "test"};
    
    // Q: What does auto& [a, b] do?
    // A:
    // R:
    
    auto& [num, str] = p;
    
    static_assert(std::is_same<decltype(num), int>::value, "num is int");
    static_assert(std::is_same<decltype(str), std::string>::value, "str is string");
    
    num = 20;
    str = "modified";
    
    // Q: What are the values of p.first and p.second?
    // A:
    // R:
    
    EXPECT_EQ(p.first, 20);
    EXPECT_EQ(p.second, "modified");
}

// ============================================================================
// Scenario 3: Structured Bindings with Tuples (Easy)
// ============================================================================

std::tuple<int, double, std::string> get_tuple()
{
    return {1, 2.5, "three"};
}

TEST_F(StructuredBindingsTest, BindingsWithTuples)
{
    // Q: How many variables can we bind from a tuple?
    // A:
    // R:
    
    auto [a, b, c] = get_tuple();
    
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 2.5);
    EXPECT_EQ(c, "three");
    
    // Q: What happens if we try to bind fewer variables than tuple elements?
    // A:
    // R:
    
    // auto [x, y] = get_tuple();  // This would fail to compile
}

// ============================================================================
// Scenario 4: Structured Bindings with Structs (Moderate)
// ============================================================================

struct Point
{
    int x;
    int y;
    int z;
};

Point get_point()
{
    return {10, 20, 30};
}

TEST_F(StructuredBindingsTest, BindingsWithStructs)
{
    // Q: What requirement must a struct meet for structured bindings?
    // A:
    // R:
    
    auto [x, y, z] = get_point();
    
    EXPECT_EQ(x, 10);
    EXPECT_EQ(y, 20);
    EXPECT_EQ(z, 30);
    
    // Q: What is the order of binding: declaration order or initialization order?
    // A:
    // R:
    
    Point p{1, 2, 3};
    auto& [px, py, pz] = p;
    
    px = 100;
    
    EXPECT_EQ(p.x, 100);
}

// ============================================================================
// Scenario 5: Structured Bindings with Maps (Moderate)
// ============================================================================

TEST_F(StructuredBindingsTest, BindingsWithMaps)
{
    std::map<std::string, int> scores;
    scores["Alice"] = 95;
    scores["Bob"] = 87;
    scores["Charlie"] = 92;
    
    // Q: What does auto& [key, value] bind to in a map iteration?
    // A:
    // R:
    
    for (const auto& [name, score] : scores)
    {
        EventLog::instance().record("Score: " + name + " = " + std::to_string(score));
    }
    
    EXPECT_EQ(EventLog::instance().count_events("Score:"), 3);
    
    // Q: Can we modify the key through structured bindings?
    // A:
    // R:
    
    for (auto& [name, score] : scores)
    {
        score += 5;
        // name = "modified";  // This would fail to compile
    }
    
    EXPECT_EQ(scores["Alice"], 100);
}

// ============================================================================
// Scenario 6: Structured Bindings with Tracked Objects (Hard)
// ============================================================================

std::pair<Tracked, Tracked> make_tracked_pair()
{
    return {Tracked("First"), Tracked("Second")};
}

TEST_F(StructuredBindingsTest, BindingsWithTrackedObjects)
{
    EventLog::instance().clear();
    
    // Q: How many Tracked objects are constructed in make_tracked_pair?
    // A:
    // R:
    
    auto [first, second] = make_tracked_pair();
    
    // Q: How many copy or move operations occurred?
    // A:
    // R:
    
    EXPECT_EQ(first.name(), "First");
    EXPECT_EQ(second.name(), "Second");
    
    // Q: What observable signal shows whether copy or move was used?
    // A:
    // R:
}
