#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class PolymorphismPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

class Base
{
public:
    explicit Base(const std::string& name)
    : tracked_(name)
    {
    }
    
    virtual ~Base() = default;
    
    virtual std::string type() const
    {
        return "Base";
    }
    
private:
    Tracked tracked_;
};

class Derived : public Base
{
public:
    explicit Derived(const std::string& name)
    : Base(name)
    , derived_tracked_(name + "_derived")
    {
    }
    
    std::string type() const override
    {
        return "Derived";
    }
    
private:
    Tracked derived_tracked_;
};

TEST_F(PolymorphismPatternsTest, DynamicPointerCastBasic)
{
    std::shared_ptr<Base> base_ptr = std::make_shared<Derived>("D1");
    
    long base_count = base_ptr.use_count();
    
    std::shared_ptr<Derived> derived_ptr = std::dynamic_pointer_cast<Derived>(base_ptr);
    
    long after_cast_base_count = base_ptr.use_count();
    long after_cast_derived_count = derived_ptr.use_count();
    // Q: Why do both `base_ptr` and `derived_ptr` report `use_count() == 2`? What does this reveal about the control block?
    // A:
    // R:
    
    bool cast_succeeded = (derived_ptr != nullptr);
    
    EXPECT_EQ(base_count, 1);
    EXPECT_EQ(after_cast_base_count, 2);
    EXPECT_EQ(after_cast_derived_count, 2);
    EXPECT_TRUE(cast_succeeded);
}

TEST_F(PolymorphismPatternsTest, DynamicPointerCastFailure)
{
    std::shared_ptr<Base> base_ptr = std::make_shared<Base>("B1");
    
    long base_count = base_ptr.use_count();
    
    std::shared_ptr<Derived> derived_ptr = std::dynamic_pointer_cast<Derived>(base_ptr);
    
    long after_cast_base_count = base_ptr.use_count();
    // Q: When `dynamic_pointer_cast` fails, why does `after_cast_base_count` remain at 1? What does this reveal about failed cast behavior?
    // A:
    // R:
    
    bool cast_failed = (derived_ptr == nullptr);
    long derived_count = derived_ptr ? derived_ptr.use_count() : 0;
    
    EXPECT_EQ(base_count, 1);
    EXPECT_EQ(after_cast_base_count, 1);
    EXPECT_TRUE(cast_failed);
    EXPECT_EQ(derived_count, 0);
}

TEST_F(PolymorphismPatternsTest, StaticPointerCast)
{
    std::shared_ptr<Derived> derived_ptr = std::make_shared<Derived>("D1");
    
    std::shared_ptr<Base> base_ptr = std::static_pointer_cast<Base>(derived_ptr);
    
    long derived_count = derived_ptr.use_count();
    long base_count = base_ptr.use_count();
    // Q: Why do `derived_count` and `base_count` both equal 2? What does `static_pointer_cast` do to the control block?
    // A:
    // R:
    
    std::shared_ptr<Derived> back_to_derived = std::static_pointer_cast<Derived>(base_ptr);
    
    long final_count = derived_ptr.use_count();
    
    EXPECT_EQ(derived_count, 2);
    EXPECT_EQ(base_count, 2);
    EXPECT_EQ(final_count, 3);
}

TEST_F(PolymorphismPatternsTest, ConstPointerCast)
{
    std::shared_ptr<const Tracked> const_ptr = std::make_shared<const Tracked>("Const");
    
    long const_count = const_ptr.use_count();
    
    std::shared_ptr<Tracked> mutable_ptr = std::const_pointer_cast<Tracked>(const_ptr);
    
    long after_cast_const_count = const_ptr.use_count();
    long after_cast_mutable_count = mutable_ptr.use_count();
    // Q: After `const_pointer_cast`, both pointers report `use_count() == 2`. What does this reveal about const-casting's impact on the underlying object?
    // A:
    // R:
    
    EXPECT_EQ(const_count, 1);
    EXPECT_EQ(after_cast_const_count, 2);
    EXPECT_EQ(after_cast_mutable_count, 2);
}

class WidgetImpl
{
public:
    explicit WidgetImpl(const std::string& name)
    : tracked_(name)
    {
    }
    
    void do_work()
    {
    }
    
private:
    Tracked tracked_;
};

// Complete implementation - study Pimpl pattern
class Widget
{
public:
    explicit Widget(const std::string& name)
    : pimpl_(std::make_shared<WidgetImpl>(name))
    {
    }
    
    void do_work()
    {
        pimpl_->do_work();
    }
    
    long impl_use_count() const
    {
        return pimpl_.use_count();
    }
    
private:
    std::shared_ptr<WidgetImpl> pimpl_;
};

