// Test Suite: Debugging Techniques and Assertions
// Estimated Time: 4 hours
// Difficulty: Moderate


#include <gtest/gtest.h>
#include "instrumentation.h"
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <stdexcept>

class DebuggingTechniquesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Observable State for Debugging - Easy
// ============================================================================

class DebugCounter
{
public:
    DebugCounter() : value_(0), operation_count_(0)
    {
        EventLog::instance().record("DebugCounter::ctor");
    }

    void increment()
    {
        ++value_;
        ++operation_count_;
        EventLog::instance().record("DebugCounter::increment() value=" +
                                   std::to_string(value_) +
                                   " ops=" + std::to_string(operation_count_));
    }

    void decrement()
    {
        --value_;
        ++operation_count_;
        EventLog::instance().record("DebugCounter::decrement() value=" +
                                   std::to_string(value_) +
                                   " ops=" + std::to_string(operation_count_));
    }

    int value() const { return value_; }
    int operation_count() const { return operation_count_; }

private:
    int value_;
    int operation_count_;
};

TEST_F(DebuggingTechniquesTest, ObservableStateDebugging)
{
    DebugCounter counter;

    counter.increment();
    counter.increment();
    counter.decrement();

    EXPECT_EQ(counter.value(), 1);
    EXPECT_EQ(counter.operation_count(), 3);

    // Q: The EventLog records every operation with the current state. How does this help
    // Q: debug issues compared to only checking the final value?
    // A:
    // R:

    // Q: If value() returned an unexpected result, what EventLog pattern would you search
    // Q: for to find where the bug was introduced?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("increment()"), 2);
    EXPECT_EQ(EventLog::instance().count_events("decrement()"), 1);
}

// ============================================================================
// TEST 2: Assertion vs Exception for Invariant Violations - Moderate
// ============================================================================

class BoundedBuffer
{
public:
    explicit BoundedBuffer(size_t capacity)
    : capacity_(capacity)
    {
        EventLog::instance().record("BoundedBuffer::ctor capacity=" +
                                   std::to_string(capacity));
    }

    void push(int value)
    {
        if (buffer_.size() >= capacity_)
        {
            EventLog::instance().record("BoundedBuffer::push() FAILED - buffer full");
            throw std::overflow_error("Buffer is full");
        }
        buffer_.push_back(value);
        EventLog::instance().record("BoundedBuffer::push(" + std::to_string(value) + ")");
    }

    int pop()
    {
        assert(!buffer_.empty() && "BoundedBuffer::pop() called on empty buffer");

        int value = buffer_.back();
        buffer_.pop_back();
        EventLog::instance().record("BoundedBuffer::pop() returned " + std::to_string(value));
        return value;
    }

    size_t size() const { return buffer_.size(); }

private:
    std::vector<int> buffer_;
    size_t capacity_;
};

TEST_F(DebuggingTechniquesTest, AssertionVsException)
{
    BoundedBuffer buffer(2);

    buffer.push(10);
    buffer.push(20);

    EXPECT_THROW(buffer.push(30), std::overflow_error);

    // Q: push() throws an exception when the buffer is full. Why use an exception here
    // Q: instead of an assertion? What is the difference in intended use?
    // A:
    // R:

    // Q: pop() uses assert() for empty buffer. In Release builds, assertions are disabled.
    // Q: What would happen if pop() were called on an empty buffer in Release mode?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("FAILED - buffer full"), 1);
    EXPECT_EQ(buffer.size(), 2);
}

// ============================================================================
// TEST 3: Debugging Move Semantics with Observable State - Hard
// ============================================================================

class DebugResource
{
public:
    explicit DebugResource(int id)
    : id_(id)
    , valid_(true)
    {
        EventLog::instance().record("DebugResource::ctor id=" + std::to_string(id));
    }

    DebugResource(const DebugResource&) = delete;
    DebugResource& operator=(const DebugResource&) = delete;

    DebugResource(DebugResource&& other) noexcept
    : id_(other.id_)
    , valid_(other.valid_)
    {
        other.valid_ = false;
        EventLog::instance().record("DebugResource::move_ctor id=" + std::to_string(id_) +
                                   " (source now invalid)");
    }

    DebugResource& operator=(DebugResource&& other) noexcept
    {
        if (this != &other)
        {
            if (valid_)
            {
                EventLog::instance().record("DebugResource::move_assign releasing id=" +
                                           std::to_string(id_));
            }
            id_ = other.id_;
            valid_ = other.valid_;
            other.valid_ = false;
            EventLog::instance().record("DebugResource::move_assign acquired id=" +
                                       std::to_string(id_));
        }
        return *this;
    }

    int id() const { return id_; }
    bool is_valid() const { return valid_; }

    ~DebugResource()
    {
        if (valid_)
        {
            EventLog::instance().record("DebugResource::dtor releasing id=" +
                                       std::to_string(id_));
        }
        else
        {
            EventLog::instance().record("DebugResource::dtor moved-from id=" +
                                       std::to_string(id_));
        }
    }

private:
    int id_;
    bool valid_;
};

