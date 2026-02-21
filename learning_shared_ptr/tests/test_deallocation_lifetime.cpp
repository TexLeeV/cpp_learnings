#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class DeallocationLifetimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(DeallocationLifetimeTest, BasicDestructionOrder)
{
    {
        std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Obj1");
        std::shared_ptr<Tracked> p2 = std::make_shared<Tracked>("Obj2");
        
        // Q: At what point in the code will Obj1's destructor be called?
        // A: When it goes out of scope or .reset() is called
        // R: Correct. More precisely, when the closing brace on line 31 is reached, p1's destructor runs, decrements the reference count to 0, and invokes Obj1's destructor.
        // R: For junior SWEs: When p1 goes out of scope at the closing brace, Obj1 is destroyed.
        
        // Q: At what point in the code will Obj2's destructor be called?
        // A: When it goes out of scope or .reset() is called
        // R: Correct. Same mechanism as Obj1—when p2 goes out of scope at the closing brace.
        // R: For junior SWEs: When p2 goes out of scope at the closing brace, Obj2 is destroyed.
        
        // Q: Which object will be destroyed first, Obj1 or Obj2, and why?
        // A: Obj2, due to how popping(unwinding) the stack
        // R: Correct. Automatic objects are destroyed in reverse order of construction. p2 was constructed after p1, so p2 is destroyed first, triggering Obj2's destructor before Obj1's.
        // R: For junior SWEs: p2 was created second, so it's destroyed first. Stack objects are destroyed in reverse order.
        
        // Q: The test expects 4 events. What are these 4 events?
        // A: p1 ctor, p2 ctor, p2 dtor p1 dtor
        // R: Correct. The EventLog captures: Obj1 constructor, Obj2 constructor, Obj2 destructor, Obj1 destructor.
        // R: For junior SWEs: Two constructors when created, two destructors when destroyed.
    }
    
    auto events = EventLog::instance().events();
    size_t event_count = events.size();
    
    EXPECT_EQ(event_count, 4);
}

TEST_F(DeallocationLifetimeTest, CustomDeleter)
{
    {
        std::shared_ptr<Tracked> p1(new Tracked("Obj1"), LoggingDeleter<Tracked>("CustomDeleter"));
        
        // Q: When will the custom deleter be invoked?
        // A: When p1 goes out of this scope
        // R: Correct. When p1's destructor runs at the closing brace, the reference count reaches 0, and the control block invokes the stored LoggingDeleter.
        // R: For junior SWEs: When p1 goes out of scope, the custom deleter is called to clean up the object.
        
        // Q: What is the difference between the deleter stored in the control block and the default deleter used by make_shared?
        // A: The are stored the same, I believe.  Just one is custom provided and the default one is implicitly provided
        // R: Conceptually correct understanding. Technically: make_shared uses a default deleter (typically just `delete`) that's part of the control block's type. A custom deleter is stored as a callable object in the control block and can have arbitrary cleanup logic. Both are invoked when the reference count reaches 0, but custom deleters add storage overhead and type-erase the deleter.
        // R: For junior SWEs: make_shared uses a simple delete. Custom deleters can do anything you want and are stored in the control block.
        
        // Q: Can you change the deleter after the shared_ptr is constructed?
        // A: No
        // R: Correct. The deleter is fixed at construction and stored in the control block. You cannot modify it later. You can only replace the entire managed object and control block via reset().
        // R: For junior SWEs: The deleter is locked in when you create the shared_ptr and cannot be changed.
    }
    
    auto events = EventLog::instance().events();
    bool deleter_called = false;
    
    for (const auto& event : events)
    {
        if (event.find("CustomDeleter::operator()") != std::string::npos)
        {
            deleter_called = true;
            break;
        }
    }
    
    EXPECT_TRUE(deleter_called);
}

