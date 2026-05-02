// Test Suite: Virtual Dispatch
// Estimated Time: 2 hours
// Difficulty: Easy
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class VirtualDispatchTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Type hierarchy used across scenarios
// ============================================================================

class Shape
{
public:
    virtual ~Shape() = default;

    // Q: What changes about the call site if `name()` is virtual vs. non-virtual?
    // A:
    // R:
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

    // Hides Shape::label rather than overriding it (label is non-virtual).
    std::string label() const
    {
        EventLog::instance().record("Circle::label");
        return "label:Circle";
    }
};

class Square final : public Shape
{
public:
    std::string name() const override
    {
        EventLog::instance().record("Square::name");
        return "Square";
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

    // Q: Which override runs here, and what observable signal in EventLog confirms it?
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

    // `label()` is non-virtual: dispatch is decided at compile time from the
    // static type of the expression on the left of `.`.
    const std::string lbl = base_ref.label();

    // Q: Why does this call `Shape::label` even though the dynamic type is Circle?
    // A:
    // R:

    EXPECT_EQ(lbl, "label:Shape");
    EXPECT_EQ(EventLog::instance().count_events("Shape::label"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Circle::label"), 0);

    // Q: What single keyword on `Shape::label` would make this test fail by
    //    causing dispatch to honor the dynamic type instead?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Object Slicing on Pass-By-Value (Moderate)
// ============================================================================

// Pass-by-value parameter: forces a copy into a `Shape`-typed object,
// destroying any derived portion before `name()` is even called.
static std::string call_name_by_value(Shape s)
{
    return s.name();
}

static std::string call_name_by_ref(const Shape& s)
{
    return s.name();
}

TEST_F(VirtualDispatchTest, SlicingOnPassByValue)
{
    Circle c;

    EventLog::instance().clear();
    const std::string sliced = call_name_by_value(c);

    // Q: What does EventLog show happened to the Circle on the way into
    //    call_name_by_value, and why does that determine the result?
    // A:
    // R:

    EXPECT_EQ(sliced, "Shape");
    EXPECT_EQ(EventLog::instance().count_events("Shape::name"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Circle::name"), 0);

    EventLog::instance().clear();
    const std::string preserved = call_name_by_ref(c);

    // Q: What changed structurally between these two calls that lets the
    //    second one reach Circle::name?
    // A:
    // R:

    EXPECT_EQ(preserved, "Circle");
    EXPECT_EQ(EventLog::instance().count_events("Circle::name"), 1);
}

// ============================================================================
// Scenario 4: Polymorphism Through a Container of Base Pointers (Moderate)
// ============================================================================

TEST_F(VirtualDispatchTest, HeterogeneousContainerDispatch)
{
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>());
    shapes.push_back(std::make_unique<Square>());
    shapes.push_back(std::make_unique<Shape>());

    EventLog::instance().clear();

    std::vector<std::string> names;
    for (const auto& s : shapes)
    {
        names.push_back(s->name());
    }

    // Q: Why does iterating over `unique_ptr<Shape>` still reach Circle::name
    //    and Square::name, when slicing destroyed it in the previous scenario?
    // A:
    // R:

    ASSERT_EQ(names.size(), 3u);
    EXPECT_EQ(names[0], "Circle");
    EXPECT_EQ(names[1], "Square");
    EXPECT_EQ(names[2], "Shape");

    EXPECT_EQ(EventLog::instance().count_events("Circle::name"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Square::name"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Shape::name"), 1);
}

// ============================================================================
// Scenario 5: `final` Prevents Further Override (Hard)
// ============================================================================

// TODO (learner): Try to declare `class Hexagon : public Square { ... };` and
// override `name()`. The compiler should reject it. Explain in `// A:` below
// what diagnostic you expect and why `final` made it impossible.
//
// // class Hexagon : public Square
// // {
// // public:
// //     std::string name() const override { return "Hexagon"; }
// // };

TEST_F(VirtualDispatchTest, FinalDerivedStillDispatchesDynamically)
{
    std::unique_ptr<Shape> s = std::make_unique<Square>();

    // Q: `Square` is `final`. Does that change how dispatch is performed
    //    through `Shape*`, or only what derivations are allowed below Square?
    // A:
    // R:

    EXPECT_EQ(s->name(), "Square");
    EXPECT_EQ(EventLog::instance().count_events("Square::name"), 1);
}

// ============================================================================
// Scenario 6: Override-Only-In-Theory (Hard)
// ============================================================================

class Animal
{
public:
    virtual ~Animal() = default;

    // Note the `const`-qualified signature.
    virtual std::string speak() const
    {
        EventLog::instance().record("Animal::speak");
        return "...";
    }
};

class Dog : public Animal
{
public:
    // TODO (learner): The signature below intentionally drops `const`, so it
    // does NOT override `Animal::speak`. Add `override` to force a compile
    // error, then fix the signature to match the base. Record what changed
    // in the EventLog counts when you re-run the test.
    std::string speak() /* const */
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

    // Q: Why does this dispatch to `Animal::speak` even though `Dog` defines
    //    a method also named `speak`? What observable signal proves it?
    // A:
    // R:

    EXPECT_EQ(sound, "...");
    EXPECT_EQ(EventLog::instance().count_events("Animal::speak"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Dog::speak"), 0);

    // Q: What single keyword, applied at the Dog::speak declaration, would
    //    have turned this silent bug into a compile error?
    // A:
    // R:
}
