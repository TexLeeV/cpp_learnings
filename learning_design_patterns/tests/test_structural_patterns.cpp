// Test Suite: Structural Patterns (Adapter, Decorator, Proxy, Flyweight)
// Estimated Time: 6 hours
// Difficulty: Moderate
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <thread>
#include <chrono>

class StructuralPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Adapter Pattern (Object Adapter) - Easy
// ============================================================================

class LegacyRectangle
{
public:
    LegacyRectangle(int x, int y, int w, int h)
    : x_(x)
    , y_(y)
    , w_(w)
    , h_(h)
    {
        EventLog::instance().record("LegacyRectangle::ctor");
    }

    void legacy_draw()
    {
        EventLog::instance().record("LegacyRectangle::legacy_draw()");
    }

    int get_x() const { return x_; }
    int get_y() const { return y_; }
    int get_w() const { return w_; }
    int get_h() const { return h_; }

private:
    int x_, y_, w_, h_;
};

class Shape
{
public:
    virtual ~Shape() = default;
    virtual void draw() = 0;
    virtual std::string get_info() const = 0;
};

class RectangleAdapter : public Shape
{
public:
    explicit RectangleAdapter(std::unique_ptr<LegacyRectangle> rect)
    : rect_(std::move(rect))
    {
        EventLog::instance().record("RectangleAdapter::ctor");
    }

    void draw() override
    {
        EventLog::instance().record("RectangleAdapter::draw() delegating to legacy");
        rect_->legacy_draw();
    }

    std::string get_info() const override
    {
        std::ostringstream oss;
        oss << "Rectangle at (" << rect_->get_x() << "," << rect_->get_y() << ")";
        return oss.str();
    }

private:
    std::unique_ptr<LegacyRectangle> rect_;
};

TEST_F(StructuralPatternsTest, AdapterPattern)
{
    auto legacy = std::make_unique<LegacyRectangle>(10, 20, 100, 50);
    Shape* shape = new RectangleAdapter(std::move(legacy));

    shape->draw();
    EXPECT_EQ(shape->get_info(), "Rectangle at (10,20)");

    delete shape;

    // Q: The RectangleAdapter takes ownership of LegacyRectangle via unique_ptr.
    // Q: When the adapter is deleted, what destructor sequence occurs?
    // A:
    // R:

    // Q: If LegacyRectangle were passed by raw pointer instead of unique_ptr, who would
    // Q: own it? What ownership ambiguity would this create?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("LegacyRectangle::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("legacy_draw()"), 1);
}

// ============================================================================
// TEST 2: Decorator Pattern with Smart Pointers - Moderate
// ============================================================================

class Component
{
public:
    virtual ~Component() = default;
    virtual std::string operation() const = 0;
};

class ConcreteComponent : public Component
{
public:
    ConcreteComponent()
    {
        EventLog::instance().record("ConcreteComponent::ctor");
    }

    std::string operation() const override
    {
        return "ConcreteComponent";
    }

    ~ConcreteComponent() override
    {
        EventLog::instance().record("ConcreteComponent::dtor");
    }
};

class Decorator : public Component
{
public:
    explicit Decorator(std::unique_ptr<Component> component)
    : component_(std::move(component))
    {
    }

    std::string operation() const override
    {
        return component_->operation();
    }

protected:
    std::unique_ptr<Component> component_;
};

class ConcreteDecoratorA : public Decorator
{
public:
    explicit ConcreteDecoratorA(std::unique_ptr<Component> component)
    : Decorator(std::move(component))
    {
        EventLog::instance().record("ConcreteDecoratorA::ctor");
    }

    std::string operation() const override
    {
        return "DecoratorA(" + Decorator::operation() + ")";
    }
};

class ConcreteDecoratorB : public Decorator
{
public:
    explicit ConcreteDecoratorB(std::unique_ptr<Component> component)
    : Decorator(std::move(component))
    {
        EventLog::instance().record("ConcreteDecoratorB::ctor");
    }

    std::string operation() const override
    {
        return "DecoratorB(" + Decorator::operation() + ")";
    }
};

TEST_F(StructuralPatternsTest, DecoratorPattern)
{
    {
        auto component = std::make_unique<ConcreteComponent>();
        auto decorated = std::make_unique<ConcreteDecoratorA>(std::move(component));
        auto double_decorated = std::make_unique<ConcreteDecoratorB>(std::move(decorated));

        std::string result = double_decorated->operation();
        EXPECT_EQ(result, "DecoratorB(DecoratorA(ConcreteComponent))");
    }

    // Q: When double_decorated goes out of scope, what destructor chain occurs?
    // Q: In what order are the three objects destroyed?
    // A:
    // R:

    // Q: Each decorator takes ownership via std::move(). After the moves, what state
    // Q: are the original unique_ptrs (component, decorated) in?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("ConcreteComponent::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("DecoratorA::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("DecoratorB::ctor"), 1);
}

// ============================================================================
// TEST 3: Proxy Pattern with Lazy Initialization - Moderate
// ============================================================================

class Subject
{
public:
    virtual ~Subject() = default;
    virtual std::string request() const = 0;
};

class RealSubject : public Subject
{
public:
    RealSubject()
    {
        EventLog::instance().record("RealSubject::ctor (expensive initialization)");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::string request() const override
    {
        EventLog::instance().record("RealSubject::request()");
        return "RealSubject response";
    }

    ~RealSubject() override
    {
        EventLog::instance().record("RealSubject::dtor");
    }
};

class LazyProxy : public Subject
{
public:
    LazyProxy()
    : real_subject_(nullptr)
    {
        EventLog::instance().record("LazyProxy::ctor (cheap)");
    }