TEST_F(DeallocationLifetimeTest, ArrayDeleter)
{
    {
        std::shared_ptr<Tracked> p1(new Tracked[3]{Tracked("D1"), Tracked("D2"), Tracked("D3")}, LoggingArrayDeleter<Tracked>("ArrayDeleter"));
        
        // Q: Why is a custom deleter necessary when managing arrays with shared_ptr?
        // A: Because the array is created by new keyword rather than with make_shared and compiler cannot provide the delete functionality implicitly
        // R: Correct understanding. More precisely: shared_ptr's default deleter uses `delete`, not `delete[]`. For arrays allocated with `new[]`, you must provide a custom deleter that calls `delete[]`, otherwise you invoke undefined behavior (mismatched new[]/delete).
        // R: For junior SWEs: Arrays need delete[] but shared_ptr uses delete by default. You must provide a custom deleter that uses delete[].
        
        // Q: What would happen if you used the default deleter (delete) instead of delete[] for an array?
        // A: It will probably delete just the first "D1" Tracked object
        // R: Correct intuition about partial destruction. Technically: this is undefined behavior. Typically only the first element's destructor runs, leaking the other two elements and potentially corrupting memory. The behavior is not guaranteed—it could crash, leak, or appear to work.
        // R: For junior SWEs: Using delete on an array is undefined behavior. Usually only the first element is destroyed and the rest leak.
        
        // Q: In what order will the three array elements be destroyed?
        // A: D1 -> D2 -> D3
        // R: Close, but reversed. Array elements are destroyed in reverse order of construction: D3 → D2 → D1. This matches the general C++ rule that objects are destroyed in reverse order of construction.
        // R: For junior SWEs: Arrays are destroyed backwards. Last element first: D3, then D2, then D1.
    }
    
    auto events = EventLog::instance().events();
    bool array_deleter_called = false;
    
    for (const auto& event : events)
    {
        if (event.find("ArrayDeleter::operator()") != std::string::npos)
        {
            array_deleter_called = true;
            break;
        }
    }
    
    EXPECT_TRUE(array_deleter_called);
}

TEST_F(DeallocationLifetimeTest, ResetWithCustomDeleter)
{
    std::shared_ptr<Tracked> p1(new Tracked("Obj1"), LoggingDeleter<Tracked>("FirstDeleter"));
    
    EventLog::instance().clear();
    
    p1.reset(new Tracked("Obj2"), LoggingDeleter<Tracked>("SecondDeleter"));
    
    // Q: Which deleter was invoked when reset() was called, FirstDeleter or SecondDeleter?
    // A: FirstDeleter
    // R: Correct. reset() decrements the reference count for the first object to 0, triggering FirstDeleter to destroy Obj1 and deallocate the first control block.
    // R: For junior SWEs: FirstDeleter runs when reset() is called because the old object is being replaced.
    
    // Q: What happened to the control block associated with the first object?
    // A: It was released
    // R: Correct. The first control block (containing FirstDeleter) was deallocated after FirstDeleter executed. p1 now references a completely new control block containing SecondDeleter.
    // R: For junior SWEs: The old control block is deleted. p1 gets a new control block for the new object.
    
    // Q: After reset(), which deleter is stored in p1's control block?
    // A: SecondDeleter
    // R: Correct. reset() creates a new control block for the new object, and SecondDeleter is stored in that new control block.
    // R: For junior SWEs: SecondDeleter is now in p1's new control block.
    
    // Q: When will SecondDeleter be invoked?
    // A: Upon reset or going out of scope
    // R: Correct. SecondDeleter will be invoked when p1's reference count for Obj2 reaches 0—either via another reset(), assignment to nullptr, or when p1 goes out of scope at the end of the test function.
    // R: For junior SWEs: SecondDeleter runs when p1 is reset again or when p1 goes out of scope.
    
    auto events = EventLog::instance().events();
    bool first_deleter_called = false;
    
    for (const auto& event : events)
    {
        if (event.find("FirstDeleter::operator()") != std::string::npos)
        {
            first_deleter_called = true;
            break;
        }
    }
    
    EXPECT_TRUE(first_deleter_called);
}

