// Test Suite: std::string_view
// Estimated Time: 2 hours
// Difficulty: Easy

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <vector>

class StringViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: string_view Basics (Easy)
// ============================================================================

TEST_F(StringViewTest, BasicStringView)
{
    // Q: What is std::string_view?
    // A:
    // R:

    std::string str = "Hello, World!";
    std::string_view sv = str;

    EXPECT_EQ(sv.size(), 13);
    EXPECT_EQ(sv, "Hello, World!");

    // Q: Does string_view own the string data?
    // A:
    // R:

    // Q: What happens if we modify str?
    // A:
    // R:

    str[0] = 'h';

    EXPECT_EQ(sv[0], 'h');
}

// ============================================================================
// Scenario 2: string_view from Literals (Easy)
// ============================================================================

TEST_F(StringViewTest, StringViewFromLiterals)
{
    // Q: What is the advantage of string_view over const string& for string literals?
    // A:
    // R:

    std::string_view sv = "literal";

    EXPECT_EQ(sv.size(), 7);

    // Q: Does creating string_view from a literal allocate memory?
    // A:
    // R:

    const char* literal = "another";
    std::string_view sv2 = literal;

    EXPECT_EQ(sv2, "another");
}

// ============================================================================
// Scenario 3: string_view Lifetime Issues (Hard)
// ============================================================================

std::string_view get_dangerous_view()
{
    std::string temp = "temporary";
    return temp;
}

TEST_F(StringViewTest, LifetimeIssues)
{
    // Q: What is wrong with get_dangerous_view()?
    // A:
    // R:

    // DANGER: Uncommenting this creates undefined behavior
    // auto view = get_dangerous_view();
    // std::string copy(view);  // Undefined behavior

    // Q: What lifetime guarantee does string_view require?
    // A:
    // R:

    std::string safe_str = "safe";
    std::string_view safe_view = safe_str;

    EXPECT_EQ(safe_view, "safe");
}

// ============================================================================
// Scenario 4: string_view Substrings (Moderate)
// ============================================================================

TEST_F(StringViewTest, StringViewSubstrings)
{
    std::string str = "Hello, World!";
    std::string_view sv = str;

    // Q: Does substr() on string_view allocate memory?
    // A:
    // R:

    std::string_view sub = sv.substr(0, 5);

    EXPECT_EQ(sub, "Hello");

    // Q: What does sub point to?
    // A:
    // R:

    EXPECT_EQ(sub.data(), str.data());

    // TODO: Get substring "World"
    std::string_view world = sv.substr(7, 5);

    EXPECT_EQ(world, "World");
}

// ============================================================================
// Scenario 5: string_view Performance (Moderate)
// ============================================================================

void process_string_copy(std::string s)
{
    EventLog::instance().record("process_string_copy: " + s);
}

void process_string_view(std::string_view sv)
{
    EventLog::instance().record("process_string_view: " + std::string(sv));
}

TEST_F(StringViewTest, StringViewPerformance)
{
    std::string str = "performance test";

    EventLog::instance().clear();

    // Q: How many string copies occur when calling process_string_copy?
    // A:
    // R:

    process_string_copy(str);

    // Q: How many string copies occur when calling process_string_view?
    // A:
    // R:

    process_string_view(str);

    // Q: When should you prefer string_view over const string&?
    // A:
    // R:
}

// ============================================================================
// Scenario 6: string_view Modification Restrictions (Easy)
// ============================================================================

TEST_F(StringViewTest, StringViewModificationRestrictions)
{
    std::string str = "mutable";
    std::string_view sv = str;

    // Q: Can we modify the underlying string through string_view?
    // A:
    // R:

    // sv[0] = 'M';  // This would fail to compile

    str[0] = 'M';

    EXPECT_EQ(sv, "Mutable");

    // Q: What operations can string_view perform?
    // A:
    // R:
}
