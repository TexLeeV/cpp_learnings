#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <mutex>

class SingletonRegistryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// Complete implementation - study Meyers Singleton with weak_ptr cache
class MeyersSingleton
{
public:
    static MeyersSingleton& instance()
    {
        static MeyersSingleton instance;
        return instance;
    }
    
    std::shared_ptr<Tracked> get_resource()
    {
        if (std::shared_ptr<Tracked> cached = cached_resource_.lock())
        {
            return cached;
        }
        
        std::shared_ptr<Tracked> new_resource = std::make_shared<Tracked>("SingletonResource");
        cached_resource_ = new_resource;
        return new_resource;
    }
    
    bool has_cached_resource() const
    {
        return !cached_resource_.expired();
    }
    
    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;
    
private:
    MeyersSingleton()
    : tracked_("Singleton")
    {
    }
    
    Tracked tracked_;
    std::weak_ptr<Tracked> cached_resource_;
};

TEST_F(SingletonRegistryTest, MeyersSingletonBasic)
{
    MeyersSingleton& s1 = MeyersSingleton::instance();
    MeyersSingleton& s2 = MeyersSingleton::instance();
    
    // Q: Why does Meyers Singleton return a reference instead of a pointer?
    // A:
    // R:
    
    bool same_instance = (&s1 == &s2);
    
    // Q: What guarantees thread-safety in Meyers Singleton (C++11+)?
    // A:
    // R:
    
    EXPECT_TRUE(same_instance);
}

TEST_F(SingletonRegistryTest, MeyersSingletonWithWeakPtrCache)
{
    bool has_cache_initial = MeyersSingleton::instance().has_cached_resource();
    
    // Q: Why is the cache initially empty?
    // A:
    // R:
    
    {
        std::shared_ptr<Tracked> r1 = MeyersSingleton::instance().get_resource();
        std::shared_ptr<Tracked> r2 = MeyersSingleton::instance().get_resource();
        
        // Q: Why does use_count equal 2 after getting resource twice?
        // A:
        // R:
        
        long use_count = r1.use_count();
        bool has_cache_alive = MeyersSingleton::instance().has_cached_resource();
        
        // Q: How does the weak_ptr cache detect the resource is still alive?
        // A:
        // R:
        
        EXPECT_EQ(use_count, 2);
        EXPECT_TRUE(has_cache_alive);
    }
    
    bool has_cache_expired = MeyersSingleton::instance().has_cached_resource();
    
    // Q: After r1 and r2 are destroyed, why does has_cached_resource() return false?
    // A:
    // R:
    
    EXPECT_FALSE(has_cache_initial);
    EXPECT_FALSE(has_cache_expired);
}

TEST_F(SingletonRegistryTest, MeyersSingletonCacheRecreation)
{
    {
        std::shared_ptr<Tracked> r1 = MeyersSingleton::instance().get_resource();
        // Q: What happens to the cached weak_ptr when r1 goes out of scope?
        // A:
        // R:
    }
    
    std::shared_ptr<Tracked> r2 = MeyersSingleton::instance().get_resource();
    
    // Q: Why does get_resource() create a new Tracked object instead of reusing the cache?
    // A:
    // R:
    
    long use_count = r2.use_count();
    
    // Q: What would happen if the cache stored shared_ptr instead of weak_ptr?
    // A:
    // R:
    
    EXPECT_EQ(use_count, 1);
}

// Complete implementation - study thread-safe singleton
class ThreadSafeSingleton
{
public:
    static std::shared_ptr<ThreadSafeSingleton> instance()
    {
        std::call_once(init_flag_, []()
        {
            instance_ = std::shared_ptr<ThreadSafeSingleton>(new ThreadSafeSingleton());
        });
        
        return instance_;
    }
    
    std::string name() const
    {
        return tracked_.name();
    }
    
    ThreadSafeSingleton(const ThreadSafeSingleton&) = delete;
    ThreadSafeSingleton& operator=(const ThreadSafeSingleton&) = delete;
    
private:
    ThreadSafeSingleton()
    : tracked_("ThreadSafe")
    {
    }
    
    Tracked tracked_;
    static std::shared_ptr<ThreadSafeSingleton> instance_;
    static std::once_flag init_flag_;
};

std::shared_ptr<ThreadSafeSingleton> ThreadSafeSingleton::instance_;
std::once_flag ThreadSafeSingleton::init_flag_;

TEST_F(SingletonRegistryTest, ThreadSafeSingletonBasic)
{
    std::shared_ptr<ThreadSafeSingleton> s1 = ThreadSafeSingleton::instance();
    std::shared_ptr<ThreadSafeSingleton> s2 = ThreadSafeSingleton::instance();
    
    // Q: Why does this singleton return shared_ptr instead of a reference?
    // A:
    // R:
    
    bool same_instance = (s1.get() == s2.get());
    long use_count = s1.use_count();
    
    // Q: Why is use_count 3 instead of 2?
    // A:
    // R:
    
    // Q: What role does std::call_once play in thread safety?
    // A:
    // R:
    
    EXPECT_TRUE(same_instance);
    EXPECT_EQ(use_count, 3);
}