TEST_F(DeallocationLifetimeTest, SharedOwnershipDeleterInvocation)
{
    std::shared_ptr<Tracked> p1(new Tracked("SharedObj"), LoggingDeleter<Tracked>("SharedDeleter"));
    
    std::shared_ptr<Tracked> p2 = p1;
    std::shared_ptr<Tracked> p3 = p1;
    
    // Q: How many shared_ptr instances share ownership of the object at this point?
    // A: 3
    // R: Correct. p1, p2, and p3 all share ownership of SharedObj. The control block's reference count is 3.
    // R: For junior SWEs: Three shared_ptr instances point to the same object.
    
    // Q: How many copies of the deleter exist?
    // A: 1
    // R: Correct. The deleter is stored once in the shared control block. All three shared_ptr instances reference the same control block, which contains the single LoggingDeleter instance.
    // R: For junior SWEs: Only one deleter exists in the control block, shared by all three pointers.
    
    EventLog::instance().clear();
    
    p1.reset();
    
    // Q: Why wasn't the deleter invoked when p1 was reset?
    // A: use_count goes to 2
    // R: Correct. The reference count decremented from 3 to 2, but hasn't reached 0 yet. The deleter is only invoked when the reference count reaches 0. p2 and p3 still hold references, keeping the object alive.
    // R: For junior SWEs: The deleter only runs when the reference count hits 0. It's still 2 because p2 and p3 are alive.
    
    // Q: What is the reference count after p1.reset()?
    // A: 2
    // R: Correct. p2 and p3 remain, so the reference count is 2.
    // R: For junior SWEs: Two pointers left, so reference count is 2.
    
    auto events_after_first = EventLog::instance().events();
    bool deleter_called_early = false;
    
    for (const auto& event : events_after_first)
    {
        if (event.find("SharedDeleter::operator()") != std::string::npos)
        {
            deleter_called_early = true;
            break;
        }
    }
    
    p2.reset();
    p3.reset();
    
    // Q: After which reset() call was the deleter invoked?
    // A: p3
    // R: Correct. p2.reset() decremented the count from 2 to 1. p3.reset() decremented from 1 to 0, triggering the deleter.
    // R: For junior SWEs: The second reset (p3) brought the count to 0, so the deleter ran then.
    
    // Q: What determines when the deleter stored in the control block is invoked?
    // A: When use_count goes to 0
    // R: Correct. The control block invokes the deleter when the strong reference count (use_count) reaches 0. This is the fundamental rule for shared_ptr cleanup.
    // R: For junior SWEs: The deleter runs when the reference count hits 0.
    
    auto events_after_all = EventLog::instance().events();
    bool deleter_called_final = false;
    
    for (const auto& event : events_after_all)
    {
        if (event.find("SharedDeleter::operator()") != std::string::npos)
        {
            deleter_called_final = true;
            break;
        }
    }
    
    EXPECT_FALSE(deleter_called_early);
    EXPECT_TRUE(deleter_called_final);
}

TEST_F(DeallocationLifetimeTest, AliasingConstructorLifetime)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };
    
    EventLog::instance().clear();
    
    {
        std::shared_ptr<Container> owner(new Container("ContainerObj"), LoggingDeleter<Container>("ContainerDeleter"));
        
        std::shared_ptr<Tracked> alias(owner, &owner->member);
        
        // Q: What does alias point to?
        // A: owner->member but the control block uses use_count against Container("ContainerObj")
        // R: Correct. alias.get() returns a pointer to the Tracked member, but alias shares owner's control block, which manages the Container's lifetime. This is the aliasing constructor's key feature: pointer to one thing, ownership of another.
        // R: For junior SWEs: alias points to the member but shares ownership of the whole Container.
        
        // Q: Which control block does alias share with owner?
        // A: Container("ContainerObj")
        // R: Correct understanding. More precisely: alias shares the control block that manages the Container. Both owner and alias increment the same reference count in that control block.
        // R: For junior SWEs: alias and owner share the same control block that manages the Container.
        
        // Q: Which deleter will be invoked when the last reference is released, the one for Container or the one for Tracked?
        // A: Container
        // R: Correct. The control block stores ContainerDeleter, which deletes the Container. The aliasing constructor doesn't change the deleter—it only changes what alias.get() returns. The stored pointer type (Tracked*) is separate from the owned object type (Container).
        // R: For junior SWEs: ContainerDeleter runs because the control block manages the Container, not the member.
        
        owner.reset();
        
        // Q: Why wasn't the Container deleted when owner was reset?
        // A: Because alias has ownership of Container
        // R: Correct. alias shares the control block, so the reference count only decremented from 2 to 1. The Container remains alive because alias still holds a reference.
        // R: For junior SWEs: alias is still alive and shares ownership, so the Container stays alive.
        
        // Q: What is keeping the Container alive?
        // A: alias keeping use_count to 1
        // R: Correct. alias holds the last reference to the shared control block, keeping the reference count at 1 and preventing deletion.
        // R: For junior SWEs: alias keeps the reference count at 1, so the Container stays alive.
        
        // Q: What happens to the Tracked member when the Container is eventually deleted?
        // A: It also gets deleted in Container's dtor
        // R: Correct. When alias goes out of scope, ContainerDeleter destroys the Container. The Container's destructor then destroys its member (the Tracked object) as part of normal member destruction.
        // R: For junior SWEs: When the Container is destroyed, its member is automatically destroyed too.
        
        auto events_after_owner_reset = EventLog::instance().events();
        bool container_deleted = false;
        
        for (const auto& event : events_after_owner_reset)
        {
            if (event.find("ContainerDeleter::operator()") != std::string::npos)
            {
                container_deleted = true;
                break;
            }
        }
        
        EXPECT_FALSE(container_deleted);
    }
    
    auto events_final = EventLog::instance().events();
    bool container_deleted_final = false;
    
    for (const auto& event : events_final)
    {
        if (event.find("ContainerDeleter::operator()") != std::string::npos)
        {
            container_deleted_final = true;
            break;
        }
    }
    
    EXPECT_TRUE(container_deleted_final);
}

