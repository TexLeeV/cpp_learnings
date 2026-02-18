#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <map>
#include <vector>
#include <string>

class CollectionPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

class CachedResource
{
public:
    explicit CachedResource(const std::string& name)
    : tracked_(name)
    {
    }
    
    std::string name() const
    {
        return tracked_.name();
    }
    
private:
    Tracked tracked_;
};

// Complete implementation - study this pattern
class ResourceCache
{
public:
    std::shared_ptr<CachedResource> get(const std::string& key)
    {
        auto it = cache_.find(key);
        
        if (it != cache_.end())
        {
            std::shared_ptr<CachedResource> resource = it->second.lock();
            
            if (resource)
            {
                return resource;
            }
            
            cache_.erase(it);
        }
        
        std::shared_ptr<CachedResource> new_resource = std::make_shared<CachedResource>(key);
        cache_[key] = new_resource;
        return new_resource;
    }
    
    size_t size() const
    {
        return cache_.size();
    }
    
    void cleanup()
    {
        for (auto it = cache_.begin(); it != cache_.end();)
        {
            if (it->second.expired())
            {
                it = cache_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    
private:
    std::map<std::string, std::weak_ptr<CachedResource>> cache_;
};

TEST_F(CollectionPatternsTest, WeakPtrCacheBasic)
{
    ResourceCache cache;
    
    size_t initial_size = 0;
    // TODO: Get initial cache size
    // initial_size = ???
    
    // TODO: Get resource "Resource1" from cache
    // YOUR CODE HERE
    
    size_t after_first_get = 0;
    // TODO: Get cache size after first get
    // after_first_get = ???
    
    // TODO: Get same resource again
    // YOUR CODE HERE
    
    size_t after_second_get = 0;
    // TODO: Get cache size after second get
    // after_second_get = ???
    
    long use_count = 0;
    // TODO: Get use_count of r1
    // use_count = ???
    
    EXPECT_EQ(initial_size, 0);
    EXPECT_EQ(after_first_get, 1);
    EXPECT_EQ(after_second_get, 1);
    EXPECT_EQ(use_count, 2);
}

TEST_F(CollectionPatternsTest, WeakPtrCacheExpiration)
{
    ResourceCache cache;
    
    {
        // TODO: Get resource "Temp" in inner scope
        // YOUR CODE HERE
    }
    
    size_t before_cleanup = 0;
    // TODO: Get cache size before cleanup
    // before_cleanup = ???
    
    // TODO: Call cache.cleanup()
    // YOUR CODE HERE
    
    size_t after_cleanup = 0;
    // TODO: Get cache size after cleanup
    // after_cleanup = ???
    
    EXPECT_EQ(before_cleanup, 1);
    EXPECT_EQ(after_cleanup, 0);
}

TEST_F(CollectionPatternsTest, WeakPtrCacheAutoRecreate)
{
    ResourceCache cache;
    
    {
        // TODO: Get resource "AutoRecreate" in inner scope
        // YOUR CODE HERE
    }
    
    // TODO: Get same resource again after first is destroyed
    // YOUR CODE HERE
    
    long use_count = 0;
    // TODO: Get use_count
    // use_count = ???
    
    EXPECT_EQ(use_count, 1);
}

class Event
{
public:
    explicit Event(const std::string& name)
    : tracked_(name)
    {
    }
    
private:
    Tracked tracked_;
};

class Observer
{
public:
    explicit Observer(const std::string& name)
    : tracked_(name)
    {
    }
    
    void notify(const Event& event)
    {
    }
    
private:
    Tracked tracked_;
};

// Complete implementation - study this pattern
class Subject
{
public:
    void attach(std::shared_ptr<Observer> observer)
    {
        observers_.push_back(observer);
    }
    
    void notify_all(const Event& event)
    {
        for (auto it = observers_.begin(); it != observers_.end();)
        {
            std::shared_ptr<Observer> observer = it->lock();
            
            if (observer)
            {
                observer->notify(event);
                ++it;
            }
            else
            {
                it = observers_.erase(it);
            }
        }
    }
    
    size_t observer_count() const
    {
        return observers_.size();
    }
    
private:
    std::vector<std::weak_ptr<Observer>> observers_;
};

TEST_F(CollectionPatternsTest, ObserverPatternBasic)
{
    Subject subject;
    
    // TODO: Create obs1 and obs2
    // YOUR CODE HERE
    
    // TODO: Attach both observers to subject
    // YOUR CODE HERE
    
    size_t count = 0;
    // TODO: Get observer count
    // count = ???
    
    // TODO: Create event and notify all
    // YOUR CODE HERE
    
    EXPECT_EQ(count, 2);
}

TEST_F(CollectionPatternsTest, ObserverPatternAutoRemoval)
{
    Subject subject;
    
    {
        // TODO: Create two observers in inner scope and attach them
        // YOUR CODE HERE
    }
    
    size_t before_notify = 0;
    // TODO: Get observer count before notify
    // before_notify = ???
    
    // TODO: Create event and notify all
    // YOUR CODE HERE
    
    size_t after_notify = 0;
    // TODO: Get observer count after notify
    // after_notify = ???
    
    EXPECT_EQ(before_notify, 2);
    EXPECT_EQ(after_notify, 0);
}

TEST_F(CollectionPatternsTest, ObserverPatternPartialExpiration)
{
    Subject subject;
    
    // TODO: Create persistent observer in outer scope
    // YOUR CODE HERE
    
    // TODO: Attach persistent observer
    // YOUR CODE HERE
    
    {
        // TODO: Create temporary observer in inner scope and attach
        // YOUR CODE HERE
    }
    
    size_t before_notify = 0;
    // TODO: Get observer count before notify
    // before_notify = ???
    
    // TODO: Notify all
    // YOUR CODE HERE
    
    size_t after_notify = 0;
    // TODO: Get observer count after notify
    // after_notify = ???
    
    EXPECT_EQ(before_notify, 2);
    EXPECT_EQ(after_notify, 1);
}

class RegistryEntry
{
public:
    explicit RegistryEntry(const std::string& name)
    : tracked_(name)
    {
    }
    
    std::string name() const
    {
        return tracked_.name();
    }
    
private:
    Tracked tracked_;
};

// Complete implementation - study this pattern
class Registry
{
public:
    void register_entry(const std::string& key, std::shared_ptr<RegistryEntry> entry)
    {
        entries_[key] = entry;
    }
    
    std::shared_ptr<RegistryEntry> lookup(const std::string& key)
    {
        auto it = entries_.find(key);
        
        if (it != entries_.end())
        {
            return it->second.lock();
        }
        
        return nullptr;
    }
    
    size_t size() const
    {
        return entries_.size();
    }
    
    void cleanup()
    {
        for (auto it = entries_.begin(); it != entries_.end();)
        {
            if (it->second.expired())
            {
                it = entries_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    
private:
    std::map<std::string, std::weak_ptr<RegistryEntry>> entries_;
};

TEST_F(CollectionPatternsTest, RegistryPattern)
{
    Registry registry;
    
    {
        // TODO: Create entry1 in inner scope and register with "key1"
        // YOUR CODE HERE
    }
    
    // TODO: Create entry2 in outer scope and register with "key2"
    // YOUR CODE HERE
    
    size_t before_cleanup = 0;
    // TODO: Get registry size before cleanup
    // before_cleanup = ???
    
    // TODO: Call registry.cleanup()
    // YOUR CODE HERE
    
    size_t after_cleanup = 0;
    // TODO: Get registry size after cleanup
    // after_cleanup = ???
    
    // TODO: Lookup both keys
    // YOUR CODE HERE
    
    bool lookup1_null = false;
    bool lookup2_not_null = false;
    // TODO: Check lookup results
    // lookup1_null = ???
    // lookup2_not_null = ???
    
    EXPECT_EQ(before_cleanup, 2);
    EXPECT_EQ(after_cleanup, 1);
    EXPECT_TRUE(lookup1_null);
    EXPECT_TRUE(lookup2_not_null);
}

TEST_F(CollectionPatternsTest, MultipleObserversSharedLifetime)
{
    Subject subject;
    
    // TODO: Create one observer
    // YOUR CODE HERE
    
    // TODO: Attach same observer three times
    // YOUR CODE HERE
    
    size_t count = 0;
    // TODO: Get observer count
    // count = ???
    
    long use_count = 0;
    // TODO: Get use_count of obs1
    // use_count = ???
    
    EXPECT_EQ(count, 3);
    EXPECT_EQ(use_count, 1);
}
