// Test Suite: Creational Patterns (Factory, Builder, Singleton)
// Estimated Time: 5 hours
// Difficulty: Moderate

// LEAVE THIS FILE ALONE.  It makes sense that it is 500+ lines due to the 5 patterns being tested

#include "instrumentation.h"

#include <functional>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string>

class CreationalPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Factory Method Pattern - Easy
// ============================================================================

class Product
{
public:
    virtual ~Product() = default;
    virtual std::string get_type() const = 0;
};

class ConcreteProductA : public Product
{
public:
    ConcreteProductA()
    {
        EventLog::instance().record("ConcreteProductA::ctor");
    }

    std::string get_type() const override
    {
        return "ProductA";
    }

    ~ConcreteProductA() override
    {
        EventLog::instance().record("ConcreteProductA::dtor");
    }
};

class ConcreteProductB : public Product
{
public:
    ConcreteProductB()
    {
        EventLog::instance().record("ConcreteProductB::ctor");
    }

    std::string get_type() const override
    {
        return "ProductB";
    }

    ~ConcreteProductB() override
    {
        EventLog::instance().record("ConcreteProductB::dtor");
    }
};

class Creator
{
public:
    virtual ~Creator() = default;
    virtual std::unique_ptr<Product> factory_method() const = 0;

    std::string operate() const
    {
        auto product = factory_method();
        return "Creator: working with " + product->get_type();
    }
};

class ConcreteCreatorA : public Creator
{
public:
    std::unique_ptr<Product> factory_method() const override
    {
        return std::make_unique<ConcreteProductA>();
    }
};

class ConcreteCreatorB : public Creator
{
public:
    std::unique_ptr<Product> factory_method() const override
    {
        return std::make_unique<ConcreteProductB>();
    }
};

TEST_F(CreationalPatternsTest, FactoryMethodPattern)
{
    {
        ConcreteCreatorA creator_a;
        std::string result = creator_a.operate();
        EXPECT_EQ(result, "Creator: working with ProductA");
    }

    // Q: The factory_method() returns std::unique_ptr<Product>. What ownership transfer
    // Q: occurs when operate() calls factory_method()? Who owns the Product?
    // A:
    // R:

    // Q: When the result string goes out of scope at the closing brace, what destructor
    // Q: sequence occurs? Which object is destroyed first?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("ProductA::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("ProductA::dtor"), 1);
}

// ============================================================================
// TEST 3: Builder Pattern with Fluent Interface - Moderate
// ============================================================================

class Pizza
{
public:
    void set_dough(const std::string& dough)
    {
        dough_ = dough;
        EventLog::instance().record("Pizza::set_dough(" + dough + ")");
    }

    void set_sauce(const std::string& sauce)
    {
        sauce_ = sauce;
        EventLog::instance().record("Pizza::set_sauce(" + sauce + ")");
    }

    void set_topping(const std::string& topping)
    {
        topping_ = topping;
        EventLog::instance().record("Pizza::set_topping(" + topping + ")");
    }

    std::string describe() const
    {
        return "Pizza with " + dough_ + ", " + sauce_ + ", " + topping_;
    }

private:
    std::string dough_;
    std::string sauce_;
    std::string topping_;
};

class PizzaBuilder
{
public:
    PizzaBuilder() : pizza_(std::make_unique<Pizza>())
    {
    }

    PizzaBuilder& dough(const std::string& dough)
    {
        pizza_->set_dough(dough);
        return *this;
    }

    PizzaBuilder& sauce(const std::string& sauce)
    {
        pizza_->set_sauce(sauce);
        return *this;
    }

    PizzaBuilder& topping(const std::string& topping)
    {
        pizza_->set_topping(topping);
        return *this;
    }

    std::unique_ptr<Pizza> build()
    {
        return std::move(pizza_);
    }

private:
    std::unique_ptr<Pizza> pizza_;
};

TEST_F(CreationalPatternsTest, BuilderPatternFluentInterface)
{
    PizzaBuilder builder;
    auto pizza = builder.dough("thin").sauce("tomato").topping("pepperoni").build();

    EXPECT_EQ(pizza->describe(), "Pizza with thin, tomato, pepperoni");

    // Q: Each builder method returns `*this` by reference. What would happen if they
    // Q: returned by value instead? What copy/move operations would occur?
    // A:
    // R:

    // Q: The build() method returns std::move(pizza_). After build() is called, what
    // Q: state is pizza_ in? Can you call build() again safely?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("set_dough"), 1);
    EXPECT_EQ(EventLog::instance().count_events("set_sauce"), 1);
    EXPECT_EQ(EventLog::instance().count_events("set_topping"), 1);
}

// ============================================================================
// TEST 4: Object Pool Pattern - Hard
// ============================================================================

class PooledObject
{
public:
    explicit PooledObject(int id) : id_(id), in_use_(false)
    {
        EventLog::instance().record("PooledObject::ctor id=" + std::to_string(id));
    }

    void use()
    {
        in_use_ = true;
        EventLog::instance().record("PooledObject::use() id=" + std::to_string(id_));
    }

    void reset()
    {
        in_use_ = false;
        EventLog::instance().record("PooledObject::reset() id=" + std::to_string(id_));
    }

    bool in_use() const
    {
        return in_use_;
    }
    int id() const
    {
        return id_;
    }

