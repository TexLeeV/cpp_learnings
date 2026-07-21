// Test Suite: Variadic Templates
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <string>

class VariadicTemplatesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: sizeof...(pack) (Easy)
// ============================================================================

template <typename... Args> void log_arity(Args...)
{
    EventLog::instance().record("arity=" + std::to_string(sizeof...(Args)));
}

TEST_F(VariadicTemplatesTest, SizeofPackCountsArguments)
{
    log_arity();
    log_arity(1);
    log_arity(1, 2.0, "three");

    // Q: What does `sizeof...(Args)` report for each call, and which EventLog strings
    //    confirm those arities?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("arity=0"), 1);
    EXPECT_EQ(EventLog::instance().count_events("arity=1"), 1);
    EXPECT_EQ(EventLog::instance().count_events("arity=3"), 1);
}

// ============================================================================
// Scenario 2: Fold Expression (Moderate)
// ============================================================================

template <typename... Args> auto sum_fold(Args... args)
{
    EventLog::instance().record("sum_fold");
    return (args + ...);
}

TEST_F(VariadicTemplatesTest, FoldExpressionReducesPack)
{
    EXPECT_EQ(sum_fold(1, 2, 3, 4, 5), 15);
    EXPECT_DOUBLE_EQ(sum_fold(1.5, 2.5), 4.0);

    // Q: What expression does `(args + ...)` expand to for five integers, and how does
    //    that compare to writing a recursive peel-one-argument helper?
    // A:
    // R:

    // Q: What EventLog count confirms both integer and floating fold calls executed?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("sum_fold"), 2);
}

// ============================================================================
// Scenario 3: Recursive Print via EventLog (Moderate)
// ============================================================================

void print_pack()
{
    EventLog::instance().record("print: base");
}

template <typename T, typename... Rest> void print_pack(T first, Rest... rest)
{
    EventLog::instance().record("print: " + std::to_string(first));
    print_pack(rest...);
}

TEST_F(VariadicTemplatesTest, RecursivePackPeelsOneArgument)
{
    print_pack(10, 20, 30);

    // Q: How many recursive instantiations run before the non-template base case, and
    //    which EventLog counts prove that?
    // A:
    // R:

    // Q: Why must the empty-pack base case exist for this peel-one pattern to terminate?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("print: 10"), 1);
    EXPECT_EQ(EventLog::instance().count_events("print: 20"), 1);
    EXPECT_EQ(EventLog::instance().count_events("print: 30"), 1);
    EXPECT_EQ(EventLog::instance().count_events("print: base"), 1);
}

// ============================================================================
// Scenario 4: Variadic Class Template (Easy)
// ============================================================================

template <typename... Types> struct TypeList
{
    TypeList()
    {
        EventLog::instance().record("TypeList::size=" + std::to_string(sizeof...(Types)));
    }

    static constexpr std::size_t size()
    {
        return sizeof...(Types);
    }
};

TEST_F(VariadicTemplatesTest, VariadicClassHoldsTypePack)
{
    TypeList<> empty;
    TypeList<int, double, std::string> triple;

    EXPECT_EQ(empty.size(), 0u);
    EXPECT_EQ(triple.size(), 3u);

    // Q: What does `sizeof...(Types)` mean for a class template parameter pack, and
    //    which EventLog strings confirm empty vs triple packs?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("TypeList::size=0"), 1);
    EXPECT_EQ(EventLog::instance().count_events("TypeList::size=3"), 1);
}
