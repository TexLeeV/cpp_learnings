// Test Suite: Template Specialization
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <string>
#include <vector>

class SpecializationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Full Specialization (Easy)
// ============================================================================

template <typename T> struct TypeName
{
    static std::string name()
    {
        EventLog::instance().record("TypeName: generic");
        return "unknown";
    }
};

template <> struct TypeName<int>
{
    static std::string name()
    {
        EventLog::instance().record("TypeName: int");
        return "int";
    }
};

TEST_F(SpecializationTest, FullSpecializationReplacesPrimary)
{
    EXPECT_EQ(TypeName<int>::name(), "int");
    EXPECT_EQ(TypeName<double>::name(), "unknown");

    // Q: For `TypeName<int>`, which definition runs, and what EventLog tag proves the
    //    primary template was not used?
    // A:
    // R:

    // Q: For `TypeName<double>`, why does the primary template still apply?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("TypeName: int"), 1);
    EXPECT_EQ(EventLog::instance().count_events("TypeName: generic"), 1);
}

// ============================================================================
// Scenario 2: Partial Specialization on Pointer Pattern (Moderate)
// ============================================================================

template <typename T> class Holder
{
public:
    void add(T value)
    {
        EventLog::instance().record("Holder: generic");
        data_.push_back(value);
    }
    std::size_t size() const { return data_.size(); }

private:
    std::vector<T> data_;
};

template <typename T> class Holder<T*>
{
public:
    void add(T* value)
    {
        EventLog::instance().record("Holder: pointer");
        data_.push_back(value);
    }
    std::size_t size() const { return data_.size(); }

private:
    std::vector<T*> data_;
};

TEST_F(SpecializationTest, PartialSpecializationMatchesPointerPattern)
{
    Holder<int> values;
    values.add(42);

    int x = 10;
    Holder<int*> pointers;
    pointers.add(&x);

    EXPECT_EQ(values.size(), 1u);
    EXPECT_EQ(pointers.size(), 1u);

    // Q: Which pattern match selects `Holder<T*>` for `Holder<int*>`, and what is `T`
    //    in that match?
    // A:
    // R:

    // Q: Which EventLog tags confirm generic vs pointer specializations both ran once?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Holder: generic"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Holder: pointer"), 1);
}

// ============================================================================
// Scenario 3: Most Specialized Wins (Moderate)
// ============================================================================

template <typename T> struct Selector
{
    static std::string select()
    {
        EventLog::instance().record("Selector: primary");
        return "primary";
    }
};

template <typename T> struct Selector<T*>
{
    static std::string select()
    {
        EventLog::instance().record("Selector: pointer");
        return "pointer";
    }
};

template <typename T> struct Selector<const T*>
{
    static std::string select()
    {
        EventLog::instance().record("Selector: const_pointer");
        return "const_pointer";
    }
};


TEST_F(SpecializationTest, MostSpecializedSpecializationWins)
{
    EXPECT_EQ(Selector<int>::select(), "primary");
    EXPECT_EQ(Selector<int*>::select(), "pointer");
    EXPECT_EQ(Selector<const int*>::select(), "const_pointer");

    // Q: Both `Selector<T*>` and `Selector<const T*>` could describe `const int*`.
    //    Which one is chosen, and what rule decides that?
    // A:
    // R:

    // Q: Hypothetical: if you queried `Selector<const int>`, which specialization
    //    would run, and why?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Selector: primary"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Selector: pointer"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Selector: const_pointer"), 1);
}