    ~PooledObject()
    {
        EventLog::instance().record("PooledObject::dtor id=" + std::to_string(id_));
    }

private:
    int id_;
    bool in_use_;
};

class ObjectPool
{
public:
    explicit ObjectPool(size_t size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            pool_.push_back(std::make_unique<PooledObject>(i));
        }
    }

    std::shared_ptr<PooledObject> acquire()
    {
        for (auto& obj : pool_)
        {
            if (!obj->in_use())
            {
                obj->use();
                return std::shared_ptr<PooledObject>(obj.get(), [](PooledObject* p) { p->reset(); });
            }
        }
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<PooledObject>> pool_;
};

TEST_F(CreationalPatternsTest, ObjectPoolPattern)
{
    ObjectPool pool(3);

    auto obj1 = pool.acquire();
    auto obj2 = pool.acquire();
    auto obj3 = pool.acquire();
    auto obj4 = pool.acquire();

    EXPECT_NE(obj1, nullptr);
    EXPECT_NE(obj2, nullptr);
    EXPECT_NE(obj3, nullptr);
    EXPECT_EQ(obj4, nullptr);

    int obj1_id = obj1->id();
    obj1.reset();

    auto obj5 = pool.acquire();
    EXPECT_NE(obj5, nullptr);
    EXPECT_EQ(obj5->id(), obj1_id);

    // Q: The shared_ptr returned by acquire() uses a custom deleter that calls reset().
    // Q: What happens when obj1.reset() is called? Which deleter runs?
    // A:
    // R:

    // Q: The pool stores unique_ptr but returns shared_ptr with a non-owning raw pointer.
    // Q: Who owns the PooledObject? What prevents double-deletion?
    // A:
    // R:

    obj2.reset();
    obj3.reset();
    obj5.reset();

    EXPECT_EQ(EventLog::instance().count_events("PooledObject::ctor"), 3);
    EXPECT_EQ(EventLog::instance().count_events("::use()"), 4);
    EXPECT_EQ(EventLog::instance().count_events("::reset()"), 4);
}

// ============================================================================
// TEST 5: TODO - Implement Prototype Pattern with Clone - Moderate
// ============================================================================

// TODO: Implement a Prototype pattern where objects can clone themselves
// TODO: Use virtual clone() method returning std::unique_ptr
// TODO: Demonstrate deep copy vs shallow copy behavior

class Prototype
{
public:
    virtual ~Prototype() = default;
    virtual std::unique_ptr<Prototype> clone() const = 0;
    virtual std::string get_info() const = 0;
};

class ConcretePrototype : public Prototype
{
public:
    explicit ConcretePrototype(const std::string& data) : data_(data)
    {
        // TODO: Add EventLog recording
    }

    std::unique_ptr<Prototype> clone() const override
    {
        // TODO: Implement clone that creates a deep copy
        return nullptr;
    }

    std::string get_info() const override
    {
        return data_;
    }

private:
    std::string data_;
};

TEST_F(CreationalPatternsTest, DISABLED_PrototypePattern)
{
    ConcretePrototype original("original data");
    auto cloned = original.clone();

    EXPECT_EQ(original.get_info(), "original data");
    EXPECT_EQ(cloned->get_info(), "original data");

    // Q: After clone() returns, how many ConcretePrototype objects exist? What ownership
    // Q: does the returned unique_ptr represent?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Registry-Based Factory with std::function - Hard
// ============================================================================

class Shape
{
public:
    virtual ~Shape() = default;
    virtual std::string draw() const = 0;
};

class Circle : public Shape
{
public:
    Circle()
    {
        EventLog::instance().record("Circle::ctor");
    }

    std::string draw() const override
    {
        return "Drawing Circle";
    }
};

class Rectangle : public Shape
{
public:
    Rectangle()
    {
        EventLog::instance().record("Rectangle::ctor");
    }

    std::string draw() const override
    {
        return "Drawing Rectangle";
    }
};

class ShapeFactory
{
public:
    using Creator = std::function<std::unique_ptr<Shape>()>;

    void register_shape(const std::string& type, Creator creator)
    {
        registry_[type] = std::move(creator);
        EventLog::instance().record("ShapeFactory::register_shape(" + type + ")");
    }

    std::unique_ptr<Shape> create(const std::string& type) const
    {
        auto it = registry_.find(type);
        if (it != registry_.end())
        {
            return it->second();
        }
        return nullptr;
    }

private:
    std::map<std::string, Creator> registry_;
};

TEST_F(CreationalPatternsTest, RegistryBasedFactory)
{
    ShapeFactory factory;

    factory.register_shape("circle", []() { return std::make_unique<Circle>(); });
    factory.register_shape("rectangle", []() { return std::make_unique<Rectangle>(); });

    auto shape1 = factory.create("circle");
    auto shape2 = factory.create("rectangle");
    auto shape3 = factory.create("triangle");

    EXPECT_NE(shape1, nullptr);
    EXPECT_EQ(shape1->draw(), "Drawing Circle");
    EXPECT_NE(shape2, nullptr);
    EXPECT_EQ(shape3, nullptr);

    // Q: The registry stores std::function<unique_ptr<Shape>()> which captures the lambda.
    // Q: What is the lifetime of the lambda? When is it destroyed?
    // A:
    // R:

    // Q: Each call to create("circle") invokes the lambda. How many Circle objects are
    // Q: created if create("circle") is called 3 times?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Circle::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Rectangle::ctor"), 1);
}
