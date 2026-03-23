#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class AntiPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(AntiPatternsTest, SharedPtrAsGlobal)
{
    auto global_ptr = std::make_shared<Tracked>("Global");
    
    long use_count = global_ptr.use_count();
    // Q: If this were a global shared_ptr, what observable problem would arise when multiple translation units access it during static initialization?
    // A:
    // R:
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(AntiPatternsTest, PassingByValueUnnecessarily)
{
    auto process = [](std::shared_ptr<Tracked> item)
    {
        auto copy = *item;
    };
    
    auto ptr = std::make_shared<Tracked>("Item");
    
    EventLog::instance().clear();
    
    process(ptr);
    
    auto events = EventLog::instance().events();
    // Q: What observable overhead does passing `shared_ptr` by value introduce compared to passing by `const shared_ptr&`?
    // A:
    // R:
    
    EXPECT_GT(events.size(), 0);
}

TEST_F(AntiPatternsTest, CreatingTwoSharedPtrsFromSameRaw)
{
    Tracked* raw = new Tracked("Dangerous");
    
    std::shared_ptr<Tracked> p1(raw);
    
    long use_count = p1.use_count();
    // Q: If you uncommented `std::shared_ptr<Tracked> p2(raw);`, what would `p1.use_count()` return and why?
    // A:
    // R:
    
    // Q: What runtime failure would occur when both `p1` and the hypothetical `p2` go out of scope?
    // A:
    // R:
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(AntiPatternsTest, HoldingSharedPtrToThis)
{
    // ANTI-PATTERN: Storing shared_ptr to this without enable_shared_from_this
    // Problem: Circular reference, memory leak
    
    class BadSelfReference
    {
    public:
        explicit BadSelfReference(const std::string& name)
        : tracked_(name)
        {
        }
        
        void set_self(std::shared_ptr<BadSelfReference> self)
        {
            self_ = self;  // Creates circular reference!
        }
        
    private:
        Tracked tracked_;
        std::shared_ptr<BadSelfReference> self_;
    };
    
    {
        auto obj = std::make_shared<BadSelfReference>("SelfRef");
        
        obj->set_self(obj);
        
        long use_count = obj.use_count();
        // Q: Why does `use_count` equal 2 after `set_self(obj)`?
        // A:
        // R:
        
        EXPECT_EQ(use_count, 2);
    }
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    // Q: After the scope exits, why does `dtor_count` equal 0? What prevents the destructor from being called?
    // A:
    // R:
    
    EXPECT_EQ(dtor_count, 0);
}

TEST_F(AntiPatternsTest, UsingSharedPtrForUniqueOwnership)
{
    auto ptr = std::make_shared<Tracked>("Unique");
    
    long use_count = ptr.use_count();
    // Q: If `use_count` is always 1 throughout the object's lifetime, what runtime overhead does shared_ptr impose compared to unique_ptr?
    // A:
    // R:
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(AntiPatternsTest, NotUsingMakeShared)
{
    std::shared_ptr<Tracked> p1(new Tracked("WithNew"));
    
    auto p2 = std::make_shared<Tracked>("WithMakeShared");
    // Q: How many heap allocations does `p1` require compared to `p2`? What gets allocated separately?
    // A:
    // R:
    
    EXPECT_NE(p1, nullptr);
    EXPECT_NE(p2, nullptr);
}

TEST_F(AntiPatternsTest, StoringWeakPtrWhenSharedPtrNeeded)
{
    std::weak_ptr<Tracked> weak;
    
    {
        auto shared = std::make_shared<Tracked>("Temporary");
        weak = shared;
    }
    
    bool expired = weak.expired();
    // Q: What design decision would lead you to store a weak_ptr instead of a shared_ptr? What ownership semantics does this express?
    // A:
    // R:
    
    EXPECT_TRUE(expired);
}

TEST_F(AntiPatternsTest, IgnoringWeakPtrLockResult)
{
    std::weak_ptr<Tracked> weak;
    
    {
        auto shared = std::make_shared<Tracked>("Temporary");
        weak = shared;
    }
    
    auto locked = weak.lock();
    
    bool is_null = (locked == nullptr);
    // Q: What runtime failure would occur if you dereferenced `locked` without checking `is_null` first?
    // A:
    // R:
    
    EXPECT_TRUE(is_null);
}

TEST_F(AntiPatternsTest, CyclicReferencesWithoutWeakPtr)
{
    // ANTI-PATTERN: Creating cycles without breaking them with weak_ptr
    // Problem: Memory leak
    
    class Node
    {
    public:
        explicit Node(const std::string& name)
        : tracked_(name)
        {
        }
        
        void set_next(std::shared_ptr<Node> next)
        {
            next_ = next;
        }
        
    private:
        Tracked tracked_;
        std::shared_ptr<Node> next_;  // Should be weak_ptr!
    };
    
    {
        auto node1 = std::make_shared<Node>("Node1");
        auto node2 = std::make_shared<Node>("Node2");
        node1->set_next(node2);
        node2->set_next(node1);
        // Q: After creating the cycle, what are the use counts of `node1` and `node2`?
        // A:
        // R:
    }
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    // Q: After the scope exits, why does `dtor_count` equal 0? What prevents the reference count from reaching zero?
    // A:
    // R:
    
    EXPECT_EQ(dtor_count, 0);
}

TEST_F(AntiPatternsTest, UsingGetToCreateNewSharedPtr)
{
    auto original = std::make_shared<Tracked>("Original");
    
    Tracked* raw = original.get();
    // Q: What is the safe use case for calling `get()` on a shared_ptr? Under what condition does it become dangerous?
    // A:
    // R:
    
    long use_count = original.use_count();
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(AntiPatternsTest, HoldingSharedPtrInUnstableContainer)
{
    std::vector<std::shared_ptr<Tracked>> cache;
    
    {
        auto resource = std::make_shared<Tracked>("Cached");
        
        cache.push_back(resource);
    }
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    // Q: After the scope exits, why does `dtor_count` equal 0? What ownership decision does storing `shared_ptr` in the cache express?
    // A:
    // R:
    
    // Q: If the cache stored `weak_ptr<Tracked>` instead, what would `dtor_count` be after the scope exits?
    // A:
    // R:
    
    EXPECT_EQ(dtor_count, 0);
}