TEST_F(PolymorphismPatternsTest, PimplIdiomBasic)
{
    Widget w1("Widget1");
    
    long use_count_single = w1.impl_use_count();
    
    Widget w2 = w1;
    
    long use_count_copied = w1.impl_use_count();
    // Q: After copying `w1` to `w2`, what does `use_count_copied == 2` reveal about the implementation object's lifetime?
    // A:
    // R:
    
    // Q: What observable consequence would occur if `w2.do_work()` modified the implementation state?
    // A:
    // R:
    
    EXPECT_EQ(use_count_single, 1);
    EXPECT_EQ(use_count_copied, 2);
}

TEST_F(PolymorphismPatternsTest, PimplIdiomSharedImpl)
{
    Widget w1("Widget1");
    Widget w2 = w1;
    Widget w3 = w2;
    
    long use_count = w1.impl_use_count();
    
    EXPECT_EQ(use_count, 3);
}

class AbstractInterface
{
public:
    virtual ~AbstractInterface() = default;
    virtual void execute() = 0;
};

class ConcreteA : public AbstractInterface
{
public:
    explicit ConcreteA(const std::string& name)
    : tracked_(name)
    {
    }
    
    void execute() override
    {
    }
    
private:
    Tracked tracked_;
};

class ConcreteB : public AbstractInterface
{
public:
    explicit ConcreteB(const std::string& name)
    : tracked_(name)
    {
    }
    
    void execute() override
    {
    }
    
private:
    Tracked tracked_;
};

TEST_F(PolymorphismPatternsTest, PolymorphicContainer)
{
    std::vector<std::shared_ptr<AbstractInterface>> container;
    
    container.push_back(std::make_shared<ConcreteA>("A1"));
    container.push_back(std::make_shared<ConcreteB>("B1"));
    container.push_back(std::make_shared<ConcreteA>("A2"));
    
    size_t container_size = container.size();
    
    long first_use_count = container[0].use_count();
    // Q: Why does storing different concrete types in the same container require shared_ptr instead of unique_ptr?
    // A:
    // R:
    
    EXPECT_EQ(container_size, 3);
    EXPECT_EQ(first_use_count, 1);
}

TEST_F(PolymorphismPatternsTest, DowncastingInHierarchy)
{
    std::shared_ptr<AbstractInterface> interface = std::make_shared<ConcreteA>("A1");
    
    auto as_a = std::dynamic_pointer_cast<ConcreteA>(interface);
    auto as_b = std::dynamic_pointer_cast<ConcreteB>(interface);
    
    bool a_succeeded = (as_a != nullptr);
    bool b_failed = (as_b == nullptr);
    
    long interface_count = interface.use_count();
    // Q: Why does `interface_count` equal 2 after two casts, one of which failed? What contributes to the reference count?
    // A:
    // R:
    
    EXPECT_TRUE(a_succeeded);
    EXPECT_TRUE(b_failed);
    EXPECT_EQ(interface_count, 2);
}

class MultiDerived : public Base
{
public:
    explicit MultiDerived(const std::string& name)
    : Base(name)
    , multi_tracked_(name + "_multi")
    {
    }
    
    std::string type() const override
    {
        return "MultiDerived";
    }
    
private:
    Tracked multi_tracked_;
};

TEST_F(PolymorphismPatternsTest, MultiLevelHierarchyCasting)
{
    std::shared_ptr<Base> base = std::make_shared<MultiDerived>("MD1");
    
    auto as_derived = std::dynamic_pointer_cast<Derived>(base);
    auto as_multi = std::dynamic_pointer_cast<MultiDerived>(base);
    
    bool derived_failed = (as_derived == nullptr);
    bool multi_succeeded = (as_multi != nullptr);
    // Q: Why does casting to `Derived` fail while casting to `MultiDerived` succeeds? What does this reveal about the actual object type?
    // A:
    // R:
    
    long base_count = base.use_count();
    
    EXPECT_TRUE(derived_failed);
    EXPECT_TRUE(multi_succeeded);
    EXPECT_EQ(base_count, 2);
}

TEST_F(PolymorphismPatternsTest, AliasingWithPolymorphicTypes)
{
    struct Container
    {
        Base base_member;
        explicit Container(const std::string& name)
        : base_member(name)
        {
        }
    };
    
    auto container = std::make_shared<Container>("Container1");
    
    std::shared_ptr<Base> alias(container, &container->base_member);
    
    long container_count = container.use_count();
    long alias_count = alias.use_count();
    // Q: The aliasing constructor points `alias` to `base_member` but shares ownership with `container`. What would happen if `container` were destroyed while `alias` still exists?
    // A:
    // R:
    
    EXPECT_EQ(container_count, 2);
    EXPECT_EQ(alias_count, 2);
}
