#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class AliasingWeakTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(AliasingWeakTest, AliasingBasic)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };
    
    long owner_count = 0;
    long alias_count = 0;
    
    std::shared_ptr<Container> owner = std::make_shared<Container>("Container1");
    std::shared_ptr<Tracked> alias(owner, &owner->member);
    
    owner_count = owner.use_count();
    alias_count = alias.use_count();
    
    // Q: Why do both owner and alias report a use_count of 2?
    // A:
    // R:
    
    // Q: What object will be deleted when the last shared_ptr (owner or alias) is destroyed?
    // A:
    // R:
    
    // Q: What happens if you call alias.get()? What pointer value does it return?
    // A:
    // R:
    
    EXPECT_EQ(owner_count, 2);
    EXPECT_EQ(alias_count, 2);
}

TEST_F(AliasingWeakTest, AliasingKeepsOwnerAlive)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };
    
    std::shared_ptr<Tracked> alias;
    
    {
        std::shared_ptr<Container> owner = std::make_shared<Container>("Container2");
        alias = std::shared_ptr<Tracked>(owner, &owner->member);
        owner.reset();
        
        // Q: At this point, owner has been reset. Why hasn't the Container been destroyed?
        // A:
        // R:
        
        // Q: What is the use_count of alias at this point?
        // A:
        // R:
    }
    
    // Q: The inner scope has ended. Why hasn't the Container been destroyed yet?
    // A:
    // R:
    
    // Q: What will happen when alias goes out of scope at the end of this test?
    // A:
    // R:
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    EXPECT_EQ(dtor_count, 0);
}

TEST_F(AliasingWeakTest, WeakPtrBasic)
{
    bool expired_before = false;
    bool expired_after = false;
    
    std::weak_ptr<Tracked> weak;
    
    expired_before = weak.expired();
    
    // Q: Why is a default-constructed weak_ptr considered expired?
    // A:
    // R:
    
    {
        std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
        weak = shared;
        
        // Q: What is the use_count of shared at this point?
        // A:
        // R:
        
        // Q: What is the use_count reported by weak at this point?
        // A:
        // R:
    }
    
    expired_after = weak.expired();
    
    // Q: Why is weak expired after the inner scope ends?
    // A:
    // R:
    
    // Q: Does weak_ptr prevent the object from being destroyed?
    // A:
    // R:
    
    EXPECT_TRUE(expired_before);
    EXPECT_TRUE(expired_after);
}

TEST_F(AliasingWeakTest, WeakPtrLock)
{
    long use_count_with_lock = 0;
    bool lock_succeeded = false;
    
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
    std::weak_ptr<Tracked> weak = shared;
    
    // Q: Before calling lock(), what is the use_count of shared?
    // A:
    // R:
    
    std::shared_ptr<Tracked> locked = weak.lock();
    
    lock_succeeded = (locked != nullptr);
    use_count_with_lock = shared.use_count();
    
    // Q: Why does the use_count increase to 2 after calling lock()?
    // A:
    // R:
    
    // Q: What happens to the object if you destroy shared but keep locked alive?
    // A:
    // R:
    
    // Q: What is the relationship between locked and shared? Do they share the same control block?
    // A:
    // R:
    
    EXPECT_TRUE(lock_succeeded);
    EXPECT_EQ(use_count_with_lock, 2);
}

TEST_F(AliasingWeakTest, WeakPtrExpiredLock)
{
    std::weak_ptr<Tracked> weak;
    
    {
        std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
        weak = shared;
    }
    
    // Q: At this point, is weak.expired() true or false?
    // A:
    // R:
    
    std::shared_ptr<Tracked> locked_ptr = weak.lock();
    
    bool lock_failed = (locked_ptr == nullptr);
    
    // Q: Why does lock() return nullptr when the weak_ptr is expired?
    // A:
    // R:
    
    // Q: Is it safe to call lock() on an expired weak_ptr?
    // A:
    // R:
    
    // Q: What would happen if you tried to dereference locked_ptr without checking for nullptr?
    // A:
    // R:
    
    EXPECT_TRUE(lock_failed);
}

