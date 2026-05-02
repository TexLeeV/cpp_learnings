// Test Suite: RTTI, dynamic_cast, and Cast Hazards
// Estimated Time: 2-3 hours
// Difficulty: Hard
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

class RttiAndCastsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Polymorphic hierarchy used across scenarios
// ============================================================================

class Event
{
public:
    virtual ~Event() = default;
    virtual std::string kind() const
    {
        return "Event";
    }
};

class ClickEvent : public Event
{
public:
    explicit ClickEvent(int x, int y) : x_(x), y_(y) {}

    std::string kind() const override
    {
        return "Click";
    }

    int x() const { return x_; }
    int y() const { return y_; }

private:
    int x_{};
    int y_{};
};

class KeyEvent : public Event
{
public:
    explicit KeyEvent(char c) : c_(c) {}

    std::string kind() const override
    {
        return "Key";
    }

    char ch() const { return c_; }

private:
    char c_{};
};

// Non-polymorphic type (no virtual functions): used to demonstrate that
// typeid degrades to a static-type query and dynamic_cast cannot see
// derived information.
class PlainBase
{
public:
    int magic{42};
};

class PlainDerived : public PlainBase
{
public:
    int extra{7};
};

// ============================================================================
// Scenario 1: typeid Reports the Dynamic Type for Polymorphic Objects (Easy)
// ============================================================================

TEST_F(RttiAndCastsTest, TypeidIsDynamicForPolymorphicTypes)
{
    ClickEvent c{10, 20};
    Event& e = c;

    const std::type_info& static_view = typeid(e);

    // Q: Why does `typeid(e)` here report `ClickEvent` and not `Event`,
    //    even though `e` is declared as `Event&`? What property of the
    //    `Event` class enables that?
    // A:
    // R:

    EXPECT_STREQ(static_view.name(), typeid(ClickEvent).name());
    EXPECT_NE(static_view, typeid(KeyEvent));
    EXPECT_NE(static_view, typeid(Event));
}

TEST_F(RttiAndCastsTest, TypeidIsStaticForNonPolymorphicTypes)
{
    PlainDerived d;
    PlainBase& b = d;

    const std::type_info& view = typeid(b);

    // Q: `PlainBase` has no virtual function. Why does `typeid(b)` report
    //    `PlainBase` here rather than `PlainDerived`?
    // A:
    // R:

    EXPECT_EQ(view, typeid(PlainBase));
    EXPECT_NE(view, typeid(PlainDerived));
}

// ============================================================================
// Scenario 2: dynamic_cast Pointer Form Returns Null on Mismatch (Easy)
// ============================================================================

TEST_F(RttiAndCastsTest, DynamicCastPointerFormReturnsNullOnMismatch)
{
    std::unique_ptr<Event> e1 = std::make_unique<ClickEvent>(1, 2);
    std::unique_ptr<Event> e2 = std::make_unique<KeyEvent>('q');

    auto* as_click1 = dynamic_cast<ClickEvent*>(e1.get());
    auto* as_click2 = dynamic_cast<ClickEvent*>(e2.get());

    // Q: What runtime check did dynamic_cast perform that produced
    //    `nullptr` in the second case but a valid pointer in the first?
    // A:
    // R:

    ASSERT_NE(as_click1, nullptr);
    EXPECT_EQ(as_click1->x(), 1);
    EXPECT_EQ(as_click2, nullptr);
}

// ============================================================================
// Scenario 3: dynamic_cast Reference Form Throws std::bad_cast (Moderate)
// ============================================================================

TEST_F(RttiAndCastsTest, DynamicCastReferenceFormThrowsOnMismatch)
{
    KeyEvent k{'z'};
    Event& e = k;

    bool threw = false;
    try
    {
        ClickEvent& bad = dynamic_cast<ClickEvent&>(e);
        (void)bad;
    }
    catch (const std::bad_cast&)
    {
        threw = true;
    }

    // Q: Why does the reference form of dynamic_cast THROW on mismatch
    //    while the pointer form returns null? What invariant of references
    //    forces that asymmetry?
    // A:
    // R:

    EXPECT_TRUE(threw);
}

// ============================================================================
// Scenario 4: static_cast Downcast Is Unchecked (Hard)
// ============================================================================

TEST_F(RttiAndCastsTest, StaticCastDowncastTrustsTheProgrammer)
{
    ClickEvent c{3, 4};
    Event& e = c;

    // SAFE: actually a ClickEvent.
    ClickEvent& cc = static_cast<ClickEvent&>(e);
    EXPECT_EQ(cc.x(), 3);

    // Q: If `e` actually referred to a `KeyEvent`, what would
    //    `static_cast<ClickEvent&>(e)` produce, and what category of
    //    failure would result if you then read `cc.x()`?
    // A:
    // R:

    // TODO (learner): Walk through the EventLog with a debugger and explain
    // why no runtime entry is recorded *for the cast itself* in either the
    // safe path above or an unsafe path. What does that absence prove
    // about where static_cast performs its work (compile vs. runtime)?

    // Q: Given that `dynamic_cast` performs a runtime check and `static_cast`
    //    does not, when is `static_cast` actually preferable for a downcast?
    //    Frame the answer in terms of an externally enforced guarantee.
    // A:
    // R:
}

