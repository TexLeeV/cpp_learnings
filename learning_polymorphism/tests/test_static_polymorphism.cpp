// Test Suite: Static Polymorphism
// Estimated Time: 1-2 hours
// Difficulty: Moderate / Hard
// C++ Standard: C++20

#include "instrumentation.h"

#include <concepts>
#include <gtest/gtest.h>
#include <string>
#include <variant>
#include <vector>

class StaticPolymorphismTest : public ::testing::Test
{
protected:
    void SetUp() override { EventLog::instance().clear(); }
};

template <typename Derived> class Greeter
{
public:
    std::string greet() const
    {
        EventLog::instance().record("Greeter::greet");
        return static_cast<const Derived*>(this)->greet_impl();
    }
};

class English : public Greeter<English>
{
public:
    std::string greet_impl() const
    {
        EventLog::instance().record("English::greet_impl");
        return "hello";
    }
};

class Spanish : public Greeter<Spanish>
{
public:
    std::string greet_impl() const
    {
        EventLog::instance().record("Spanish::greet_impl");
        return "hola";
    }
};

// ============================================================================
// Scenario 1: CRTP Static Dispatch (Easy / Moderate)
// ============================================================================

TEST_F(StaticPolymorphismTest, CrtpStaticallyDispatchesToDerived)
{
    English e;
    Spanish s;
    EXPECT_EQ(e.greet(), "hello");
    EXPECT_EQ(s.greet(), "hola");

    // Q: Greeter<English> and Greeter<Spanish> are different types. What does
    //    that buy vs. a virtual Greeter for (a) call cost and (b) one container?
    // A:
    // R:

    // Q: What compile-time cast inside Greeter::greet selects greet_impl
    //    without a vtable?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Greeter::greet"), 2);
    EXPECT_EQ(EventLog::instance().count_events("English::greet_impl"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Spanish::greet_impl"), 1);
}

// ============================================================================
// Scenario 2: variant + visit Closed-Set Dispatch (Moderate)
// ============================================================================

struct VCircle
{
    double r;
    double area() const
    {
        EventLog::instance().record("VCircle::area");
        return 3.14159 * r * r;
    }
};

struct VSquare
{
    double side;
    double area() const
    {
        EventLog::instance().record("VSquare::area");
        return side * side;
    }
};

using VShape = std::variant<VCircle, VSquare>;

TEST_F(StaticPolymorphismTest, VariantVisitDispatchesByActiveAlternative)
{
    std::vector<VShape> shapes{VCircle{1.0}, VSquare{2.0}, VCircle{3.0}};

    double total = 0.0;
    for (const auto& s : shapes)
        total += std::visit([](const auto& v) { return v.area(); }, s);

    // Q: What EventLog signal confirms two different paths ran through one
    //    std::visit call site?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("VCircle::area"), 2);
    EXPECT_EQ(EventLog::instance().count_events("VSquare::area"), 1);
    EXPECT_NEAR(total, 3.14159 * 1.0 + 4.0 + 3.14159 * 9.0, 1e-6);

    // Q: How does closed-set variant<VCircle, VSquare> differ from open-set
    //    unique_ptr<Shape> for who can extend the system?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Concepts Constrain Static Polymorphism (Moderate / Hard)
// ============================================================================

template <typename T>
concept Areal = requires(const T t) {
    { t.area() } -> std::convertible_to<double>;
};

template <Areal T> double total_area(const std::vector<T>& items)
{
    double sum = 0.0;
    for (const auto& it : items)
        sum += it.area();
    return sum;
}

struct VTriangle
{
    double b;
    double h;
    double area() const
    {
        EventLog::instance().record("VTriangle::area");
        return 0.5 * b * h;
    }
};

struct NotAShape
{
};

TEST_F(StaticPolymorphismTest, ConceptsConstrainStaticPolymorphism)
{
    std::vector<VTriangle> tris{{2.0, 3.0}, {4.0, 5.0}};
    EXPECT_NEAR(total_area(tris), 0.5 * 2.0 * 3.0 + 0.5 * 4.0 * 5.0, 1e-6);
    EXPECT_EQ(EventLog::instance().count_events("VTriangle::area"), 2);

    // Q: How does Areal differ from a virtual IShape::area in (a) when the
    //    contract is checked and (b) what the call site demands?
    // A:
    // R:

    static_assert(Areal<VTriangle>);
    static_assert(!Areal<NotAShape>);

    // Q: What compile-time failure would total_area(vector<NotAShape>{})
    //    produce?
    // A:
    // R:
}
