// Test Suite: Abstract Interfaces and Polymorphic Ownership
// Estimated Time: 2-3 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class AbstractInterfacesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Pure-virtual interface: no data, no implementation, only contract
// ============================================================================

class ILogger
{
public:
    virtual ~ILogger() = default;

    virtual void log(const std::string& msg) = 0;
    virtual int level() const = 0;
};

// ============================================================================
// Two concrete implementations of the same interface
// ============================================================================

class ConsoleLogger : public ILogger
{
public:
    void log(const std::string& msg) override
    {
        EventLog::instance().record("ConsoleLogger::log: " + msg);
    }

    int level() const override
    {
        return 1;
    }
};

class FileLogger : public ILogger
{
public:
    void log(const std::string& msg) override
    {
        EventLog::instance().record("FileLogger::log: " + msg);
    }

    int level() const override
    {
        return 2;
    }
};

// ============================================================================
// Scenario 1: Cannot Instantiate an Abstract Type (Easy)
// ============================================================================

TEST_F(AbstractInterfacesTest, AbstractTypeBlocksDirectInstantiation)
{
    // TODO (learner): Uncomment the line below. The compiler should reject
    // it. Record the diagnostic in `// A:`. The error pinpoints which pure
    // virtual functions are still unimplemented.
    //
    // ILogger logger;

    // Q: Why is `ILogger logger;` rejected, and what specifically must change
    //    in a derived class for it to become instantiable?
    // A:
    // R:

    // Pointer-to-abstract is fine - it just cannot be constructed yet.
    ILogger* p = nullptr;
    EXPECT_EQ(p, nullptr);
}

// ============================================================================
// Scenario 2: Polymorphic Ownership Through unique_ptr<Interface> (Easy)
// ============================================================================

TEST_F(AbstractInterfacesTest, UniquePtrInterfaceOwnership)
{
    std::unique_ptr<ILogger> logger = std::make_unique<ConsoleLogger>();

    logger->log("hello");

    // Q: What two properties of `ILogger` together make this safe:
    //    (1) virtual destructor, (2) something else? Name both.
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("ConsoleLogger::log: hello"), 1);
    EXPECT_EQ(logger->level(), 1);
}

// ============================================================================
// Scenario 3: Factory Returning an Interface (Moderate)
// ============================================================================

enum class LoggerKind
{
    Console,
    File
};

static std::unique_ptr<ILogger> make_logger(LoggerKind kind)
{
    switch (kind)
    {
    case LoggerKind::Console:
        return std::make_unique<ConsoleLogger>();
    case LoggerKind::File:
        return std::make_unique<FileLogger>();
    }
    return nullptr;
}

TEST_F(AbstractInterfacesTest, FactoryHidesConcreteType)
{
    auto a = make_logger(LoggerKind::Console);
    auto b = make_logger(LoggerKind::File);

    a->log("a");
    b->log("b");

    // Q: The caller never names ConsoleLogger or FileLogger. What observable
    //    signal in EventLog tells you which concrete type each pointer
    //    actually owns?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("ConsoleLogger::log: a"), 1);
    EXPECT_EQ(EventLog::instance().count_events("FileLogger::log: b"), 1);

    // Q: Why is returning `unique_ptr<ILogger>` from a factory more useful
    //    than returning `unique_ptr<ConsoleLogger>`? Frame the answer in
    //    terms of what the *caller* is allowed to swap without recompiling.
    // A:
    // R:
}

// ============================================================================
// Scenario 4: Heterogeneous Container of Interface Pointers (Moderate)
// ============================================================================

TEST_F(AbstractInterfacesTest, HeterogeneousLoggerPipeline)
{
    std::vector<std::unique_ptr<ILogger>> sinks;
    sinks.push_back(std::make_unique<ConsoleLogger>());
    sinks.push_back(std::make_unique<FileLogger>());
    sinks.push_back(std::make_unique<ConsoleLogger>());

    EventLog::instance().clear();

    for (auto& s : sinks)
    {
        s->log("event");
    }

    // Q: Why does this fan-out work without any `if`/`switch` on type? What
    //    does the compiler emit at the call site instead?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("ConsoleLogger::log: event"), 2);
    EXPECT_EQ(EventLog::instance().count_events("FileLogger::log: event"), 1);
}

