// Test Suite: Interfaces and Casts
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>

class InterfacesAndCastsTest : public ::testing::Test
{
protected:
    void SetUp() override { EventLog::instance().clear(); }
};

class ILogger
{
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& msg) = 0;
    virtual int level() const = 0;
};

class ConsoleLogger : public ILogger
{
public:
    void log(const std::string& msg) override
    {
        EventLog::instance().record("ConsoleLogger::log: " + msg);
    }

    int level() const override { return 1; }
};

class Event
{
public:
    virtual ~Event() = default;
    virtual std::string kind() const { return "Event"; }
};

class ClickEvent : public Event
{
public:
    explicit ClickEvent(int x, int y) : x_(x), y_(y) {}
    std::string kind() const override { return "Click"; }
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
    std::string kind() const override { return "Key"; }
    char ch() const { return c_; }

private:
    char c_{};
};

// ============================================================================
// Scenario 1: Cannot Instantiate Abstract Type (Easy)
// ============================================================================

TEST_F(InterfacesAndCastsTest, AbstractTypeBlocksDirectInstantiation)
{
    // ILogger logger;  // rejected: pure virtuals unimplemented

    // Q: Why is `ILogger logger;` rejected, and what must a derived class
    //    supply before it becomes instantiable?
    // A:
    // R:

    // Q: Why is a pointer (or reference) to ILogger allowed when a stack
    //    object of that type is not?
    // A:
    // R:

    ILogger* p = nullptr;
    EXPECT_EQ(p, nullptr);
}

// ============================================================================
// Scenario 2: unique_ptr<Interface> Ownership (Easy)
// ============================================================================

TEST_F(InterfacesAndCastsTest, UniquePtrInterfaceOwnership)
{
    std::unique_ptr<ILogger> logger = std::make_unique<ConsoleLogger>();
    logger->log("hello");

    // Q: What two properties of ILogger make this ownership safe: (1) the
    //    virtual destructor, and (2) what else about the type?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("ConsoleLogger::log: hello"), 1);
    EXPECT_EQ(logger->level(), 1);

    // Q: Which EventLog entry proves the call reached ConsoleLogger rather
    //    than some empty ILogger default?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: dynamic_cast Pointer Null on Mismatch (Moderate)
// ============================================================================

TEST_F(InterfacesAndCastsTest, DynamicCastPointerNullOnMismatch)
{
    std::unique_ptr<Event> e1 = std::make_unique<ClickEvent>(1, 2);
    std::unique_ptr<Event> e2 = std::make_unique<KeyEvent>('q');

    auto* as_click1 = dynamic_cast<ClickEvent*>(e1.get());
    auto* as_click2 = dynamic_cast<ClickEvent*>(e2.get());

    // Q: What runtime check did dynamic_cast perform that produced nullptr
    //    for e2 but a valid pointer for e1?
    // A:
    // R:

    ASSERT_NE(as_click1, nullptr);
    EXPECT_EQ(as_click1->x(), 1);
    EXPECT_EQ(as_click2, nullptr);
}

// ============================================================================
// Scenario 4: dynamic_cast Reference Throws bad_cast (Moderate)
// ============================================================================

TEST_F(InterfacesAndCastsTest, DynamicCastReferenceThrowsOnMismatch)
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

    // Q: Why does the reference form of dynamic_cast throw on mismatch while
    //    the pointer form returns null?
    // A:
    // R:

    EXPECT_TRUE(threw);

    // Q: What exception type is thrown, and why can a null ClickEvent& never
    //    be the mismatch result?
    // A:
    // R:
}
