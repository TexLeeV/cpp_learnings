// Test Suite: Delegating and Inheriting Constructors
// Estimated Time: 2 hours
// Difficulty: Easy
// C++11

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <string>

class DelegatingConstructorsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Delegating Constructors Basics (Easy)
// ============================================================================

class Resource
{
public:
    Resource(const std::string& name, int value)
    : name_(name)
    , value_(value)
    {
        EventLog::instance().record("Resource::ctor(name=" + name_ + ", value=" + 
                                    std::to_string(value_) + ")");
    }
    
    // Q: What does this constructor delegate to?
    // A:
    // R:
    
    Resource(const std::string& name)
    : Resource(name, 0)
    {
        EventLog::instance().record("Resource::delegating_ctor(name=" + name_ + ")");
    }
    
    // Q: What is the order of execution: delegated constructor first or delegating constructor first?
    // A:
    // R:
    
    Resource()
    : Resource("default")
    {
        EventLog::instance().record("Resource::default_ctor");
    }
    
    std::string name() const
    {
        return name_;
    }
    
    int value() const
    {
        return value_;
    }
    
private:
    std::string name_;
    int value_;
};

TEST_F(DelegatingConstructorsTest, BasicDelegation)
{
    Resource r1("test", 42);
    
    EXPECT_EQ(r1.name(), "test");
    EXPECT_EQ(r1.value(), 42);
    EXPECT_EQ(EventLog::instance().count_events("Resource::ctor(name=test, value=42)"), 1);
    
    EventLog::instance().clear();
    
    Resource r2("partial");
    
    // Q: How many constructor calls appear in EventLog?
    // A:
    // R:
    
    EXPECT_EQ(r2.name(), "partial");
    EXPECT_EQ(r2.value(), 0);
    EXPECT_EQ(EventLog::instance().count_events("Resource::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("delegating_ctor"), 1);
    
    EventLog::instance().clear();
    
    Resource r3;
    
    EXPECT_EQ(r3.name(), "default");
    EXPECT_EQ(r3.value(), 0);
    
    // Q: What is the delegation chain for the default constructor?
    // A:
    // R:
    
    EXPECT_EQ(EventLog::instance().count_events("default_ctor"), 1);
}

// ============================================================================
// Scenario 2: Delegating with Tracked Objects (Moderate)
// ============================================================================

class TrackedContainer
{
public:
    explicit TrackedContainer(const std::string& name, int capacity)
    : data_(std::make_shared<Tracked>(name))
    , capacity_(capacity)
    {
        EventLog::instance().record("TrackedContainer::main_ctor");
    }
    
    explicit TrackedContainer(const std::string& name)
    : TrackedContainer(name, 10)
    {
        EventLog::instance().record("TrackedContainer::delegating_ctor");
    }
    
    std::string name() const
    {
        return data_->name();
    }
    
    int capacity() const
    {
        return capacity_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    int capacity_;
};

TEST_F(DelegatingConstructorsTest, DelegationWithTracked)
{
    TrackedContainer tc("Container");
    
    // Q: How many Tracked objects were constructed?
    // A:
    // R:
    
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Container)::ctor"), 1);
    
    // Q: In what order do the EventLog entries appear?
    // A:
    // R:
    
    EXPECT_EQ(EventLog::instance().count_events("main_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("delegating_ctor"), 1);
    
    auto events = EventLog::instance().events();
    
    // Q: Which event comes first in the log?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Inheriting Constructors (C++11) (Moderate)
// ============================================================================

class Base
{
public:
    explicit Base(const std::string& name)
    : name_(name)
    {
        EventLog::instance().record("Base::ctor(name=" + name_ + ")");
    }
    
    Base(const std::string& name, int value)
    : name_(name)
    , value_(value)
    {
        EventLog::instance().record("Base::ctor(name=" + name_ + ", value=" + 
                                    std::to_string(value_) + ")");
    }
    
    std::string name() const
    {
        return name_;
    }
    
    int value() const
    {
        return value_;
    }
    
protected:
    std::string name_;
    int value_ = 0;
};

class Derived : public Base
{
public:
    // Q: What does 'using Base::Base' do?
    // A:
    // R:
    
    using Base::Base;
    
    void extra_method()
    {
        EventLog::instance().record("Derived::extra_method");
    }
};

TEST_F(DelegatingConstructorsTest, InheritingConstructors)
{
    // Q: Which constructor is called?
    // A:
    // R:
    
    Derived d1("inherited");
    
    EXPECT_EQ(d1.name(), "inherited");
    EXPECT_EQ(d1.value(), 0);
    EXPECT_EQ(EventLog::instance().count_events("Base::ctor(name=inherited)"), 1);
    
    EventLog::instance().clear();
    
    Derived d2("inherited", 42);
    
    EXPECT_EQ(d2.name(), "inherited");
    EXPECT_EQ(d2.value(), 42);
    EXPECT_EQ(EventLog::instance().count_events("Base::ctor(name=inherited, value=42)"), 1);
    
    // Q: What constructors does Derived have after 'using Base::Base'?
    // A:
    // R:
}
