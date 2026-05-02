// Test Suite: Static Polymorphism (CRTP, variant + visit, concepts)
// Estimated Time: 3 hours
// Difficulty: Hard
// C++ Standard: C++20

#include "instrumentation.h"

#include <concepts>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

class StaticPolymorphismTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// CRTP base: dispatches at compile time via the derived type parameter.
// ============================================================================

template <typename Derived> class Greeter
{
public:
    std::string greet() const
    {
        EventLog::instance().record("Greeter::greet");
        return self().greet_impl();
    }

private:
    const Derived& self() const
    {
        return static_cast<const Derived&>(*this);
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
// Scenario 1: CRTP Resolves Dispatch Statically (Easy / Moderate)
// ============================================================================

TEST_F(StaticPolymorphismTest, CrtpStaticallyDispatchesToDerived)
{
    English e;
    Spanish s;

    EXPECT_EQ(e.greet(), "hello");
    EXPECT_EQ(s.greet(), "hola");

    // Q: `Greeter<English>` is a *different type* from `Greeter<Spanish>`.
    //    What does that buy you compared to a virtual `Greeter` base, in
    //    terms of (a) call-site cost and (b) ability to put both in the
    //    same container?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Greeter::greet"), 2);
    EXPECT_EQ(EventLog::instance().count_events("English::greet_impl"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Spanish::greet_impl"), 1);
}

// ============================================================================
// Scenario 2: CRTP Cannot Hold Heterogeneous Pointers (Moderate)
// ============================================================================

template <typename G> std::string call_greet(const Greeter<G>& g)
{
    return g.greet();
}

TEST_F(StaticPolymorphismTest, CrtpRequiresKnownTypeAtCompileTime)
{
    English e;
    Spanish s;

    // Each call site instantiates a *different* function template.
    EXPECT_EQ(call_greet(e), "hello");
    EXPECT_EQ(call_greet(s), "hola");

    // Q: Why can you NOT write `std::vector<Greeter<???>*>` to hold both
    //    English and Spanish? Answer in terms of what `Greeter<English>`
    //    and `Greeter<Spanish>` have (or do not have) in common at the
    //    type-system level.
    // A:
    // R:

    // TODO (learner): Try to declare
    //   std::vector<Greeter<English>*> heterogeneous;
    //   heterogeneous.push_back(&s);
    // The compiler should reject the second push_back. Record the
    // diagnostic in `// A:`. This is the price CRTP pays for having no
    // vtable.
}

// ============================================================================
// Scenario 3: std::variant + std::visit Reaches Closed-Set Polymorphism (Moderate)
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
    std::vector<VShape> shapes;
    shapes.emplace_back(VCircle{1.0});
    shapes.emplace_back(VSquare{2.0});
    shapes.emplace_back(VCircle{3.0});

    double total = 0.0;
    for (const auto& s : shapes)
    {
        total += std::visit([](const auto& v) { return v.area(); }, s);
    }

    // Q: `std::visit` and the lambda look like one call site, but the
    //    compiler emits a dispatch table behind the scenes. What signal in
    //    the EventLog confirms that two different code paths actually
    //    ran?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("VCircle::area"), 2);
    EXPECT_EQ(EventLog::instance().count_events("VSquare::area"), 1);
    EXPECT_NEAR(total, 3.14159 * 1.0 + 4.0 + 3.14159 * 9.0, 1e-6);

    // Q: What is the structural difference between the closed set
    //    expressed by `std::variant<VCircle, VSquare>` and the open set
    //    expressed by `std::unique_ptr<Shape>`? Frame at least one
    //    consequence in terms of *who can extend the system*.
    // A:
    // R:
}

// ============================================================================
// Scenario 4: Concepts as a Compile-Time Interface (Moderate / Hard)
// ============================================================================

template <typename T>
concept Areal = requires(const T t) {
    { t.area() } -> std::convertible_to<double>;
};

template <Areal T> double total_area(const std::vector<T>& items)
{
    double sum = 0.0;
    for (const auto& it : items)
    {
        sum += it.area();
    }
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

    // Q: How does the `Areal` concept differ from a virtual
    //    `interface IShape { virtual double area() const = 0; }`? Frame
    //    the answer in terms of (a) when the contract is checked and
    //    (b) what the function signature demands at the call site.
    // A:
    // R:

    // Compile-time check that `NotAShape` does NOT satisfy the concept.
    static_assert(Areal<VTriangle>);
    static_assert(!Areal<NotAShape>);

    // TODO (learner): Uncomment the line below. The compiler should
    // reject it with a constraint-not-satisfied diagnostic. Record the
    // exact error in `// A:`.
    //
    // total_area(std::vector<NotAShape>{NotAShape{}});
}

// ============================================================================
// Scenario 5: Trade-off Between Static and Dynamic Polymorphism (Hard)
// ============================================================================

class IShape
{
public:
    virtual ~IShape() = default;
    virtual double area() const = 0;
};

class DynCircle : public IShape
{
public:
    explicit DynCircle(double r) : r_(r) {}
    double area() const override
    {
        EventLog::instance().record("DynCircle::area");
        return 3.14159 * r_ * r_;
    }

private:
    double r_;
};

class DynSquare : public IShape
{
public:
    explicit DynSquare(double s) : s_(s) {}
    double area() const override
    {
        EventLog::instance().record("DynSquare::area");
        return s_ * s_;
    }

private:
    double s_;
};

TEST_F(StaticPolymorphismTest, DynamicAndStaticPolymorphismProduceSameAnswer)
{
    // Static (variant) version.
    std::vector<VShape> static_shapes;
    static_shapes.emplace_back(VCircle{1.0});
    static_shapes.emplace_back(VSquare{2.0});

    double static_total = 0.0;
    for (const auto& s : static_shapes)
    {
        static_total += std::visit([](const auto& v) { return v.area(); }, s);
    }

    EventLog::instance().clear();

    // Dynamic (vtable) version.
    std::vector<std::unique_ptr<IShape>> dyn_shapes;
    dyn_shapes.push_back(std::make_unique<DynCircle>(1.0));
    dyn_shapes.push_back(std::make_unique<DynSquare>(2.0));

    double dyn_total = 0.0;
    for (const auto& s : dyn_shapes)
    {
        dyn_total += s->area();
    }

    EXPECT_NEAR(static_total, dyn_total, 1e-6);
    EXPECT_EQ(EventLog::instance().count_events("DynCircle::area"), 1);
    EXPECT_EQ(EventLog::instance().count_events("DynSquare::area"), 1);

    // Q: Both styles produced the same total area. Name one situation
    //    where you'd choose `std::variant<VCircle, VSquare>` and one
    //    where you'd choose `std::unique_ptr<IShape>`. Anchor each
    //    answer in a concrete property that flips the trade-off
    //    (e.g., locality of data, ABI stability, plugin extensibility).
    // A:
    // R:

    // Q: A `std::variant<VCircle, VSquare>` is contiguous-storable; a
    //    `std::unique_ptr<IShape>` is not. What observable runtime
    //    consequence does that have when iterating a million-element
    //    container?
    // A:
    // R:
}
