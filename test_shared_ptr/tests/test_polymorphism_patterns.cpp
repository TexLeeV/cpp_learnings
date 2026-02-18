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
    // TODO: Create base_ptr pointing to Derived
    // YOUR CODE HERE
    
    long base_count = 0;
    // TODO: Get use_count
    // base_count = ???
    
    // TODO: Use std::dynamic_pointer_cast to cast to Derived
    // YOUR CODE HERE
    
    long after_cast_base_count = 0;
    long after_cast_derived_count = 0;
    // TODO: Get use_counts after cast
    // after_cast_base_count = ???
    // after_cast_derived_count = ???
    
    bool cast_succeeded = false;
    // TODO: Check if cast succeeded
    // cast_succeeded = ???
    
    EXPECT_EQ(base_count, 1);
    EXPECT_EQ(after_cast_base_count, 2);
    EXPECT_EQ(after_cast_derived_count, 2);
    EXPECT_TRUE(cast_succeeded);
}

TEST_F(PolymorphismPatternsTest, DynamicPointerCastFailure)
{
    // TODO: Create base_ptr pointing to Base (not Derived)
    // YOUR CODE HERE
    
    long base_count = 0;
    // TODO: Get use_count
    // base_count = ???
    
    // TODO: Try to cast to Derived (should fail)
    // YOUR CODE HERE
    
    long after_cast_base_count = 0;
    // TODO: Get use_count after cast
    // after_cast_base_count = ???
    
    bool cast_failed = false;
    long derived_count = 0;
    // TODO: Check if cast failed and get derived_count
    // cast_failed = ???
    // derived_count = ???
    
    EXPECT_EQ(base_count, 1);
    EXPECT_EQ(after_cast_base_count, 1);
    EXPECT_TRUE(cast_failed);
    EXPECT_EQ(derived_count, 0);
}

TEST_F(PolymorphismPatternsTest, StaticPointerCast)
{
    // TODO: Create derived_ptr
    // YOUR CODE HERE
    
    // TODO: Use std::static_pointer_cast to cast to Base
    // YOUR CODE HERE
    
    long derived_count = 0;
    long base_count = 0;
    // TODO: Get use_counts
    // derived_count = ???
    // base_count = ???
    
    // TODO: Cast back to Derived using static_pointer_cast
    // YOUR CODE HERE
    
    long final_count = 0;
    // TODO: Get final use_count
    // final_count = ???
    
    EXPECT_EQ(derived_count, 2);
    EXPECT_EQ(base_count, 2);
    EXPECT_EQ(final_count, 3);
}

TEST_F(PolymorphismPatternsTest, ConstPointerCast)
{
    // TODO: Create const_ptr to const Tracked
    // YOUR CODE HERE
    
    long const_count = 0;
    // TODO: Get use_count
    // const_count = ???
    
    // TODO: Use std::const_pointer_cast to remove const
    // YOUR CODE HERE
    
    long after_cast_const_count = 0;
    long after_cast_mutable_count = 0;
    // TODO: Get use_counts after cast
    // after_cast_const_count = ???
    // after_cast_mutable_count = ???
    
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
    // TODO: Create w1
    // YOUR CODE HERE
    
    long use_count_single = 0;
    // TODO: Get impl use_count
    // use_count_single = ???
    
    // TODO: Copy w1 to w2
    // YOUR CODE HERE
    
    long use_count_copied = 0;
    // TODO: Get impl use_count after copy
    // use_count_copied = ???
    
    EXPECT_EQ(use_count_single, 1);
    EXPECT_EQ(use_count_copied, 2);
}

TEST_F(PolymorphismPatternsTest, PimplIdiomSharedImpl)
{
    // TODO: Create w1, w2 (copy of w1), w3 (copy of w2)
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get impl use_count
    // use_count = ???
    
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
    // TODO: Create vector of shared_ptr<AbstractInterface>
    // YOUR CODE HERE
    
    // TODO: Add ConcreteA, ConcreteB, ConcreteA instances
    // YOUR CODE HERE
    
    size_t container_size = 0;
    // TODO: Get container size
    // container_size = ???
    
    long first_use_count = 0;
    // TODO: Get use_count of first element
    // first_use_count = ???
    
    EXPECT_EQ(container_size, 3);
    EXPECT_EQ(first_use_count, 1);
}

TEST_F(PolymorphismPatternsTest, DowncastingInHierarchy)
{
    // TODO: Create interface pointing to ConcreteA
    // YOUR CODE HERE
    
    // TODO: Try dynamic_pointer_cast to ConcreteA and ConcreteB
    // YOUR CODE HERE
    
    bool a_succeeded = false;
    bool b_failed = false;
    // TODO: Check cast results
    // a_succeeded = ???
    // b_failed = ???
    
    long interface_count = 0;
    // TODO: Get use_count
    // interface_count = ???
    
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
    // TODO: Create base pointing to MultiDerived
    // YOUR CODE HERE
    
    // TODO: Try casting to Derived and MultiDerived
    // YOUR CODE HERE
    
    bool derived_failed = false;
    bool multi_succeeded = false;
    // TODO: Check cast results
    // derived_failed = ???
    // multi_succeeded = ???
    
    long base_count = 0;
    // TODO: Get use_count
    // base_count = ???
    
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
    
    // TODO: Create container
    // YOUR CODE HERE
    
    // TODO: Create alias to base_member
    // YOUR CODE HERE
    
    long container_count = 0;
    long alias_count = 0;
    // TODO: Get use_counts
    // container_count = ???
    // alias_count = ???
    
    EXPECT_EQ(container_count, 2);
    EXPECT_EQ(alias_count, 2);
}