TEST_F(DebuggingTechniquesTest, DebuggingMoveSemantics)
{
    {
        DebugResource r1(1);
        DebugResource r2(2);

        r2 = std::move(r1);

        EXPECT_FALSE(r1.is_valid());
        EXPECT_TRUE(r2.is_valid());
        EXPECT_EQ(r2.id(), 1);
    }

    // Q: The EventLog shows "releasing id=2" before "acquired id=1". What does this tell
    // Q: you about move assignment? What resource cleanup happens first?
    // A:
    // R:

    // Q: Both r1 and r2 destructors are called. How can you distinguish between a valid
    // Q: resource destruction and a moved-from object destruction in the log?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("move_assign releasing"), 1);
    EXPECT_EQ(EventLog::instance().count_events("moved-from"), 1);
}

// ============================================================================
// TEST 4: Debugging Lifetime Issues with Instrumentation - Hard
// ============================================================================

class Owner
{
public:
    explicit Owner(const std::string& name)
    : name_(name)
    {
        EventLog::instance().record("Owner::ctor name=" + name);
    }

    void set_child(std::unique_ptr<Owner> child)
    {
        child_ = std::move(child);
        EventLog::instance().record("Owner(" + name_ + ")::set_child()");
    }

    const std::string& name() const { return name_; }

    ~Owner()
    {
        EventLog::instance().record("Owner::dtor name=" + name_);
    }

private:
    std::string name_;
    std::unique_ptr<Owner> child_;
};

TEST_F(DebuggingTechniquesTest, DebuggingLifetimeIssues)
{
    {
        auto parent = std::make_unique<Owner>("Parent");
        auto child = std::make_unique<Owner>("Child");
        auto grandchild = std::make_unique<Owner>("Grandchild");

        child->set_child(std::move(grandchild));
        parent->set_child(std::move(child));
    }

    // Q: When parent goes out of scope, what destructor sequence occurs? Use the EventLog
    // Q: to verify the order. Why is this order important?
    // A:
    // R:

    // Q: If Child held a raw pointer to Parent instead of Parent owning Child, what
    // Q: lifetime bug could occur? How would EventLog help you detect it?
    // A:
    // R:

    std::string log = EventLog::instance().dump();
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 3);

    size_t parent_dtor_pos = log.find("Owner::dtor name=Parent");
    size_t child_dtor_pos = log.find("Owner::dtor name=Child");
    size_t grandchild_dtor_pos = log.find("Owner::dtor name=Grandchild");

    EXPECT_LT(parent_dtor_pos, child_dtor_pos);
    EXPECT_LT(child_dtor_pos, grandchild_dtor_pos);
}

// ============================================================================
// TEST 5: TODO - Implement Debug Logging with Conditional Compilation - Moderate
// ============================================================================

// TODO: Implement a debug logging system that:
// TODO: 1. Only logs in Debug builds (use #ifdef NDEBUG)
// TODO: 2. Supports different log levels (DEBUG, INFO, ERROR)
// TODO: 3. Can be disabled at compile time for performance

#ifdef NDEBUG
    #define DEBUG_LOG(msg) ((void)0)
#else
    #define DEBUG_LOG(msg) EventLog::instance().record("DEBUG: " + std::string(msg))
#endif

class DebugLogger
{
public:
    void process(int value)
    {
        DEBUG_LOG("process() called with value=" + std::to_string(value));

        if (value < 0)
        {
            // TODO: Add ERROR level logging
        }
        else
        {
            // TODO: Add INFO level logging
        }

        result_ = value * 2;
    }

    int result() const { return result_; }

private:
    int result_ = 0;
};

TEST_F(DebuggingTechniquesTest, DISABLED_ConditionalDebugLogging)
{
    DebugLogger logger;

    logger.process(10);
    EXPECT_EQ(logger.result(), 20);

    logger.process(-5);
    EXPECT_EQ(logger.result(), -10);

    // Q: In Release builds (NDEBUG defined), DEBUG_LOG becomes a no-op. What performance
    // Q: benefit does this provide? What is the cost in debuggability?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Debugging Reference Lifetime with Dangling References - Hard
// ============================================================================

class Container
{
public:
    explicit Container(const std::string& name)
    : name_(name)
    {
        EventLog::instance().record("Container::ctor name=" + name);
    }

    const std::string& get_name_ref() const
    {
        EventLog::instance().record("Container::get_name_ref() returning reference");
        return name_;
    }

    std::string get_name_copy() const
    {
        EventLog::instance().record("Container::get_name_copy() returning copy");
        return name_;
    }

    ~Container()
    {
        EventLog::instance().record("Container::dtor name=" + name_);
    }

private:
    std::string name_;
};

TEST_F(DebuggingTechniquesTest, DebuggingDanglingReferences)
{
    std::string safe_copy;
    const std::string* dangling_ref = nullptr;

    {
        Container container("test");
        safe_copy = container.get_name_copy();
        dangling_ref = &container.get_name_ref();
    }

    EXPECT_EQ(safe_copy, "test");

    // Q: After the container is destroyed, dangling_ref points to freed memory. What
    // Q: observable signal in EventLog indicates the container was destroyed?
    // A:
    // R:

    // Q: Accessing *dangling_ref is undefined behavior. What debugging tool (AddressSanitizer,
    // Q: Valgrind) would detect this? What would you see in the output?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("returning copy"), 1);
    EXPECT_EQ(EventLog::instance().count_events("returning reference"), 1);
}