// ============================================================================
// Scenario 5: dynamic_cast vs. Virtual Dispatch (Hard)
// ============================================================================

// Visitor-style alternative: lets each subtype dispatch to the correct
// handler without the caller running an `if` cascade of `dynamic_cast`s.
class VisitorBase
{
public:
    virtual ~VisitorBase() = default;
    virtual void on_click(int x, int y) = 0;
    virtual void on_key(char c) = 0;
};

class VBEvent
{
public:
    virtual ~VBEvent() = default;
    virtual void dispatch(VisitorBase& v) const = 0;
};

class VBClick : public VBEvent
{
public:
    VBClick(int x, int y) : x_(x), y_(y) {}
    void dispatch(VisitorBase& v) const override
    {
        EventLog::instance().record("VBClick::dispatch");
        v.on_click(x_, y_);
    }

private:
    int x_, y_;
};

class VBKey : public VBEvent
{
public:
    explicit VBKey(char c) : c_(c) {}
    void dispatch(VisitorBase& v) const override
    {
        EventLog::instance().record("VBKey::dispatch");
        v.on_key(c_);
    }

private:
    char c_;
};

class CountingVisitor : public VisitorBase
{
public:
    void on_click(int, int) override
    {
        ++clicks;
    }
    void on_key(char) override
    {
        ++keys;
    }

    int clicks{0};
    int keys{0};
};

TEST_F(RttiAndCastsTest, VisitorAvoidsRttiCascade)
{
    std::vector<std::unique_ptr<VBEvent>> events;
    events.push_back(std::make_unique<VBClick>(1, 1));
    events.push_back(std::make_unique<VBKey>('a'));
    events.push_back(std::make_unique<VBClick>(2, 2));

    CountingVisitor visitor;
    for (const auto& e : events)
    {
        e->dispatch(visitor);
    }

    // Q: A naive implementation would chain `if (auto* c =
    //    dynamic_cast<VBClick*>(...))` per type. What does this visitor
    //    pattern give up, and what does it gain, compared to that
    //    `dynamic_cast` cascade? Frame at least one trade-off in terms of
    //    *who must change* when a new subtype is added.
    // A:
    // R:

    EXPECT_EQ(visitor.clicks, 2);
    EXPECT_EQ(visitor.keys, 1);
    EXPECT_EQ(EventLog::instance().count_events("VBClick::dispatch"), 2);
    EXPECT_EQ(EventLog::instance().count_events("VBKey::dispatch"), 1);
}

// ============================================================================
// Scenario 6: dynamic_cast Cost vs. Type-Tag Switch (Hard)
// ============================================================================

class TaggedShape
{
public:
    enum class Kind
    {
        Circle,
        Square
    };

    explicit TaggedShape(Kind k) : kind_(k) {}
    virtual ~TaggedShape() = default;

    Kind kind() const { return kind_; }

private:
    Kind kind_;
};

class TaggedCircle : public TaggedShape
{
public:
    TaggedCircle() : TaggedShape(Kind::Circle) {}
};

class TaggedSquare : public TaggedShape
{
public:
    TaggedSquare() : TaggedShape(Kind::Square) {}
};

TEST_F(RttiAndCastsTest, TypeTagIsCheaperButLessSafeThanDynamicCast)
{
    std::unique_ptr<TaggedShape> s = std::make_unique<TaggedCircle>();

    // Tag-switch: a single integer compare, no RTTI lookup.
    bool is_circle_by_tag = (s->kind() == TaggedShape::Kind::Circle);

    // dynamic_cast: walks the type-info graph at runtime.
    bool is_circle_by_rtti = (dynamic_cast<TaggedCircle*>(s.get()) != nullptr);

    EXPECT_TRUE(is_circle_by_tag);
    EXPECT_TRUE(is_circle_by_rtti);

    // Q: Both checks produce the same answer here. What kind of bug can
    //    only the dynamic_cast catch, and what kind of bug can only the
    //    tag catch? (Hint: think about a derived type that forgets to set
    //    its tag, vs. a derived type that lies about its tag.)
    // A:
    // R:

    // TODO (learner): Add a `class LiarSquare : public TaggedSquare` whose
    // constructor smuggles `Kind::Circle` into the base. Show that the
    // tag-switch misclassifies it while `dynamic_cast<TaggedCircle*>`
    // correctly returns null. Record the failing assertion in `// A:`.
}
