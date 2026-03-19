// Test Suite: Modern C++ Pattern Implementations
// Estimated Time: 5 hours
// Difficulty: Moderate
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include "move_instrumentation.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <variant>

class ModernCppPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: RAII-Based Scope Guard Pattern - Easy
// ============================================================================

class ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> cleanup)
    : cleanup_(std::move(cleanup))
    , active_(true)
    {
        EventLog::instance().record("ScopeGuard::ctor");
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    void dismiss()
    {
        active_ = false;
        EventLog::instance().record("ScopeGuard::dismiss()");
    }

    ~ScopeGuard()
    {
        if (active_ && cleanup_)
        {
            EventLog::instance().record("ScopeGuard::dtor executing cleanup");
            cleanup_();
        }
    }

private:
    std::function<void()> cleanup_;
    bool active_;
};

TEST_F(ModernCppPatternsTest, ScopeGuardPattern)
{
    int resource_state = 0;

    {
        ScopeGuard guard([&resource_state]()
        {
            resource_state = 0;
            EventLog::instance().record("Cleanup executed");
        });
        resource_state = 1;
    }

    EXPECT_EQ(resource_state, 0);

    {
        ScopeGuard guard([&resource_state]()
        {
            resource_state = 0;
        });
        resource_state = 2;
        guard.dismiss();
    }

    EXPECT_EQ(resource_state, 2);

    // Q: When the first guard goes out of scope, the cleanup lambda captures resource_state
    // Q: by reference. What would happen if resource_state were captured by value instead?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("executing cleanup"), 1);
}

// ============================================================================
// TEST 2: Type-Safe Visitor with std::variant - Moderate
// ============================================================================

struct Add { int value; };
struct Multiply { int factor; };
struct Reset {};

using Operation = std::variant<Add, Multiply, Reset>;

class Calculator
{
public:
    Calculator() : value_(0) {}

    void execute(const Operation& op)
    {
        std::visit([this](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Add>)
            {
                value_ += arg.value;
                EventLog::instance().record("Calculator: Add " + std::to_string(arg.value));
            }
            else if constexpr (std::is_same_v<T, Multiply>)
            {
                value_ *= arg.factor;
                EventLog::instance().record("Calculator: Multiply by " + std::to_string(arg.factor));
            }
            else if constexpr (std::is_same_v<T, Reset>)
            {
                value_ = 0;
                EventLog::instance().record("Calculator: Reset");
            }
        }, op);
    }

    int value() const { return value_; }

private:
    int value_;
};

TEST_F(ModernCppPatternsTest, VariantVisitorPattern)
{
    Calculator calc;

    calc.execute(Add{10});
    calc.execute(Multiply{3});
    calc.execute(Add{5});
    calc.execute(Reset{});

    EXPECT_EQ(calc.value(), 0);

    // Q: std::visit uses if constexpr to dispatch on the variant type. What compile-time
    // Q: guarantee does this provide compared to runtime polymorphism?
    // A:
    // R:

    // Q: The Operation variant can hold Add, Multiply, or Reset. What is the size of
    // Q: Operation in memory? How does it compare to a virtual base class pointer?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Add"), 2);
    EXPECT_EQ(EventLog::instance().count_events("Multiply"), 1);
}

// ============================================================================
// TEST 3: Policy-Based Design with Templates - Hard
// ============================================================================

template<typename StoragePolicy>
class Container
{
public:
    void add(int value) { storage_.add(value); }
    int get(size_t index) const { return storage_.get(index); }
    size_t size() const { return storage_.size(); }

private:
    StoragePolicy storage_;
};

class VectorStorage
{
public:
    void add(int value)
    {
        data_.push_back(value);
        EventLog::instance().record("VectorStorage::add()");
    }
    int get(size_t index) const { return data_[index]; }
    size_t size() const { return data_.size(); }

private:
    std::vector<int> data_;
};

class ArrayStorage
{
public:
    ArrayStorage() : size_(0) {}

    void add(int value)
    {
        if (size_ < 10)
        {
            data_[size_++] = value;
            EventLog::instance().record("ArrayStorage::add()");
        }
    }
    int get(size_t index) const { return data_[index]; }
    size_t size() const { return size_; }

private:
    int data_[10];
    size_t size_;
};

TEST_F(ModernCppPatternsTest, PolicyBasedDesign)
{
    Container<VectorStorage> vec_container;
    Container<ArrayStorage> arr_container;

    vec_container.add(10);
    arr_container.add(30);

    EXPECT_EQ(vec_container.get(0), 10);
    EXPECT_EQ(arr_container.get(0), 30);

    // Q: Container<VectorStorage> and Container<ArrayStorage> are different types at
    // Q: compile time. What code reuse does the template provide? What is NOT shared?
    // A:
    // R:

    // Q: Compare this to runtime polymorphism with a StorageInterface base class.
    // Q: What performance advantage does the policy-based approach have?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("VectorStorage::add()"), 1);
    EXPECT_EQ(EventLog::instance().count_events("ArrayStorage::add()"), 1);
}

