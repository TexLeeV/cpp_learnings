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
    // A: Because alias is a shared_ptr to Container, which still increments use_count by 1
    // R: Correct. The aliasing constructor creates a shared_ptr that shares ownership of the Container (incrementing use_count) but stores a pointer to the member. Both owner and alias reference the same control block.
    
    // Q: What object will be deleted when the last shared_ptr (owner or alias) is destroyed?
    // A: I misread this.  I would have definitely said Container object
    // R: Correct! The Container object will be deleted when the last shared_ptr is destroyed. The control block's deleter points to the Container, so regardless of whether owner or alias is destroyed last, the Container gets deleted when use_count reaches 0.
    
    // Q: What happens if you call alias.get()? What pointer value does it return?
    // A: You get the Tracked member object
    // R: Correct. alias.get() returns the address of owner->member (&owner->member), which is the Tracked object. The aliasing constructor stored this pointer, not the Container pointer.
    
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
        // A: Because alias incremented the use_count to 2.  reset() just decrements it down to 1
        // R: Correct. The aliasing constructor incremented the shared ownership count to 2. When owner.reset() executes, it decrements use_count to 1. Since use_count > 0, the Container remains alive.
        
        // Q: What is the use_count of alias at this point?
        // A: 1
        // R: Correct. After owner.reset(), only alias holds a reference to the control block, so use_count is 1.
    }
    
    // Q: The inner scope has ended. Why hasn't the Container been destroyed yet?
    // A: Because alias is still alive at this scope
    // R: Correct. alias exists in the outer scope and still holds ownership (use_count = 1). The Container won't be destroyed until alias is destroyed or reset.
    
    // Q: What will happen when alias goes out of scope at the end of this test?
    // A: Then Container will be deleted.  Both the control block and object
    // R: Correct. When alias goes out of scope, use_count drops to 0, triggering deletion of the Container object. The control block is also deallocated (assuming no weak_ptrs exist).
    
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
    // A: Because it doesn't reference any valid shared_ptr control_block memory
    // R: Correct. A default-constructed weak_ptr has no control block pointer (nullptr), so expired() returns true. It doesn't observe any object.
    
    {
        std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
        weak = shared;
        
        // Q: What is the use_count of shared at this point?
        // A: 1
        // R: Correct. Only shared owns the object. Assigning to weak doesn't increment use_count; it only increments the weak reference count in the control block.
        
        // Q: What is the use_count reported by weak at this point?
        // A: 1.  expired() will return false at this point
        // R: Correct. weak.use_count() returns the shared ownership count (1), not the weak count. And yes, expired() returns false because use_count > 0.
    }
    
    expired_after = weak.expired();
    
    // Q: Why is weak expired after the inner scope ends?
    // A: Because shared went out of scope and decrement use_count to 0, deleting its memory
    // R: Correct. When shared goes out of scope, use_count drops to 0, the Tracked object is deleted, and the control block marks the object as expired. weak now observes an expired control block.
    
    // Q: Does weak_ptr prevent the object from being destroyed?
    // A: No, but it does prevent the control block from being destroyed
    // R: Correct. weak_ptr doesn't contribute to use_count, so the object can be destroyed. However, weak_ptr increments the weak count, keeping the control block alive so weak.expired() can be safely checked.
    
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
    // A: 1
    // R: Correct. Only shared owns the object at this point. weak observes but doesn't own.
    
    std::shared_ptr<Tracked> locked = weak.lock();
    
    lock_succeeded = (locked != nullptr);
    use_count_with_lock = shared.use_count();
    
    // Q: Why does the use_count increase to 2 after calling lock()?
    // A: Because shared is still valid and, thus, lock() will return a shared_ptr copy that points to the same object and control block.  Increment use_count by 1 to 2
    // R: Correct. lock() atomically checks if the object is alive (use_count > 0) and, if so, creates a new shared_ptr that shares ownership. This increments use_count from 1 to 2.
    
    // Q: What happens to the object if you destroy shared but keep locked alive?
    // A: Nothing
    // R: Correct. The object remains alive because locked still owns it (use_count would be 1). The object is only destroyed when all shared_ptrs are gone.
    
    // Q: What is the relationship between locked and shared? Do they share the same control block?
    // A: Yes, they do.  I think it would be similar to if this code was run: "std::shared_ptr<Tracked> shared2 = shared;"
    // R: Correct. locked and shared reference the same control block and point to the same object. Your analogy is perfect—lock() is functionally equivalent to copying shared.
    
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
    // A: true
    // R: Correct. The inner scope ended, shared was destroyed, use_count dropped to 0, and the object was deleted. The control block now indicates the object is expired.
    
    std::shared_ptr<Tracked> locked_ptr = weak.lock();
    
    bool lock_failed = (locked_ptr == nullptr);
    
    // Q: Why does lock() return nullptr when the weak_ptr is expired?
    // A: Because the object has been released because use_count is 0 at this point
    // R: Correct. lock() checks the control block's use_count. Since it's 0, the object no longer exists, so lock() cannot create a valid shared_ptr and returns nullptr instead.
    
    // Q: Is it safe to call lock() on an expired weak_ptr?
    // A: Yes.  It will just return a default-constructed shared_ptr<Tracked>, nullptr
    // R: Correct. lock() is designed to be safe on expired weak_ptrs—it returns an empty shared_ptr (nullptr) without undefined behavior. This is the whole point of weak_ptr: safe observation.
    
    // Q: What would happen if you tried to dereference locked_ptr without checking for nullptr?
    // A: segfault
    // R: Correct. Dereferencing a nullptr causes undefined behavior, typically a segmentation fault. Always check if lock() returned a valid pointer before dereferencing.
    
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
        // A: 2
        // R: Correct. owner and alias both share ownership of the Container through the same control block, so use_count = 2.
        
        // Q: Does weak point to the Tracked member or the Container?
        // A: Container
        // R: Not quite. weak observes the control block (which owns the Container), but weak's stored pointer points to the Tracked member because it was assigned from alias. If you called weak.lock().get(), you'd get the member's address, not the Container's.
        
        owner.reset();
        alias.reset();
        
        // Q: After resetting both owner and alias, what happens to the Container object?
        // A: Which sentence is most accurate or makes the most sense:
        // A: 1. Its object deleted
        // A: 2. Its object memory is released
        // A: 3. Its object memory is freed
        // R: All three are essentially correct and commonly used. "The object is deleted" (1) is most precise in C++ because it implies the destructor runs. "Memory is released/freed" (2/3) emphasize deallocation. In practice, all three happen: destructor runs, then memory is freed.
        
        // Q: Why does resetting alias cause the Container to be destroyed, even though weak still exists?
        // A: Because that will decrement the use_count to 0.  While the weak counter will still be 1, keeping the control block alive
        // R: Correct. Resetting both owner and alias drops use_count to 0, triggering Container deletion. weak_ptr doesn't prevent object destruction—it only keeps the control block alive (weak count = 1) so expired() can be checked.
    }
    
    bool expired = weak.expired();
    
    // Q: What does it mean for weak to be expired in this context?
    // A: That Container object is no longer valid.  i.e. use_count = 0
    // R: Correct. expired() returns true because use_count = 0 and the Container has been destroyed. The control block still exists (for weak to query), but the object is gone.
    
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
    // A: Because of shared
    // R: Correct. weak.use_count() reports the number of shared_ptr owners in the control block, not the weak count. Since shared is the only owner, use_count = 1.
    
    // Q: What does weak.use_count() actually report?
    // A: It returns the same as shared_ptr::use_count()
    // R: Correct. Both weak.use_count() and shared.use_count() query the same counter in the control block—the shared ownership count.
    
    // Q: If you create another weak_ptr from shared, what will weak.use_count() return?
    // A: Still 1
    // R: Correct. Creating more weak_ptrs increments the weak count, not use_count. The shared ownership count remains 1 because only shared owns the object.
    
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
    // A: Redundant question
    // R: Fair observation—this reinforces the previous concept. weak_ptrs observe but don't own, so they don't increment use_count. Only shared_ptrs contribute to ownership.
    
    // Q: What counter in the control block is incremented when you create weak_ptrs?
    // A: The weak counter, not use_count
    // R: Correct. The control block maintains two counters: use_count (shared owners) and weak_count (weak observers). Creating weak_ptrs increments weak_count.
    
    // Q: If you destroy shared, will the control block be deallocated immediately?
    // A: No, not until weak1, weak2, and weak3 are released from the stack
    // R: Correct. When shared is destroyed, the object is deleted (use_count → 0), but the control block remains alive because weak_count = 3. The control block is deallocated only when both use_count and weak_count reach 0.
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(AliasingWeakTest, WeakPtrCopy)
{
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
    std::weak_ptr<Tracked> weak1 = shared;
    std::weak_ptr<Tracked> weak2 = weak1;
    
    // Q: What is the weak reference count in the control block after creating weak2?
    // A: 2
    // R: Correct. weak1 and weak2 both observe the same control block, so weak_count = 2.
    
    std::shared_ptr<Tracked> locked1 = weak1.lock();
    std::shared_ptr<Tracked> locked2 = weak2.lock();
    bool both_valid = (locked1 != nullptr) && (locked2 != nullptr);
    
    // Q: What is the use_count of shared after locking both weak_ptrs?
    // A: 3
    // R: Correct. Initially use_count = 1 (shared). Each lock() creates a new shared_ptr owner, incrementing use_count: locked1 → 2, locked2 → 3.
    
    // Q: Do locked1 and locked2 point to the same object?
    // A: Yes, they point to the same object and control block
    // R: Correct. Both weak_ptrs observe the same control block and object, so their lock() calls produce shared_ptrs to the same Tracked object.
    
    EXPECT_TRUE(both_valid);
}

