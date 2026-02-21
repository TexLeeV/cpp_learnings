#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

class ConditionalLifetimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// Complete implementation - study lazy initialization pattern
class LazyResource
{
public:
    LazyResource()
    : resource_(nullptr)
    {
    }
    
    std::shared_ptr<Tracked> get_resource()
    {
        if (!resource_)
        {
            resource_ = std::make_shared<Tracked>("LazyInit");
        }
        
        return resource_;
    }
    
    bool is_initialized() const
    {
        return resource_ != nullptr;
    }
    
private:
    mutable std::shared_ptr<Tracked> resource_;
};

TEST_F(ConditionalLifetimeTest, LazyInitializationBasic)
{
    LazyResource lazy;
    
    bool initialized_before = false;
    // TODO: Check if initialized before first access
    // initialized_before = ???
    
    // TODO: Get resource (triggers initialization)
    // YOUR CODE HERE
    
    bool initialized_after = false;
    // TODO: Check if initialized after first access
    // initialized_after = ???
    
    // TODO: Get resource again
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    EXPECT_FALSE(initialized_before);
    EXPECT_TRUE(initialized_after);
    EXPECT_EQ(use_count, 2);
}

TEST_F(ConditionalLifetimeTest, LazyInitializationMultipleCalls)
{
    LazyResource lazy;
    
    // TODO: Get resource three times
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    EXPECT_EQ(use_count, 3);
}

// Complete implementation - study copy-on-write pattern
class CopyOnWriteString
{
public:
    explicit CopyOnWriteString(const std::string& str)
    : data_(std::make_shared<Tracked>(str))
    {
    }
    
    std::string get() const
    {
        return data_->name();
    }
    
    void set(const std::string& str)
    {
        if (data_.use_count() > 1)
        {
            data_ = std::make_shared<Tracked>(str);
        }
        else
        {
            data_ = std::make_shared<Tracked>(str);
        }
    }
    
    long use_count() const
    {
        return data_.use_count();
    }
    
private:
    std::shared_ptr<Tracked> data_;
};

TEST_F(ConditionalLifetimeTest, CopyOnWriteBasic)
{
    CopyOnWriteString s1("Original");
    
    long use_count_single = 0;
    // TODO: Get use_count
    // use_count_single = ???
    
    // TODO: Copy s1 to s2
    // YOUR CODE HERE
    
    long use_count_shared = 0;
    // TODO: Get use_count after copy
    // use_count_shared = ???
    
    // TODO: Set s2 to "Modified"
    // YOUR CODE HERE
    
    long use_count_after_write = 0;
    // TODO: Get use_count after write
    // use_count_after_write = ???
    
    EXPECT_EQ(use_count_single, 1);
    EXPECT_EQ(use_count_shared, 2);
    EXPECT_EQ(use_count_after_write, 1);
}

TEST_F(ConditionalLifetimeTest, CopyOnWriteMultipleCopies)
{
    CopyOnWriteString s1("Shared");
    
    // TODO: Create s2 and s3 as copies of s1
    // YOUR CODE HERE
    
    long use_count_before = 0;
    // TODO: Get use_count before modification
    // use_count_before = ???
    
    // TODO: Modify s2
    // YOUR CODE HERE
    
    long use_count_after = 0;
    // TODO: Get use_count after modification
    // use_count_after = ???
    
    EXPECT_EQ(use_count_before, 3);
    EXPECT_EQ(use_count_after, 2);
}

// Complete implementation - study deferred construction
class DeferredConstruction
{
public:
    DeferredConstruction()
    : resource_(nullptr)
    {
    }
    
    void initialize(const std::string& name)
    {
        if (!resource_)
        {
            resource_ = std::make_shared<Tracked>(name);
        }
    }
    
    std::shared_ptr<Tracked> get() const
    {
        return resource_;
    }
    
    bool is_initialized() const
    {
        return resource_ != nullptr;
    }
    
private:
    std::shared_ptr<Tracked> resource_;
};

TEST_F(ConditionalLifetimeTest, DeferredConstructionPattern)
{
    DeferredConstruction deferred;
    
    bool initialized_before = false;
    // TODO: Check if initialized before
    // initialized_before = ???
    
    // TODO: Initialize with "Deferred"
    // YOUR CODE HERE
    
    bool initialized_after = false;
    // TODO: Check if initialized after
    // initialized_after = ???
    
    // TODO: Get resource
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    EXPECT_FALSE(initialized_before);
    EXPECT_TRUE(initialized_after);
    EXPECT_EQ(use_count, 1);
}

TEST_F(ConditionalLifetimeTest, DeferredConstructionIdempotent)
{
    DeferredConstruction deferred;
    
    // TODO: Initialize with "First"
    // YOUR CODE HERE
    
    // TODO: Get resource
    // YOUR CODE HERE
    
    // TODO: Initialize again with "Second"
    // YOUR CODE HERE
    
    // TODO: Get resource again
    // YOUR CODE HERE
    
    bool same_resource = false;
    // TODO: Check if both resources are the same
    // same_resource = ???
    
    EXPECT_TRUE(same_resource);
}

// Complete implementation - study conditional ownership
class ConditionalOwnership
{
public:
    ConditionalOwnership()
    : owned_(nullptr)
    {
    }
    
    void take_ownership(std::shared_ptr<Tracked> resource)
    {
        owned_ = resource;
    }
    
    void release_ownership()
    {
        owned_.reset();
    }
    
