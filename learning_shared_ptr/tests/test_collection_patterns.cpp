#include "instrumentation.h"

#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

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
    explicit CachedResource(const std::string& name) : tracked_(name)
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

    size_t initial_size = cache.size();

    auto r1 = cache.get("Resource1");

    size_t after_first_get = cache.size();

    auto r2 = cache.get("Resource1");

    size_t after_second_get = cache.size();
    // Q: Why does `after_second_get` remain at 1 instead of increasing to 2?
    // A:
    // R:

    long use_count = r1.use_count();
    // Q: What are the two owners contributing to `use_count == 2`?
    // A:
    // R:

    EXPECT_EQ(initial_size, 0);
    EXPECT_EQ(after_first_get, 1);
    EXPECT_EQ(after_second_get, 1);
    EXPECT_EQ(use_count, 2);
}

TEST_F(CollectionPatternsTest, WeakPtrCacheExpiration)
{
    ResourceCache cache;

    {
        auto temp = cache.get("Temp");
    }

    size_t before_cleanup = cache.size();
    // Q: After the scope exits, why does `before_cleanup` still equal 1? What does the cache still contain?
    // A:
    // R:

    cache.cleanup();

    size_t after_cleanup = cache.size();
    // Q: What operation in `cleanup()` causes `after_cleanup` to become 0?
    // A:
    // R:

    EXPECT_EQ(before_cleanup, 1);
    EXPECT_EQ(after_cleanup, 0);
}

TEST_F(CollectionPatternsTest, WeakPtrCacheAutoRecreate)
{
    ResourceCache cache;

    {
        auto r1 = cache.get("AutoRecreate");
    }

    auto r2 = cache.get("AutoRecreate");

    long use_count = r2.use_count();
    // Q: After the first scope exits, what happens when `cache.get("AutoRecreate")` is called again? Walk through the
    // logic in `get()`. A: R:

    EXPECT_EQ(use_count, 1);
}

class Event
{
public:
    explicit Event(const std::string& name) : tracked_(name)
    {
    }

private:
    Tracked tracked_;
};

class Observer
{
public:
    explicit Observer(const std::string& name) : tracked_(name)
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

    auto obs1 = std::make_shared<Observer>("Obs1");
    auto obs2 = std::make_shared<Observer>("Obs2");

    subject.attach(obs1);
    subject.attach(obs2);

    size_t count = subject.observer_count();

    Event event("Event1");
    subject.notify_all(event);
    // Q: Why does `Subject` store `weak_ptr<Observer>` instead of `shared_ptr<Observer>`? What problem does this
    // prevent? A: R:

    EXPECT_EQ(count, 2);
}

TEST_F(CollectionPatternsTest, ObserverPatternAutoRemoval)
{
    Subject subject;

    {
        auto obs1 = std::make_shared<Observer>("Obs1");
        auto obs2 = std::make_shared<Observer>("Obs2");
        subject.attach(obs1);
        subject.attach(obs2);
    }

    size_t before_notify = subject.observer_count();
    // Q: After the scope exits, why does `before_notify` still equal 2? What does the vector still contain?
    // A:
    // R:

    Event event("Event1");
    subject.notify_all(event);

    size_t after_notify = subject.observer_count();
    // Q: What operation in `notify_all()` causes `after_notify` to become 0?
    // A:
    // R:

    EXPECT_EQ(before_notify, 2);
    EXPECT_EQ(after_notify, 0);
}

TEST_F(CollectionPatternsTest, ObserverPatternPartialExpiration)
{
    Subject subject;

    auto persistent = std::make_shared<Observer>("Persistent");

    subject.attach(persistent);

    {
        auto temporary = std::make_shared<Observer>("Temporary");
        subject.attach(temporary);
    }

    size_t before_notify = subject.observer_count();

    Event event("Event1");
    subject.notify_all(event);

    size_t after_notify = subject.observer_count();
    // Q: Why does `after_notify` equal 1 instead of 0? What differentiates the persistent observer from the temporary
    // one? A: R:

    EXPECT_EQ(before_notify, 2);
    EXPECT_EQ(after_notify, 1);
}

class RegistryEntry
{
public:
    explicit RegistryEntry(const std::string& name) : tracked_(name)
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
        auto entry1 = std::make_shared<RegistryEntry>("Entry1");
        registry.register_entry("key1", entry1);
    }

    auto entry2 = std::make_shared<RegistryEntry>("Entry2");
    registry.register_entry("key2", entry2);

    size_t before_cleanup = registry.size();
    // Q: After entry1's scope exits, why does `before_cleanup` still equal 2? What does the registry still contain for
    // "key1"? A: R:

    registry.cleanup();

    size_t after_cleanup = registry.size();

    auto lookup1 = registry.lookup("key1");
    auto lookup2 = registry.lookup("key2");

    bool lookup1_null = (lookup1 == nullptr);
    bool lookup2_not_null = (lookup2 != nullptr);
    // Q: Why does `lookup1` return nullptr while `lookup2` returns a valid pointer? What differentiates their states in
    // the registry? A: R:

    EXPECT_EQ(before_cleanup, 2);
    EXPECT_EQ(after_cleanup, 1);
    EXPECT_TRUE(lookup1_null);
    EXPECT_TRUE(lookup2_not_null);
}

TEST_F(CollectionPatternsTest, MultipleObserversSharedLifetime)
{
    Subject subject;

    auto obs1 = std::make_shared<Observer>("Obs1");

    subject.attach(obs1);
    subject.attach(obs1);
    subject.attach(obs1);

    size_t count = subject.observer_count();

    long use_count = obs1.use_count();
    // Q: Why does `use_count` remain at 1 despite attaching the same observer three times? What does this reveal about
    // weak_ptr's impact on reference counting? A: R:

    EXPECT_EQ(count, 3);
    EXPECT_EQ(use_count, 1);
}
