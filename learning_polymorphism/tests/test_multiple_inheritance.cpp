// Test Suite: Multiple Inheritance, Diamonds, and Virtual Inheritance
// Estimated Time: 3 hours
// Difficulty: Moderate / Hard
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>

class MultipleInheritanceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Plain Multiple Inheritance From Two Independent Bases (Easy)
// ============================================================================

class Printable
{
public:
    virtual ~Printable() = default;
    virtual std::string print() const
    {
        EventLog::instance().record("Printable::print");
        return "printable";
    }
};

class Serializable
{
public:
    virtual ~Serializable() = default;
    virtual std::string serialize() const
    {
        EventLog::instance().record("Serializable::serialize");
        return "serialized";
    }
};

class Document : public Printable, public Serializable
{
public:
    std::string print() const override
    {
        EventLog::instance().record("Document::print");
        return "doc-print";
    }

    std::string serialize() const override
    {
        EventLog::instance().record("Document::serialize");
        return "doc-serialize";
    }
};

TEST_F(MultipleInheritanceTest, IndependentBasesHaveNoOverlap)
{
    Document d;
    Printable& p = d;
    Serializable& s = d;

    p.print();
    s.serialize();

    // Q: `Document` has two distinct base subobjects in the same complete
    //    object. Why does that work without ambiguity here, even though it
    //    will NOT work when both bases share a common ancestor?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Document::print"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Document::serialize"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Printable::print"), 0);
    EXPECT_EQ(EventLog::instance().count_events("Serializable::serialize"), 0);
}

// ============================================================================
// Scenario 2: Pointer Adjustment Across Bases (Moderate)
// ============================================================================

TEST_F(MultipleInheritanceTest, BasePointerAdjustmentBetweenSubobjects)
{
    Document d;

    Printable* pp = &d;
    Serializable* ps = &d;

    // The two base subobjects sit at different offsets inside the same
    // complete object. The compiler is allowed to adjust the pointer value
    // when crossing between them.
    const void* as_printable = static_cast<const void*>(pp);
    const void* as_serializable = static_cast<const void*>(ps);

    // Q: What does it mean if `as_printable != as_serializable`? What does
    //    that tell you about the layout of `Document`?
    // A:
    // R:

    // Either could equal `&d` depending on declaration order, but at most
    // one can: the second base must live at a non-zero offset.
    int matches_d = 0;
    if (as_printable == static_cast<const void*>(&d))
        ++matches_d;
    if (as_serializable == static_cast<const void*>(&d))
        ++matches_d;

    EXPECT_EQ(matches_d, 1);

    // Q: If you `static_cast<Document*>(ps)`, the pointer value typically
    //    changes again. What invariant does the compiler maintain across
    //    these casts so that the round trip lands you back at `&d`?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Diamond Without Virtual Inheritance (Moderate / Hard)
// ============================================================================

class Animal
{
public:
    explicit Animal(const std::string& tag) : tag_(tag)
    {
        EventLog::instance().record("Animal::ctor[" + tag_ + "]");
    }

    virtual ~Animal()
    {
        EventLog::instance().record("Animal::dtor[" + tag_ + "]");
    }

    std::string tag() const { return tag_; }

protected:
    std::string tag_;
};

class Swimmer : public Animal
{
public:
    Swimmer() : Animal("swimmer")
    {
        EventLog::instance().record("Swimmer::ctor");
    }
    ~Swimmer() override
    {
        EventLog::instance().record("Swimmer::dtor");
    }
};

class Flyer : public Animal
{
public:
    Flyer() : Animal("flyer")
    {
        EventLog::instance().record("Flyer::ctor");
    }
    ~Flyer() override
    {
        EventLog::instance().record("Flyer::dtor");
    }
};

// Non-virtual diamond: Duck has TWO Animal subobjects.
class Duck : public Swimmer, public Flyer
{
public:
    Duck()
    {
        EventLog::instance().record("Duck::ctor");
    }
    ~Duck() override
    {
        EventLog::instance().record("Duck::dtor");
    }
};

TEST_F(MultipleInheritanceTest, NonVirtualDiamondHasTwoAnimalSubobjects)
{
    Duck d;

    // TODO (learner): Uncomment the line below. The compiler should reject
    // it as ambiguous: which Animal subobject did you mean? Record the
    // diagnostic in `// A:`.
    //
    // Animal& a = d;

    Animal& via_swimmer = static_cast<Swimmer&>(d);
    Animal& via_flyer = static_cast<Flyer&>(d);

    // Q: Why are `via_swimmer` and `via_flyer` references to *different*
    //    Animal subobjects of the same Duck? What observable signal in the
    //    EventLog count of `"Animal::ctor"` for a single Duck confirms it?
    // A:
    // R:

    EXPECT_NE(&via_swimmer, &via_flyer);
    EXPECT_EQ(via_swimmer.tag(), "swimmer");
    EXPECT_EQ(via_flyer.tag(), "flyer");
    EXPECT_EQ(EventLog::instance().count_events("Animal::ctor"), 2);
}

// ============================================================================
// Scenario 4: Virtual Inheritance Collapses the Diamond (Hard)
// ============================================================================

class Being
{
public:
    explicit Being(const std::string& tag) : tag_(tag)
    {
        EventLog::instance().record("Being::ctor[" + tag_ + "]");
    }

    virtual ~Being()
    {
        EventLog::instance().record("Being::dtor[" + tag_ + "]");
    }

    std::string tag() const { return tag_; }

protected:
    std::string tag_;
};

class VSwimmer : public virtual Being
{
public:
    VSwimmer() : Being("default-being")
    {
        EventLog::instance().record("VSwimmer::ctor");
    }
    ~VSwimmer() override
    {
        EventLog::instance().record("VSwimmer::dtor");
    }
};

class VFlyer : public virtual Being
{
public:
    VFlyer() : Being("default-being")
    {
        EventLog::instance().record("VFlyer::ctor");
    }
    ~VFlyer() override
    {
        EventLog::instance().record("VFlyer::dtor");
    }
};

// Most-derived must initialize the virtual base directly.
class VDuck : public VSwimmer, public VFlyer
{
public:
    VDuck() : Being("vduck")
    {
        EventLog::instance().record("VDuck::ctor");
    }
    ~VDuck() override
    {
        EventLog::instance().record("VDuck::dtor");
    }
};

TEST_F(MultipleInheritanceTest, VirtualInheritanceProducesOneSharedBase)
{
    {
        VDuck d;

        Being& b = d;

        // Q: Why is `Being& b = d;` no longer ambiguous, even though there
        //    are still two paths (via VSwimmer, via VFlyer) up to Being?
        // A:
        // R:

        EXPECT_EQ(b.tag(), "vduck");
    }

    // Q: How many `Being::ctor[...]` entries appear in the EventLog for one
    //    VDuck? Compare that to the non-virtual diamond above and explain
    //    the structural difference.
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Being::ctor[vduck]"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Being::ctor[default-being]"), 0);

    // Q: Why are the `Being("default-being")` initializers in VSwimmer and
    //    VFlyer ignored when constructing a VDuck, but used when
    //    constructing a standalone VSwimmer?
    // A:
    // R:
}