    std::string request() const override
    {
        if (!real_subject_)
        {
            EventLog::instance().record("LazyProxy: initializing RealSubject");
            real_subject_ = std::make_unique<RealSubject>();
        }
        return real_subject_->request();
    }

    ~LazyProxy() override
    {
        EventLog::instance().record("LazyProxy::dtor");
    }

private:
    mutable std::unique_ptr<RealSubject> real_subject_;
};

TEST_F(StructuralPatternsTest, ProxyPatternLazyInit)
{
    auto proxy = std::make_unique<LazyProxy>();

    EXPECT_EQ(EventLog::instance().count_events("RealSubject::ctor"), 0);

    std::string result1 = proxy->request();
    EXPECT_EQ(result1, "RealSubject response");

    std::string result2 = proxy->request();

    // Q: After the first request(), RealSubject is constructed. What observable signal
    // Q: confirms it's NOT constructed again on the second request()?
    // A:
    // R:

    // Q: The real_subject_ member is mutable. Why is this necessary when request() is const?
    // Q: What const-correctness issue does mutable solve here?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("RealSubject::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("RealSubject::request()"), 2);
    EXPECT_EQ(EventLog::instance().count_events("initializing RealSubject"), 1);
}

// ============================================================================
// TEST 4: Flyweight Pattern with Shared State - Hard
// ============================================================================

class Flyweight
{
public:
    explicit Flyweight(const std::string& shared_state)
    : shared_state_(shared_state)
    {
        EventLog::instance().record("Flyweight::ctor shared_state=" + shared_state);
    }

    void operation(const std::string& unique_state) const
    {
        EventLog::instance().record("Flyweight::operation() shared=" + shared_state_ +
                                   " unique=" + unique_state);
    }

    ~Flyweight()
    {
        EventLog::instance().record("Flyweight::dtor shared_state=" + shared_state_);
    }

private:
    std::string shared_state_;
};

class FlyweightFactory
{
public:
    std::shared_ptr<Flyweight> get_flyweight(const std::string& shared_state)
    {
        auto it = flyweights_.find(shared_state);
        if (it == flyweights_.end())
        {
            EventLog::instance().record("FlyweightFactory: creating new flyweight");
            auto flyweight = std::make_shared<Flyweight>(shared_state);
            flyweights_[shared_state] = flyweight;
            return flyweight;
        }
        else
        {
            EventLog::instance().record("FlyweightFactory: reusing existing flyweight");
            return it->second.lock();
        }
    }

    size_t flyweight_count() const
    {
        return flyweights_.size();
    }

private:
    std::map<std::string, std::weak_ptr<Flyweight>> flyweights_;
};

TEST_F(StructuralPatternsTest, FlyweightPattern)
{
    FlyweightFactory factory;

    auto fw1 = factory.get_flyweight("shared_A");
    auto fw2 = factory.get_flyweight("shared_A");
    auto fw3 = factory.get_flyweight("shared_B");

    EXPECT_EQ(fw1.get(), fw2.get());
    EXPECT_NE(fw1.get(), fw3.get());

    fw1->operation("unique_1");
    fw2->operation("unique_2");

    // Q: fw1 and fw2 point to the same Flyweight object. What is the use_count() of
    // Q: the shared_ptr after both are created? Why?
    // A:
    // R:

    // Q: The factory stores weak_ptr instead of shared_ptr. What happens to the Flyweight
    // Q: when all shared_ptrs (fw1, fw2) go out of scope?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("creating new flyweight"), 2);
    EXPECT_EQ(EventLog::instance().count_events("reusing existing"), 1);
    EXPECT_EQ(factory.flyweight_count(), 2);
}

// ============================================================================
// TEST 5: TODO - Implement Composite Pattern with Tree Structure - Hard
// ============================================================================

// TODO: Implement a Composite pattern for a file system hierarchy:
// TODO: 1. Component interface with add(), remove(), get_size()
// TODO: 2. Leaf (File) - no children
// TODO: 3. Composite (Directory) - can contain files and directories
// TODO: Use unique_ptr for ownership

class FileSystemComponent
{
public:
    virtual ~FileSystemComponent() = default;
    virtual int get_size() const = 0;
    virtual void add(std::unique_ptr<FileSystemComponent> component)
    {
        // TODO: Default implementation for leaf nodes
    }
};

class File : public FileSystemComponent
{
public:
    explicit File(const std::string& name, int size)
    : name_(name)
    , size_(size)
    {
        // TODO: Add EventLog recording
    }

    int get_size() const override
    {
        // TODO: Return file size
        return 0;
    }

private:
    std::string name_;
    int size_;
};

class Directory : public FileSystemComponent
{
public:
    explicit Directory(const std::string& name)
    : name_(name)
    {
        // TODO: Add EventLog recording
    }

    void add(std::unique_ptr<FileSystemComponent> component) override
    {
        // TODO: Add component to children
    }

    int get_size() const override
    {
        // TODO: Return sum of all children sizes
        return 0;
    }

private:
    std::string name_;
    std::vector<std::unique_ptr<FileSystemComponent>> children_;
};

TEST_F(StructuralPatternsTest, DISABLED_CompositePattern)
{
    auto root = std::make_unique<Directory>("root");
    root->add(std::make_unique<File>("file1.txt", 100));
    root->add(std::make_unique<File>("file2.txt", 200));

    auto subdir = std::make_unique<Directory>("subdir");
    subdir->add(std::make_unique<File>("file3.txt", 50));

    root->add(std::move(subdir));

    EXPECT_EQ(root->get_size(), 350);

    // Q: When root->add(std::move(subdir)) is called, what ownership transfer occurs?
    // Q: What state is the subdir unique_ptr in after the move?
    // A:
    // R:
}

// ============================================================================