// Complete implementation - study global registry
class GlobalRegistry
{
public:
    static GlobalRegistry& instance()
    {
        static GlobalRegistry instance;
        return instance;
    }
    
    void register_resource(const std::string& key, std::shared_ptr<Tracked> resource)
    {
        resources_[key] = resource;
    }
    
    std::shared_ptr<Tracked> get_resource(const std::string& key)
    {
        auto it = resources_.find(key);
        
        if (it != resources_.end())
        {
            return it->second.lock();
        }
        
        return nullptr;
    }
    
    void cleanup()
    {
        for (auto it = resources_.begin(); it != resources_.end();)
        {
            if (it->second.expired())
            {
                it = resources_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    
    size_t size() const
    {
        return resources_.size();
    }
    
    GlobalRegistry(const GlobalRegistry&) = delete;
    GlobalRegistry& operator=(const GlobalRegistry&) = delete;
    
private:
    GlobalRegistry()
    : tracked_("Registry")
    {
    }
    
    Tracked tracked_;
    std::map<std::string, std::weak_ptr<Tracked>> resources_;
};

TEST_F(SingletonRegistryTest, GlobalRegistryBasic)
{
    GlobalRegistry& registry = GlobalRegistry::instance();
    
    std::shared_ptr<Tracked> resource = std::make_shared<Tracked>("Resource1");
    registry.register_resource("key1", resource);
    
    // Q: What happens to the use_count when registering with a weak_ptr registry?
    // A:
    // R:
    
    std::shared_ptr<Tracked> retrieved = registry.get_resource("key1");
    bool not_null = (retrieved != nullptr);
    
    // Q: Why does get_resource() call lock() on the weak_ptr?
    // A:
    // R:
    
    EXPECT_TRUE(not_null);
}

TEST_F(SingletonRegistryTest, GlobalRegistryWeakPtrExpiration)
{
    GlobalRegistry& registry = GlobalRegistry::instance();
    
    {
        std::shared_ptr<Tracked> resource = std::make_shared<Tracked>("Temp");
        registry.register_resource("temp", resource);
        // Q: What happens to the weak_ptr in the registry when resource goes out of scope?
        // A:
        // R:
    }
    
    size_t before_cleanup = registry.size();
    
    // Q: Why does the registry still have size 1 even though the resource is destroyed?
    // A:
    // R:
    
    registry.cleanup();
    size_t after_cleanup = registry.size();
    
    // Q: What does cleanup() check to determine if a weak_ptr should be removed?
    // A:
    // R:
    
    EXPECT_EQ(before_cleanup, 1);
    EXPECT_EQ(after_cleanup, 0);
}

TEST_F(SingletonRegistryTest, GlobalRegistryMultipleResources)
{
    GlobalRegistry& registry = GlobalRegistry::instance();
    
    std::shared_ptr<Tracked> r1 = std::make_shared<Tracked>("R1");
    std::shared_ptr<Tracked> r2 = std::make_shared<Tracked>("R2");
    std::shared_ptr<Tracked> r3 = std::make_shared<Tracked>("R3");
    
    registry.register_resource("key1", r1);
    registry.register_resource("key2", r2);
    registry.register_resource("key3", r3);
    
    size_t size = registry.size();
    
    std::shared_ptr<Tracked> g1 = registry.get_resource("key1");
    std::shared_ptr<Tracked> g2 = registry.get_resource("key2");
    std::shared_ptr<Tracked> g3 = registry.get_resource("key3");
    
    // Q: What is the use_count of r1 after retrieving g1?
    // A:
    // R:
    
    bool all_valid = (g1 != nullptr) && (g2 != nullptr) && (g3 != nullptr);
    
    EXPECT_EQ(size, 3);
    EXPECT_TRUE(all_valid);
}

TEST_F(SingletonRegistryTest, GlobalRegistryPartialExpiration)
{
    GlobalRegistry& registry = GlobalRegistry::instance();
    
    std::shared_ptr<Tracked> persistent = std::make_shared<Tracked>("Persistent");
    registry.register_resource("persist", persistent);
    
    {
        std::shared_ptr<Tracked> temp = std::make_shared<Tracked>("Temp");
        registry.register_resource("temp", temp);
        // Q: What is the registry size at this point?
        // A:
        // R:
    }
    
    // Q: After the inner scope ends, which weak_ptr is expired and which is not?
    // A:
    // R:
    
    registry.cleanup();
    size_t size = registry.size();
    
    // Q: Why does cleanup() only remove the temporary resource?
    // A:
    // R:
    
    EXPECT_EQ(size, 1);
}