// ============================================================================
// Scenario 5: Default Method on Top of a Pure-Virtual Hook (Moderate)
// ============================================================================

class IPipeline
{
public:
    virtual ~IPipeline() = default;

    // Hook: derived classes must implement this.
    virtual void process(const std::string& item) = 0;

    // Default behavior implemented in terms of the hook.
    void process_all(const std::vector<std::string>& items)
    {
        EventLog::instance().record("IPipeline::process_all begin");
        for (const auto& it : items)
        {
            process(it);
        }
        EventLog::instance().record("IPipeline::process_all end");
    }
};

class UpperPipeline : public IPipeline
{
public:
    void process(const std::string& item) override
    {
        EventLog::instance().record("UpperPipeline::process: " + item);
    }
};

TEST_F(AbstractInterfacesTest, NonVirtualInterfaceTemplateMethod)
{
    UpperPipeline p;
    p.process_all({"a", "b", "c"});

    // Q: `process_all` is non-virtual but calls `process`, which is pure
    //    virtual. Why does that not infinite-loop or static-dispatch back
    //    into the base?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("IPipeline::process_all begin"), 1);
    EXPECT_EQ(EventLog::instance().count_events("UpperPipeline::process: a"), 1);
    EXPECT_EQ(EventLog::instance().count_events("UpperPipeline::process: b"), 1);
    EXPECT_EQ(EventLog::instance().count_events("UpperPipeline::process: c"), 1);
    EXPECT_EQ(EventLog::instance().count_events("IPipeline::process_all end"), 1);

    // Q: This is the "non-virtual interface" idiom (NVI). What does it let
    //    the base class enforce that it could not enforce if `process_all`
    //    were itself virtual?
    // A:
    // R:
}

// ============================================================================
// Scenario 6: Interface Segregation (Hard)
// ============================================================================

class IReader
{
public:
    virtual ~IReader() = default;
    virtual std::string read() = 0;
};

class IWriter
{
public:
    virtual ~IWriter() = default;
    virtual void write(const std::string& s) = 0;
};

// Implements only the half it needs.
class ReadOnlySource : public IReader
{
public:
    std::string read() override
    {
        EventLog::instance().record("ReadOnlySource::read");
        return "data";
    }
};

// Implements both, but each can be passed where only one is needed.
class ReadWriteSink : public IReader, public IWriter
{
public:
    std::string read() override
    {
        EventLog::instance().record("ReadWriteSink::read");
        return buf_;
    }

    void write(const std::string& s) override
    {
        EventLog::instance().record("ReadWriteSink::write: " + s);
        buf_ = s;
    }

private:
    std::string buf_;
};

TEST_F(AbstractInterfacesTest, InterfaceSegregationKeepsBoundariesNarrow)
{
    ReadOnlySource ro;
    ReadWriteSink rw;

    IReader* r1 = &ro;
    IReader* r2 = &rw;
    IWriter* w1 = &rw;

    r1->read();
    r2->read();
    w1->write("payload");

    // Q: Why is exposing `IReader*` to a consumer that only reads structurally
    //    safer than exposing `ReadWriteSink*` directly? Frame the answer in
    //    terms of what the consumer is *unable* to call by accident.
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("ReadOnlySource::read"), 1);
    EXPECT_EQ(EventLog::instance().count_events("ReadWriteSink::read"), 1);
    EXPECT_EQ(EventLog::instance().count_events("ReadWriteSink::write: payload"), 1);

    // TODO (learner): Add a function `consume(IReader& r)` that calls only
    // `r.read()`. Confirm it accepts both `ReadOnlySource` and
    // `ReadWriteSink`. Then try to call `r.write(...)` inside `consume`
    // and explain in `// A:` why the compiler rejects it.
}