    bool has_ownership() const
    {
        return owned_ != nullptr;
    }
    
    long use_count() const
    {
        return owned_ ? owned_.use_count() : 0;
    }
    
private:
    std::shared_ptr<Tracked> owned_;
};

TEST_F(ConditionalLifetimeTest, ConditionalOwnershipPattern)
{
    ConditionalOwnership owner;
    
    bool has_ownership_initial = false;
    // TODO: Check if has ownership initially
    // has_ownership_initial = ???
    
    // TODO: Create resource
    // YOUR CODE HERE
    
    long use_count_before = 0;
    // TODO: Get use_count before taking ownership
    // use_count_before = ???
    
    // TODO: Take ownership
    // YOUR CODE HERE
    
    long use_count_after = 0;
    bool has_ownership_after = false;
    // TODO: Get use_count and check ownership after
    // use_count_after = ???
    // has_ownership_after = ???
    
    // TODO: Release ownership
    // YOUR CODE HERE
    
    long use_count_released = 0;
    bool has_ownership_released = false;
    // TODO: Get use_count and check ownership after release
    // use_count_released = ???
    // has_ownership_released = ???
    
    EXPECT_FALSE(has_ownership_initial);
    EXPECT_EQ(use_count_before, 1);
    EXPECT_EQ(use_count_after, 2);
    EXPECT_TRUE(has_ownership_after);
    EXPECT_EQ(use_count_released, 1);
    EXPECT_FALSE(has_ownership_released);
}

// Complete implementation - study resource pool
class ResourcePool
{
public:
    std::shared_ptr<Tracked> acquire(const std::string& name)
    {
        if (!pool_.empty())
        {
            std::shared_ptr<Tracked> resource = pool_.back();
            pool_.pop_back();
            return resource;
        }
        
        return std::make_shared<Tracked>(name);
    }
    
    void release(std::shared_ptr<Tracked> resource)
    {
        if (resource.use_count() == 1)
        {
            pool_.push_back(resource);
        }
    }
    
    size_t pool_size() const
    {
        return pool_.size();
    }
    
private:
    std::vector<std::shared_ptr<Tracked>> pool_;
};

TEST_F(ConditionalLifetimeTest, ResourcePoolPattern)
{
    ResourcePool pool;
    
    size_t initial_size = 0;
    // TODO: Get initial pool size
    // initial_size = ???
    
    // TODO: Acquire resource
    // YOUR CODE HERE
    
    size_t after_acquire = 0;
    // TODO: Get pool size after acquire
    // after_acquire = ???
    
    // TODO: Release resource
    // YOUR CODE HERE
    
    size_t after_release = 0;
    // TODO: Get pool size after release
    // after_release = ???
    
    EXPECT_EQ(initial_size, 0);
    EXPECT_EQ(after_acquire, 0);
    EXPECT_EQ(after_release, 0);
}

TEST_F(ConditionalLifetimeTest, ResourcePoolReuse)
{
    ResourcePool pool;
    
    // TODO: Acquire, release, and reset resource
    // YOUR CODE HERE
    
    size_t pool_size = 0;
    // TODO: Get pool size
    // pool_size = ???
    
    // TODO: Acquire again
    // YOUR CODE HERE
    
    size_t after_reacquire = 0;
    // TODO: Get pool size after reacquire
    // after_reacquire = ???
    
    EXPECT_EQ(pool_size, 1);
    EXPECT_EQ(after_reacquire, 0);
}

// Complete implementation - study optional resource
class OptionalResource
{
public:
    OptionalResource()
    : resource_(nullptr)
    {
    }
    
    void set(std::shared_ptr<Tracked> resource)
    {
        resource_ = resource;
    }
    
    std::shared_ptr<Tracked> get() const
    {
        return resource_;
    }
    
    bool has_value() const
    {
        return resource_ != nullptr;
    }
    
    void reset()
    {
        resource_.reset();
    }
    
private:
    std::shared_ptr<Tracked> resource_;
};

TEST_F(ConditionalLifetimeTest, OptionalResourcePattern)
{
    OptionalResource optional;
    
    bool has_value_initial = false;
    // TODO: Check if has value initially
    // has_value_initial = ???
    
    // TODO: Create resource and set it
    // YOUR CODE HERE
    
    bool has_value_set = false;
    long use_count = 0;
    // TODO: Check if has value and get use_count
    // has_value_set = ???
    // use_count = ???
    
    // TODO: Reset optional
    // YOUR CODE HERE
    
    bool has_value_reset = false;
    long use_count_after_reset = 0;
    // TODO: Check if has value and get use_count after reset
    // has_value_reset = ???
    // use_count_after_reset = ???
    
    EXPECT_FALSE(has_value_initial);
    EXPECT_TRUE(has_value_set);
    EXPECT_EQ(use_count, 2);
    EXPECT_FALSE(has_value_reset);
    EXPECT_EQ(use_count_after_reset, 1);
}

TEST_F(ConditionalLifetimeTest, UseCountBasedDecision)
{
    // TODO: Create shared_ptr
    // YOUR CODE HERE
    
    bool is_unique_before = false;
    // TODO: Check if use_count == 1
    // is_unique_before = ???
    
    // TODO: Create copy
    // YOUR CODE HERE
    
    bool is_unique_after = false;
    // TODO: Check if use_count == 1
    // is_unique_after = ???
    
    EXPECT_TRUE(is_unique_before);
    EXPECT_FALSE(is_unique_after);
}