// ============================================================================
// Scenario 5: dynamic_cast Across Sibling Branches (Hard)
// ============================================================================

class Base
{
public:
    virtual ~Base() = default;
};

class LeftBranch : public virtual Base
{
public:
    void left_only()
    {
        EventLog::instance().record("LeftBranch::left_only");
    }
};

class RightBranch : public virtual Base
{
public:
    void right_only()
    {
        EventLog::instance().record("RightBranch::right_only");
    }
};

class Combined : public LeftBranch, public RightBranch
{
};

TEST_F(MultipleInheritanceTest, DynamicCastTraversesAcrossSiblings)
{
    Combined c;
    LeftBranch* lp = &c;

    // dynamic_cast can navigate across a complete object: from one
    // sibling branch up to the most-derived type and back down to the
    // other sibling branch. This is sometimes called a "cross-cast".
    RightBranch* rp = dynamic_cast<RightBranch*>(lp);

    ASSERT_NE(rp, nullptr);
    rp->right_only();

    // Q: Why can dynamic_cast succeed here when static_cast<RightBranch*>(lp)
    //    would either be a hard compile error or silent UB? What runtime
    //    information does dynamic_cast consult that static_cast does not?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("RightBranch::right_only"), 1);

    // Negative case: a standalone LeftBranch has no RightBranch sibling.
    LeftBranch standalone;
    LeftBranch* lp2 = &standalone;
    RightBranch* rp2 = dynamic_cast<RightBranch*>(lp2);

    EXPECT_EQ(rp2, nullptr);

    // Q: What single property of the hierarchy makes the dynamic_cast
    //    return null instead of throwing or returning a garbage pointer?
    // A:
    // R:
}
