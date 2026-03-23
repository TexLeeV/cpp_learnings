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
    {
    }
    
    std::shared_ptr<Tracked> get_resource()
    {
        if (auto cached = resource_.lock())
        {
            return cached;
        }
        
        auto new_resource = std::make_shared<Tracked>("LazyInit");
        resource_ = new_resource;
        return new_resource;
    }
    
    bool is_initialized() const
    {
        return !resource_.expired();
    }
    
private:
    mutable std::weak_ptr<Tracked> resource_;
};

TEST_F(ConditionalLifetimeTest, LazyInitializationBasic)
{
    LazyResource lazy;
    
    bool initialized_before = lazy.is_initialized();
    
    auto r1 = lazy.get_resource();
    
    bool initialized_after = lazy.is_initialized();
    
    auto r2 = lazy.get_resource();
    
    long use_count = r1.use_count();
    // Q: Why does `use_count` equal 2 after two calls to `get_resource()`?
    // A:
    // R:
    
    EXPECT_FALSE(initialized_before);
    EXPECT_TRUE(initialized_after);
    EXPECT_EQ(use_count, 2);
}

TEST_F(ConditionalLifetimeTest, LazyInitializationMultipleCalls)
{
    LazyResource lazy;
    
    auto r1 = lazy.get_resource();
    auto r2 = lazy.get_resource();
    auto r3 = lazy.get_resource();
    
    long use_count = r1.use_count();
    // Q: What happens to the weak_ptr stored in `resource_` after the first call to `get_resource()`?
    // A:
    // R:
    
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
    
    long use_count_single = s1.use_count();
    
    CopyOnWriteString s2 = s1;
    
    long use_count_shared = s1.use_count();
    // Q: After copying `s1` to `s2`, what does `use_count_shared == 2` tell you about the underlying Tracked object?
    // A:
    // R:
    
    s2.set("Modified");
    
    long use_count_after_write = s2.use_count();
    // Q: Why does `s2.set()` result in `use_count_after_write == 1`? What observable signal in EventLog would confirm a new allocation occurred?
    // A:
    // R:
    
    EXPECT_EQ(use_count_single, 1);
    EXPECT_EQ(use_count_shared, 2);
    EXPECT_EQ(use_count_after_write, 1);
}

TEST_F(ConditionalLifetimeTest, CopyOnWriteMultipleCopies)
{
    CopyOnWriteString s1("Shared");
    
    CopyOnWriteString s2 = s1;
    CopyOnWriteString s3 = s1;
    
    long use_count_before = s1.use_count();
    
    s2.set("Modified");
    
    long use_count_after = s1.use_count();
    // Q: After `s2.set()`, why does `s1.use_count()` drop from 3 to 2 instead of remaining at 3?
    // A:
    // R:
    
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
    
    bool initialized_before = deferred.is_initialized();
    
    deferred.initialize("Deferred");
    
    bool initialized_after = deferred.is_initialized();
    
    auto resource = deferred.get();
    
    long use_count = resource.use_count();
    // Q: Why does `use_count` equal 2 after calling `get()`? What are the two owners?
    // A:
    // R:
    
    EXPECT_FALSE(initialized_before);
    EXPECT_TRUE(initialized_after);
    EXPECT_EQ(use_count, 2);
}

TEST_F(ConditionalLifetimeTest, DeferredConstructionIdempotent)
{
    DeferredConstruction deferred;
    
    deferred.initialize("First");
    
    auto r1 = deferred.get();
    
    deferred.initialize("Second");
    
    auto r2 = deferred.get();
    
    bool same_resource = (r1 == r2);
    // Q: Given that `initialize()` was called twice with different names, why does `same_resource` evaluate to true?
    // A:
    // R:
    
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
    
    bool has_ownership_initial = owner.has_ownership();
    
    auto resource = std::make_shared<Tracked>("Resource");
    
    long use_count_before = resource.use_count();
    
    owner.take_ownership(resource);
    
    long use_count_after = resource.use_count();
    bool has_ownership_after = owner.has_ownership();
    // Q: After `take_ownership()`, what prevents the Tracked object from being destroyed if `resource` goes out of scope?
    // A:
    // R:
    
    owner.release_ownership();
    
    long use_count_released = resource.use_count();
    bool has_ownership_released = owner.has_ownership();
    // Q: After `release_ownership()`, what observable signal confirms the owner no longer participates in reference counting?
    // A:
    // R:
    
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
    
    size_t initial_size = pool.pool_size();
    
    auto resource = pool.acquire("Resource1");
    
    size_t after_acquire = pool.pool_size();
    
    pool.release(resource);
    
    size_t after_release = pool.pool_size();
    // Q: Why does `after_release` equal 0 instead of 1? What condition in `release()` prevents the resource from being pooled?
    // A:
    // R:
    
    EXPECT_EQ(initial_size, 0);
    EXPECT_EQ(after_acquire, 0);
    EXPECT_EQ(after_release, 0);
}

TEST_F(ConditionalLifetimeTest, ResourcePoolReuse)
{
    ResourcePool pool;
    
    {
        auto r1 = pool.acquire("Resource1");
        pool.release(std::move(r1));
    }
    
    size_t pool_size = pool.pool_size();
    // Q: After the scope exits, why does `pool_size` equal 1? What prevents the pooled resource from being destroyed?
    // A:
    // R:
    
    auto r2 = pool.acquire("Resource2");
    
    size_t after_reacquire = pool.pool_size();
    // Q: Why does `after_reacquire` equal 0? What happened to the pooled resource from the previous scope?
    // A:
    // R:
    
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
    
    bool has_value_initial = optional.has_value();
    
    auto resource = std::make_shared<Tracked>("Resource");
    optional.set(resource);
    
    bool has_value_set = optional.has_value();
    long use_count = resource.use_count();
    
    optional.reset();
    
    bool has_value_reset = optional.has_value();
    long use_count_after_reset = resource.use_count();
    // Q: After `optional.reset()`, what guarantees that the Tracked object remains valid for `resource` to use?
    // A:
    // R:
    
    EXPECT_FALSE(has_value_initial);
    EXPECT_TRUE(has_value_set);
    EXPECT_EQ(use_count, 2);
    EXPECT_FALSE(has_value_reset);
    EXPECT_EQ(use_count_after_reset, 1);
}

TEST_F(ConditionalLifetimeTest, UseCountBasedDecision)
{
    auto ptr = std::make_shared<Tracked>("Unique");
    
    bool is_unique_before = (ptr.use_count() == 1);
    
    auto copy = ptr;
    
    bool is_unique_after = (ptr.use_count() == 1);
    // Q: If you used `use_count() == 1` to decide whether to modify the object in-place, what race condition could occur in a multi-threaded context?
    // A:
    // R:
    
    EXPECT_TRUE(is_unique_before);
    EXPECT_FALSE(is_unique_after);
}