TEST_F(AliasingWeakTest, AliasingWithWeakPtr)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };
    
    std::weak_ptr<Tracked> weak;
    
    {
        std::shared_ptr<Container> owner = std::make_shared<Container>("Container3");
        std::shared_ptr<Tracked> alias(owner, &owner->member);
        weak = alias;
        
        // Q: What is the use_count of the control block at this point?
        // A:
        // R:
        
        // Q: Does weak point to the Tracked member or the Container?
        // A:
        // R:
        
        owner.reset();
        alias.reset();
        
        // Q: After resetting both owner and alias, what happens to the Container object?
        // A:
        // A:
        // A:
        // A:
        // R:
        
        // Q: Why does resetting alias cause the Container to be destroyed, even though weak still exists?
        // A:
        // R:
    }
    
    bool expired = weak.expired();
    
    // Q: What does it mean for weak to be expired in this context?
    // A:
    // R:
    
    EXPECT_TRUE(expired);
}

TEST_F(AliasingWeakTest, WeakPtrUseCount)
{
    long shared_count = 0;
    long weak_count = 0;
    
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
    std::weak_ptr<Tracked> weak = shared;
    
    shared_count = shared.use_count();
    weak_count = weak.use_count();
    
    // Q: Why does weak.use_count() return 1, even though weak_ptr doesn't contribute to ownership?
    // A:
    // R:
    
    // Q: What does weak.use_count() actually report?
    // A:
    // R:
    
    // Q: If you create another weak_ptr from shared, what will weak.use_count() return?
    // A:
    // R:
    
    EXPECT_EQ(shared_count, 1);
    EXPECT_EQ(weak_count, 1);
}

TEST_F(AliasingWeakTest, MultipleWeakPtrs)
{
    long use_count = 0;
    
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
    std::weak_ptr<Tracked> weak1 = shared;
    std::weak_ptr<Tracked> weak2 = shared;
    std::weak_ptr<Tracked> weak3 = shared;
    
    use_count = shared.use_count();
    
    // Q: Why does the use_count remain 1 even with three weak_ptrs?
    // A:
    // R:
    
    // Q: What counter in the control block is incremented when you create weak_ptrs?
    // A:
    // R:
    
    // Q: If you destroy shared, will the control block be deallocated immediately?
    // A:
    // R:
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(AliasingWeakTest, WeakPtrCopy)
{
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
    std::weak_ptr<Tracked> weak1 = shared;
    std::weak_ptr<Tracked> weak2 = weak1;
    
    // Q: What is the weak reference count in the control block after creating weak2?
    // A:
    // R:
    
    std::shared_ptr<Tracked> locked1 = weak1.lock();
    std::shared_ptr<Tracked> locked2 = weak2.lock();
    bool both_valid = (locked1 != nullptr) && (locked2 != nullptr);
    
    // Q: What is the use_count of shared after locking both weak_ptrs?
    // A:
    // R:
    
    // Q: Do locked1 and locked2 point to the same object?
    // A:
    // R:
    
    EXPECT_TRUE(both_valid);
}

TEST_F(AliasingWeakTest, WeakPtrReset)
{
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
    std::weak_ptr<Tracked> weak = shared;
    
    bool expired_before = weak.expired();
    
    // Q: What is the weak reference count before calling reset()?
    // A:
    // R:
    
    weak.reset();
    
    bool expired_after = weak.expired();
    
    // Q: After calling weak.reset(), does the shared_ptr still own the object?
    // A:
    // R:
    
    // Q: What is the weak reference count after calling reset()?
    // A:
    // R:
    
    // Q: What is the difference between a reset weak_ptr and a default-constructed weak_ptr?
    // A:
    // R:
    
    EXPECT_FALSE(expired_before);
    EXPECT_TRUE(expired_after);
}