TEST_F(DeallocationLifetimeTest, NullptrReset)
{
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Obj1");
    
    EventLog::instance().clear();
    
    p1.reset();
    
    // Q: What does p1 point to after reset() with no arguments?
    // A: nullptr
    // R: Correct. reset() with no arguments releases the managed object and sets p1 to an empty state (equivalent to nullptr).
    // R: For junior SWEs: After reset(), p1 points to nothing (nullptr).
    
    // Q: What happened to the object that p1 was managing?
    // A: It memory is released
    // R: Correct. The reference count reached 0, triggering Obj1's destructor and deallocating both the object and the control block.
    // R: For junior SWEs: The object was destroyed and its memory freed.
    
    // Q: Is reset() with no arguments equivalent to p1 = nullptr?
    // A: No.  reset() atomically updates its use_count and other things depending on the atomic values it manages
    // R: Actually, they are functionally equivalent for single-threaded code. Both decrement the reference count and set p1 to empty. The difference is stylistic: reset() is more explicit about intent. In thread-safe implementations, both use atomic operations on the control block. The assignment operator calls reset() internally.
    // R: For junior SWEs: They do the same thing. Both release the object and set p1 to nullptr.
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    EXPECT_EQ(dtor_count, 1);
}

TEST_F(DeallocationLifetimeTest, MultipleResetsInSequence)
{
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Initial");
    
    EventLog::instance().clear();
    
    p1.reset(new Tracked("First"));
    p1.reset(new Tracked("Second"));
    p1.reset(new Tracked("Third"));
    
    // Q: How many objects were destroyed during the three reset() calls?
    // A: 3
    // R: Correct. Each reset() call destroyed the previously managed object: "Initial" (from the first reset), "First" (from the second reset), and "Second" (from the third reset).
    // R: For junior SWEs: Three objects destroyed, one for each reset() call.
    
    // Q: When was each object destroyed?
    // A: Upon each reset call
    // R: Correct. Each reset() immediately decrements the reference count to 0 for the old object, triggering its destructor before the new object is assigned.
    // R: For junior SWEs: Each object is destroyed right when reset() is called.
    
    // Q: After the three reset() calls, which object does p1 manage?
    // A: Tracked("Third")
    // R: Correct. The last reset() assigned "Third" to p1, so that's what p1 currently manages.
    // R: For junior SWEs: p1 now manages "Third".
    
    // Q: What would happen if you added a fourth reset() with no arguments?
    // A: Tracked("Third") would be deleted and then p1 points to nullptr and has control block memory released
    // R: Correct. reset() with no arguments would destroy "Third", deallocate its control block, and leave p1 in an empty state (pointing to nullptr with no control block).
    // R: For junior SWEs: "Third" would be destroyed and p1 would become empty (nullptr).
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    EXPECT_EQ(dtor_count, 3);
}