// ============================================================================
// TEST 4: Dependency Injection with std::function - Moderate
// ============================================================================

class Logger
{
public:
    using LogFunction = std::function<void(const std::string&)>;

    explicit Logger(LogFunction log_func)
    : log_func_(std::move(log_func))
    {
        EventLog::instance().record("Logger::ctor");
    }

    void log(const std::string& message)
    {
        if (log_func_)
        {
            log_func_(message);
        }
    }

private:
    LogFunction log_func_;
};

class Service
{
public:
    explicit Service(std::unique_ptr<Logger> logger)
    : logger_(std::move(logger))
    {
    }

    void do_work()
    {
        logger_->log("Service::do_work() called");
    }

private:
    std::unique_ptr<Logger> logger_;
};

TEST_F(ModernCppPatternsTest, DependencyInjection)
{
    std::vector<std::string> log_output;

    auto logger = std::make_unique<Logger>([&log_output](const std::string& msg)
    {
        log_output.push_back(msg);
        EventLog::instance().record("Lambda logger: " + msg);
    });

    Service service(std::move(logger));
    service.do_work();

    EXPECT_EQ(log_output.size(), 1);

    // Q: The Logger is injected via constructor. What testability advantage does this
    // Q: provide compared to Logger being a global or created inside Service?
    // A:
    // R:

    // Q: The lambda captures log_output by reference. What lifetime constraint does
    // Q: this create? What would happen if log_output were destroyed before Service?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Lambda logger"), 1);
}

// ============================================================================
// TEST 5: Factory with Perfect Forwarding - Hard
// ============================================================================

class Base
{
public:
    virtual ~Base() = default;
    virtual std::string get_info() const = 0;
};

class DerivedA : public Base
{
public:
    explicit DerivedA(int value)
    : value_(value)
    {
        EventLog::instance().record("DerivedA::ctor value=" + std::to_string(value));
    }

    std::string get_info() const override
    {
        return "DerivedA(" + std::to_string(value_) + ")";
    }

private:
    int value_;
};

class DerivedB : public Base
{
public:
    DerivedB(std::string name, int count)
    : name_(std::move(name))
    , count_(count)
    {
        EventLog::instance().record("DerivedB::ctor name=" + name_);
    }

    std::string get_info() const override
    {
        return "DerivedB(" + name_ + "," + std::to_string(count_) + ")";
    }

private:
    std::string name_;
    int count_;
};

template<typename T, typename... Args>
std::unique_ptr<Base> make_derived(Args&&... args)
{
    EventLog::instance().record("make_derived<T>() called");
    return std::make_unique<T>(std::forward<Args>(args)...);
}

TEST_F(ModernCppPatternsTest, FactoryPerfectForwarding)
{
    std::string name = "test";
    auto obj1 = make_derived<DerivedA>(42);
    auto obj2 = make_derived<DerivedB>(name, 10);
    auto obj3 = make_derived<DerivedB>(std::move(name), 20);

    EXPECT_EQ(obj1->get_info(), "DerivedA(42)");
    EXPECT_EQ(obj2->get_info(), "DerivedB(test,10)");

    // Q: make_derived<DerivedB>(name, 10) passes name as an lvalue. What happens inside
    // Q: std::forward<Args>(args)...? Is name copied or moved?
    // A:
    // R:

    // Q: make_derived<DerivedB>(std::move(name), 20) passes an rvalue. What does
    // Q: std::forward preserve? Why is perfect forwarding necessary here?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("DerivedA::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("DerivedB::ctor"), 2);
}

// ============================================================================
// TEST 9: TODO - Implement Monostate Pattern with std::optional - Moderate
// ============================================================================

// TODO: Implement a Monostate pattern where all instances share the same state:
// TODO: 1. Use static members for shared state
// TODO: 2. Use std::optional to represent "not initialized" state
// TODO: 3. Demonstrate that changes in one instance affect all instances

class Monostate
{
public:
    void set_value(int value)
    {
        // TODO: Set shared static value
    }

    std::optional<int> get_value() const
    {
        // TODO: Return shared static value if initialized
        return std::nullopt;
    }

private:
    // TODO: Add static std::optional<int> shared_value_
};

TEST_F(ModernCppPatternsTest, DISABLED_MonostatePattern)
{
    Monostate m1;
    Monostate m2;

    EXPECT_FALSE(m1.get_value().has_value());

    m1.set_value(42);

    EXPECT_TRUE(m2.get_value().has_value());
    EXPECT_EQ(m2.get_value().value(), 42);

    // Q: All Monostate instances share the same static value. How does this differ from
    // Q: Singleton? What advantage does Monostate provide?
    // A:
    // R:
}