TEST_F(AliasingWeakTest, WeakPtrReset)
{
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
    std::weak_ptr<Tracked> weak = shared;
    
    bool expired_before = weak.expired();
    
    // Q: What is the weak reference count before calling reset()?
    // A: 1
    // R: Correct. Only weak observes the control block, so weak_count = 1.
    
    weak.reset();
    
    bool expired_after = weak.expired();
    
    // Q: After calling weak.reset(), does the shared_ptr still own the object?
    // A: I misread this, I would have said yes because I need to leave in a couple of minutes.  Excuse but it is honest
    // R: Correct! Yes, shared still owns the object (use_count = 1). Calling weak.reset() only affects weak—it stops observing the control block (weak_count → 0) but doesn't affect shared's ownership at all.
    
    // Q: What is the weak reference count after calling reset()?
    // A: 0
    // R: Correct. weak.reset() decrements weak_count from 1 to 0. The control block still exists because shared owns the object (use_count = 1).
    
    // Q: What is the difference between a reset weak_ptr and a default-constructed weak_ptr?
    // A: There are functionally equivalent
    // R: Correct. Both have no control block pointer (nullptr) and expired() returns true for both. They're functionally identical.
    
    EXPECT_FALSE(expired_before);
    EXPECT_TRUE(expired_after);
}
