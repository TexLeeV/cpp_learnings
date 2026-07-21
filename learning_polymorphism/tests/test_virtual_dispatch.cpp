// Test Suite: Virtual Dispatch
// Estimated Time: 1-2 hours
// Difficulty: Easy
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <string>

class VirtualDispatchTest : public ::testing::Test
{
protected:
    void SetUp() override { EventLog::instance().clear(); }
};

class Shape
{
public:
    virtual ~Shape() = default;

    virtual std::string name() const
    {
        EventLog::instance().record("Shape::name");
        return "Shape";
    }

    // Non-virtual on purpose: see StaticDispatchHidesDerived.
    std::string label() const
    {
        EventLog::instance().record("Shape::label");
        return "label:Shape";
    }
};

class Circle : public Shape
{
public:
    std::string name() const override
    {
        EventLog::instance().record("Circle::name");
        return "Circle";
    }

    std::string label() const  // hides, does not override
    {
        EventLog::instance().record("Circle::label");
        return "label:Circle";
    }
};

// ============================================================================
// Scenario 1: Dynamic Dispatch Through a Base Reference (Easy)
// ============================================================================

TEST_F(VirtualDispatchTest, DynamicDispatchSelectsDerivedOverride)
{
    Circle c;
    Shape& base_ref = c;
    const std::string n = base_ref.name();

    // Q: Which override runs, and what EventLog entry confirms it?
    // A:
    // R:

    EXPECT_EQ(n, "Circle");
    EXPECT_EQ(EventLog::instance().count_events("Circle::name"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Shape::name"), 0);
}

// ============================================================================
// Scenario 2: Static Dispatch Hides Derived Behavior (Easy)
// ============================================================================

TEST_F(VirtualDispatchTest, StaticDispatchHidesDerived)
{
    Circle c;
    Shape& base_ref = c;
    const std::string lbl = base_ref.label();

    // Q: Why does this call Shape::label even though the dynamic type is Circle?
    // A:
    // R:

    EXPECT_EQ(lbl, "label:Shape");
    EXPECT_EQ(EventLog::instance().count_events("Shape::label"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Circle::label"), 0);

    // Q: What single keyword on Shape::label would honor the dynamic type?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Object Slicing on Pass-By-Value (Moderate)
// ============================================================================

static std::string call_name_by_value(Shape s) { return s.name(); }
static std::string call_name_by_ref(const Shape& s) { return s.name(); }

TEST_F(VirtualDispatchTest, SlicingOnPassByValue)
{
    Circle c;

    EventLog::instance().clear();
    const std::string sliced = call_name_by_value(c);

    // Q: Why does call_name_by_value reach Shape::name?
    // A:
    // R:

    EXPECT_EQ(sliced, "Shape");
    EXPECT_EQ(EventLog::instance().count_events("Shape::name"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Circle::name"), 0);

    EventLog::instance().clear();
    const std::string preserved = call_name_by_ref(c);

    // Q: What structural change in call_name_by_ref preserves Circle::name?
    // A:
    // R:

    EXPECT_EQ(preserved, "Circle");
    EXPECT_EQ(EventLog::instance().count_events("Circle::name"), 1);
}

// ============================================================================
// Scenario 4: Mismatched Signature Fails to Override (Hard)
// ============================================================================

class Animal
{
public:
    virtual ~Animal() = default;

    virtual std::string speak() const
    {
        EventLog::instance().record("Animal::speak");
        return "...";
    }
};

class Dog : public Animal
{
public:
    // Drops `const` on purpose: hides rather than overrides.
    std::string speak()
    {
        EventLog::instance().record("Dog::speak");
        return "woof";
    }
};

TEST_F(VirtualDispatchTest, MismatchedSignatureSilentlyFailsToOverride)
{
    Dog d;
    Animal& a = d;
    const std::string sound = a.speak();

    // Q: Why does this hit Animal::speak? What EventLog signal proves it?
    // A:
    // R:

    EXPECT_EQ(sound, "...");
    EXPECT_EQ(EventLog::instance().count_events("Animal::speak"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Dog::speak"), 0);

    // Q: What keyword on Dog::speak would turn this into a compile error?
    // A:
    // R:
}
