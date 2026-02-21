#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class ReferenceCountingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(ReferenceCountingTest, BasicCreationAndDestruction)
{
    long initial_count = 0;
    long after_creation_count = 0;
    long after_copy_count = 0;
    long after_scope_exit_count = 0;
    
    {
        // TODO: Create a shared_ptr<Tracked> named p1 with a new Tracked("A")
        std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("A");
        
        // TODO: Capture the use_count of p1
        after_creation_count = p1.use_count();
        EXPECT_EQ(after_creation_count, 1);
        
        {
            // TODO: Create p2 by copying p1
            std::shared_ptr<Tracked> p2 = p1;
            
            // TODO: Capture the use_count of p1 after copying
            after_copy_count = p1.use_count();
            EXPECT_EQ(after_copy_count, 2);
            EXPECT_EQ(p2.use_count(), after_copy_count);
        }
        
        // TODO: Capture the use_count of p1 after p2 goes out of scope
        after_scope_exit_count = p1.use_count();
        EXPECT_EQ(after_scope_exit_count, 1);
    }

    EXPECT_EQ(after_creation_count, 1);
    EXPECT_EQ(after_copy_count, 2);
    EXPECT_EQ(after_scope_exit_count, 1);
}

TEST_F(ReferenceCountingTest, MoveSemantics)
{
    long count_before_move = 0;
    long count_after_move_source = 0;
    long count_after_move_dest = 0;
    
    // TODO: Create p1 with a new Tracked("B")
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("B");
    
    // TODO: Capture use_count before move
    count_before_move = p1.use_count(); // 1
    
    // TODO: Move p1 into p2 using std::move
    std::shared_ptr<Tracked> p2 = std::move(p1); // still 1
    
    // TODO: Capture use_count of p1 (source) after move
    count_after_move_source = p1.use_count(); // 0
    
    // TODO: Capture use_count of p2 (destination) after move
    count_after_move_dest = p2.use_count(); // 1
    
    EXPECT_EQ(count_before_move, 1);
    EXPECT_EQ(count_after_move_source, 0);
    EXPECT_EQ(count_after_move_dest, 1);
}

TEST_F(ReferenceCountingTest, AliasingConstructor)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        // Q: What does explicit prevent in this context?
        // A: Prevents implicit conversion of an empty name string being passed through
        // R: Close. `explicit` prevents implicit construction of Container from std::string.
        //    Without it: void f(Container c) {}; f("hello"); would compile (string literal -> std::string -> Container).
        //    With it: Must write f(Container("hello"));
        //
        // Q: Without explicit, what implicit conversion would be allowed?
        // A: Container owner2 = std::string(); // Is there a way to do this with the std::shared_ptr?
        // R: Yes. Without explicit: void takeContainer(Container c) {}; takeContainer("hello"); would compile.
        //    The real impact is in function calls and assignments where implicit conversion occurs.
        : member(name)
        {
        }
    };
    
    long owner_count = 0;
    long alias_count = 0;
    long both_alive_owner_count = 0;
    long both_alive_alias_count = 0;
    long after_owner_reset_alias_count = 0;
    
    // TODO: Create owner shared_ptr to Container
    std::shared_ptr<Container> owner = std::make_shared<Container>("MyContainer");
    
    // TODO: Capture owner's use_count
    owner_count = owner.use_count();
    // Q: How many shared_ptr instances share ownership of the Container at this point?
    // A: 1
    // R: Correct.
    
    // TODO: Create alias shared_ptr using aliasing constructor
    // Hint: std::shared_ptr<Tracked> alias(owner, &owner->member);
    std::shared_ptr<Tracked> alias(owner, &owner->member);
    // Q: This constructor takes two arguments: a shared_ptr and a raw pointer.
    //    Which argument determines what gets destroyed when use_count reaches 0?
    // A: The owner argument
    // R: Correct. The first argument (owner) determines the control block, which controls destruction.
    //
    // Q: Which argument determines what alias.get() returns?
    // A: The member argument
    // R: Correct. The second argument (&owner->member) determines what alias.get() returns.
    //
    // Q: The member is not a pointer—it's a Tracked object by value inside Container.
    //    How can you take &owner->member and store it in a shared_ptr without double-deletion?
    // A: By making it a shared_ptr?  Does the original implementation have the possibility of undefined behavior?
    // R: The aliasing constructor does NOT create a new shared_ptr to member. It creates a shared_ptr that:
    //    - Points to &owner->member (a raw pointer)
    //    - Shares the control block with owner
    //    - Prevents double-deletion because the control block only deletes the Container, not member
    //    When use_count reaches 0, the control block deletes the entire Container object, which includes member.
    //    There's no separate deletion of member, so no double-deletion is possible.
    //    Q: If alias held its own control block for member, what would happen when both owner and alias reach use_count == 0?
    //    A: Would result in a segmentation violation from a double-deletion
    //    R: Correct. If alias had its own control block:
    //       1. When owner reaches use_count == 0: Deletes entire Container (including member)
    //       2. When alias reaches use_count == 0: Attempts to delete member again → undefined behavior (likely segfault)
    //       This is why the aliasing constructor is critical for subobject lifetime management.
    
    // TODO: Capture alias use_count right after creation
    alias_count = alias.use_count();
    // Q: The aliasing constructor at line 107 took `owner` as its first argument.
    //    Does this create a new control block, or does it share owner's control block?
    // A: It shares the control block.  Is this true for all use cases of shared pointers?
    // R: No, only for the aliasing constructor. Other construction methods:
    //    - std::make_shared<T>() — creates NEW control block
    //    - std::shared_ptr<T>(new T) — creates NEW control block
    //    - Copy constructor — SHARES existing control block
    //    - Move constructor — TRANSFERS existing control block
    //    Q: What would happen if you wrote std::shared_ptr<Tracked> alias2(owner.get())? Would it share owner's control block?
    //    A: No, because you're returning a raw pointer, not the necessary Metadata for tracking ownership
    //    R: Correct reasoning. More precisely:
    //       - owner.get() returns a raw pointer with no control block information
    //       - std::shared_ptr<Tracked>(owner.get()) creates a new, independent control block
    //       - Now you have two control blocks managing the same object
    //       - When either reaches use_count == 0, it deletes the object
    //       - When the other reaches 0, it attempts deletion again → double-deletion
    //       Q: What observable behavior would confirm this double-deletion in practice?
    //       A: A segfault, which can simply be done with alias2.reset();
    //
    // Q: After line 107 executes, how many shared_ptr instances reference the same control block?
    // A: 2 — both owner and alias reference the same control block.
    // R: Correct.
    //
    // Q: What does alias.use_count() query—the control block or the pointed-to object?
    // A: The control block
    // R: Correct. use_count() is stored in the control block, not the pointed-to object.
    
    // TODO: Capture both owner and alias use_counts
    both_alive_owner_count = owner.use_count();
    both_alive_alias_count = alias.use_count();
    // Q: owner.use_count() and alias.use_count() both query the same control block.
    //    Between line 107 and line 132, did any shared_ptr go out of scope?
    // A: No — both owner and alias remain in scope.
    // R: Correct.
    //
    // Q: How many shared_ptr instances currently hold a reference to the control block at line 132?
    // A: 2 — owner and alias both hold references.
    // R: Correct.
    //
    // Q: Why do owner and alias return the same use_count value even though they point to different objects?
    // A: Because alias and owner share the control block
    // R: Correct. The control block is the single source of truth for use_count().
    
    // TODO: Reset owner
    owner.reset();
    
    // TODO: Capture alias use_count after owner is reset
    after_owner_reset_alias_count = alias.use_count();
    // Q: After owner.reset(), how many shared_ptr instances remain that reference the control block?
    // A: 1
    // R: Correct. After owner.reset(), only alias holds a reference to the control block.
    //
    // Q: The Container object is still alive at this point. Which shared_ptr is keeping it alive?
    // A: I didn't realize that, since the control block is shared with the alias and resetting the owner doesn't mean that its
    // A: resources are released.  I am just now becoming familiar with the control block concept.
    // R: Excellent insight. This is the key guarantee of the aliasing constructor:
    //    - alias points to &owner->member (a subobject)
    //    - alias shares owner's control block
    //    - When owner.reset(), the control block's use_count decrements from 2 → 1
    //    - The Container object remains alive because alias still holds a reference
    //    - When alias goes out of scope, use_count reaches 0, and the entire Container is destroyed
    //    Q: What would happen if you dereferenced alias after owner.reset()? Would it be safe?
    //    A: Yes, the resources are never released since alias claims ownership of that memory address space
    //    R: Correct. Dereferencing alias after owner.reset() is completely safe because:
    //       - alias still holds a reference to the control block (use_count == 1)
    //       - The Container object remains alive
    //       - alias.get() still points to valid memory (&Container::member)
    //       This is the aliasing constructor's lifetime extension guarantee.
    //
    //    Q: What observable signal in the event log would confirm that the Container is destroyed only after alias goes out of scope?
    //    A: oss << "Tracked(" << name_ << ")::dtor [id=" << id_ << "]";
    //    A: The answer is "alias"
    
    EXPECT_EQ(owner_count, 1);
    EXPECT_EQ(alias_count, 2);
    EXPECT_EQ(both_alive_owner_count, 2);
    EXPECT_EQ(both_alive_alias_count, 2);
    EXPECT_EQ(after_owner_reset_alias_count, 1);
}

TEST_F(ReferenceCountingTest, ResetBehavior)
{
    long initial_count = 0;
    long after_reset_empty_count = 0;
    long after_reset_new_count = 0;
    
    // TODO: Create p1 and p2 (copy of p1)
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Tracked1");
    std::shared_ptr<Tracked> p2 = p1;
    // Q: After line 229, how many shared_ptr instances share ownership of Tracked("Tracked1")?
    // A: 2
    // R: Correct.
    //
    // Q: What is p2's use_count() at this point?
    // A: 2
    // R: Correct.
    
    // TODO: Capture initial use_count
    initial_count = p1.use_count(); // 2
    // Q: Why do both p1 and p2 return use_count() == 2?
    // A: Because they shared the same control block
    // R: Correct. More precisely: "Because they share the same control block" (present tense).
    //    Both p1 and p2 hold pointers to the same control block, which stores use_count == 2.
    
    // TODO: Reset p1 (no argument - makes it empty)
    p1.reset(); // 1
    // Q: After p1.reset(), what happens to the Tracked("Tracked1") object?
    // A: Nothing, its resource is still alive from p2
    // R: Conceptually correct. More precise terminology: "Nothing, the object remains alive because p2 still holds a reference."
    //    "Resource" typically refers to memory/handles; here we're talking about the object's lifetime.
    //
    // Q: What is p2.use_count() after p1.reset()?
    // A: 1
    // R: Correct. The control block's reference count decremented from 2 to 1.
    //
    // Q: What does p1.get() return after reset()?
    // A: nullptr
    // R: Correct. p1 is now in the empty/null state.
    
    // TODO: Capture p1's use_count after reset
    after_reset_empty_count = p1.use_count(); // 0
    // Q: Why does an empty shared_ptr return use_count() == 0?
    // A: Because it doesn't point to a valid memory address space, nullptr
    // Follow up: Is saying the above answer technically accurate?
    // R: Conceptually correct, but terminology needs precision:
    //    More accurate: "Because it has no control block pointer (nullptr)."
    //    use_count() queries the control block. When the control block pointer is null, use_count() returns 0 by specification.
    //    The stored pointer (get()) is also nullptr, but use_count() specifically checks the control block pointer.
    //
    // Q: Is p1 now equivalent to a default-constructed shared_ptr<Tracked>()?
    // A: No, it does not contain anything data within it
    // R: Your answer is confusing. The correct answer is: **Yes, p1 is now equivalent to default-constructed shared_ptr<Tracked>().**
    //    After reset(), p1 is in the same state as if you wrote: shared_ptr<Tracked> p1;
    //    Both have: control block pointer == nullptr, stored pointer == nullptr, use_count() == 0.
    //    Your phrase "does not contain anything data within it" is true, but that's also true of default-constructed shared_ptr.
    // RQ: Explain the concept of "p1 is now equivalent to default-constructed shared_ptr<Tracked>()" as I don't understand this comment at all.
    // RR: **Technical explanation:**
    //     A shared_ptr has two internal pointers:
    //     1. Pointer to the control block (holds use_count, deleter, etc.)
    //     2. Stored pointer (what get() returns)
    //
    //     Default construction: shared_ptr<Tracked> p1;
    //     - Control block pointer: nullptr
    //     - Stored pointer: nullptr
    //     - use_count(): 0 (by specification when control block is null)
    //
    //     After p1.reset() (where p1 previously owned an object):
    //     - Control block pointer: nullptr (released the previous control block)
    //     - Stored pointer: nullptr
    //     - use_count(): 0
    //
    //     Both states are identical. The question asks: "Is the state after reset() the same as never having owned anything?"
    //     Answer: Yes. reset() returns p1 to the same state as if it was just declared without initialization.
    //
    //     **For junior SWEs:**
    //     After reset(), p1 has no control block and no stored pointer. This is the same as never initializing p1 in the first place.
    //     Both mean: p1 owns nothing and points to nothing.
    
    // TODO: Reset p1 with a new Tracked("E")
    p1.reset(new Tracked("E"));
    // Q: After this reset, how many Tracked objects exist in total?
    // A: 2, including p2
    // R: Correct. Tracked("Tracked1") (owned by p2) and Tracked("E") (owned by p1).
    //
    // Q: Does p1 now share ownership with p2?
    // A: No, they have differing control blocks
    // R: Correct. p1 has a new control block for Tracked("E"), p2 still has the original control block for Tracked("Tracked1").
    //
    // Q: What would happen if you wrote p1.reset(p2.get()) instead?
    // A: p2 copies its data into p1 and they now share the control block
    // R: **Incorrect.** This is a critical anti-pattern that causes double-deletion:
    //    - p2.get() returns a raw pointer with no control block information
    //    - p1.reset(p2.get()) creates a NEW control block for the same object
    //    - Now two independent control blocks manage the same object
    //    - When p1 reaches use_count == 0: deletes Tracked("Tracked1")
    //    - When p2 reaches use_count == 0: attempts to delete Tracked("Tracked1") again → undefined behavior (crash)
    //    To share ownership, you must copy the shared_ptr itself: p1 = p2; (not p1.reset(p2.get())).
    // RA: I overlooked the .get() part of the code.  If it was p1.reset(p2);, then my statement would be correct

    // TODO: Capture p1's use_count after reset with new object
    after_reset_new_count = p1.use_count(); // 1
    // Q: Why is p1.use_count() == 1 and not 2?
    // A: Because p1 allocated a new Tracked object onto the heap, thus not pointing to the same memory address location as p2 now
    // A: EXPECT_NE(p1, p2); // Should pass
    // R: Conceptually correct, but phrasing is imprecise.
    //    More accurate: "Because p1 has a new control block for a different object. Only p1 references this new control block."
    //    The key isn't the memory address difference—it's that p1 and p2 have separate control blocks.
    //    Your EXPECT_NE observation is correct: p1.get() != p2.get() because they point to different objects.
    //
    //    **For junior SWEs:**
    //    Because p1 and p2 point to different objects. Each object has its own control block with its own use_count.
    //
    // Q: When does Tracked("Tracked1") get destroyed?
    // A: When p2 goes out of scope or p2.reset() is called before going out of scope
    // R: Correct. More precisely: "When p2's destructor runs (at scope exit) or when p2.reset() is called."
    //    Either event decrements the control block's use_count to 0, triggering destruction.
    //
    // Q: When does Tracked("E") get destroyed?
    // A: When p1 goes out of scope or p1.reset() is called before going out of scope
    // R: Correct. Same reasoning as above.
    
    EXPECT_EQ(initial_count, 2);
    EXPECT_EQ(after_reset_empty_count, 0);
    EXPECT_EQ(after_reset_new_count, 1);
}

TEST_F(ReferenceCountingTest, MakesharedVsNewAllocation)
{
    long new_count = 0;
    long makeshared_count = 0;
    
    // TODO: Create p1 using new
    std::shared_ptr<Tracked> p1(new Tracked("Tracked1"));
    // Q: How many heap allocations occur with this construction?
    // A: 2.  One for the new and one for the shared_ptr constructor
    // R: Correct.
    //
    // Q: Where is the Tracked object allocated? Where is the control block allocated?
    // A: They are in separate memory locations. new returns a pointer to the object, shared_ptr allocates its own control block.
    // R: Correct. The object and control block are in separate, non-adjacent heap locations.
    
    // TODO: Capture use_count
    new_count = p1.use_count(); // 1
    // Q: Why is use_count() == 1 for both construction methods?
    // A: Because only one shared pointer that references the memory address location of the Tracked object, p1
    // A: having two allocations, one for the object and one for the control block, does not mean that the use count is 2
    // R: Correct understanding. use_count tracks the number of shared_ptr instances, not the number of allocations.
    //    Only one shared_ptr (p1) exists, so use_count == 1.
    //
    // QA: What is the semantic c++ process flow of the p1 object and control block heap allocations
    // RR: **Construction sequence for shared_ptr<Tracked> p1(new Tracked("Tracked1")):**
    //     1. new Tracked("Tracked1") executes:
    //        - Allocates sizeof(Tracked) bytes on heap
    //        - Calls Tracked constructor
    //        - Returns raw pointer to the Tracked object
    //     2. shared_ptr constructor executes:
    //        - Receives the raw pointer from step 1
    //        - Allocates a control block on heap (separate allocation)
    //        - Initializes control block: use_count = 1, weak_count = 0, deleter = default_delete<Tracked>
    //        - Stores the raw pointer internally
    //     3. Result: Two separate heap allocations, one shared_ptr instance
    //
    //     **For junior SWEs:**
    //     new allocates the object. Then shared_ptr allocates a control block to track ownership. Two allocations total.
    
    // TODO: Create p2 using std::make_shared
    std::shared_ptr<Tracked> p2 = std::make_shared<Tracked>("Tracked2");
    // Q: How many heap allocations occur with make_shared?
    // A: I'm going to guess that it is still 2, since one is for the object and the other for the control block
    // R: **Incorrect.** make_shared performs **1 allocation** that contains both the control block and the object.
    //    Memory layout: [Control Block | Tracked object] ← single contiguous allocation
    //    This is the key optimization of make_shared.
    //
    //    **For junior SWEs:**
    //    make_shared allocates the control block and object together in one allocation.
    //
    // Q: What is the memory layout difference between p1 and p2?
    // A: I don't know.  Provide answer
    // R: **p1 (using new):**
    //    Heap location A: [Tracked object]
    //    Heap location B: [Control block] ← separate, possibly far apart in memory
    //
    //    **p2 (using make_shared):**
    //    Heap location C: [Control block | Tracked object] ← adjacent, single allocation
    //
    //    The adjacency in p2 improves cache locality: accessing the control block (for use_count) and the object
    //    is more likely to hit the CPU cache.
    //
    //    **For junior SWEs:**
    //    p1 has object and control block in separate memory locations. p2 has them next to each other.
    //
    // Q: What exception safety advantage does make_shared provide?
    // A: Probably a noexcept within the API?  I don't really know.  Provide answer
    // R: **Exception safety issue with new:**
    //    Consider: foo(shared_ptr<T>(new T()), bar());
    //    C++ does not guarantee argument evaluation order. Possible sequence:
    //    1. new T() allocates and constructs T
    //    2. bar() is called and throws an exception
    //    3. shared_ptr<T> constructor never runs → T is leaked (no one owns it)
    //
    //    **With make_shared:**
    //    foo(make_shared<T>(), bar());
    //    make_shared is a single function call that completes atomically:
    //    - Either: allocation succeeds, object constructed, shared_ptr created (no leak possible)
    //    - Or: exception thrown during construction, allocation is cleaned up automatically
    //
    //    make_shared eliminates the window where a raw pointer exists without ownership.
    //
    //    **For junior SWEs:**
    //    With new, if an exception happens between allocation and shared_ptr construction, you leak memory.
    //    make_shared prevents this by doing everything in one step.
    
    // TODO: Capture use_count
    makeshared_count = p2.use_count(); // 1
    // Q: Both use_count() values are 1. Does this mean they have identical behavior?
    // A: Yes, they're both shared_ptr's at the variable level
    // R: **Incorrect.** They have the same use_count, but different internal behavior:
    //    - p1: 2 allocations, non-adjacent memory
    //    - p2: 1 allocation, adjacent memory
    //    - p2 has better cache locality and lower allocation overhead
    //    - p2 may delay memory release if weak_ptr exists (control block and object deallocated together)
    //
    //    **For junior SWEs:**
    //    Same use_count, but different performance and memory layout.
    //
    // Q: What performance difference exists between the two construction methods?
    // A: Not sure but I would guess p2 is more performant as it is probably optimized at the compiler level due to c++ native feature
    // QA: Explain this concept in detail
    // R: Correct intuition. **Performance differences:**
    //
    //    **p1 (new):**
    //    - 2 heap allocations (slower)
    //    - 2 separate deallocations when destroyed
    //    - Object and control block may be in different cache lines (worse cache performance)
    //
    //    **p2 (make_shared):**
    //    - 1 heap allocation (faster)
    //    - 1 deallocation when destroyed
    //    - Object and control block adjacent (better cache locality → fewer cache misses)
    //
    //    **Benchmark example (typical):**
    //    - new: ~100ns per construction (2 allocations)
    //    - make_shared: ~50ns per construction (1 allocation)
    //
    //    This isn't compiler optimization—it's a runtime library design difference.
    //    make_shared is implemented to allocate one larger block instead of two separate blocks.
    //
    //    **For junior SWEs:**
    //    make_shared is faster because it does one allocation instead of two. It also improves cache performance.
    //
    // Q: When would you prefer new over make_shared?
    // A: I don't know.  Provide answer
    // R: **Prefer new over make_shared when:**
    //
    //    1. **Custom deleters required:**
    //       shared_ptr<FILE> f(fopen("file.txt", "r"), fclose); // Can't use make_shared with custom deleter
    //
    //    2. **weak_ptr lifetime issues:**
    //       With make_shared, the object and control block are in the same allocation.
    //       If weak_ptr exists, the entire allocation (object + control block) can't be freed until weak_ptr expires,
    //       even though the object is already destroyed. This can waste memory.
    //       With new, the object memory is freed when use_count reaches 0, even if weak_ptr still exists.
    //
    //    3. **Very large objects with long-lived weak_ptr:**
    //       If you have a 10MB object and weak_ptr that outlives the object by hours, make_shared keeps 10MB allocated.
    //       With new, only the small control block remains allocated.
    //
    //    4. **Adopting existing raw pointers:**
    //       shared_ptr<T> p(existing_raw_ptr); // Can't use make_shared here
    //
    //    **Default: Use make_shared unless you have a specific reason not to.**
    //
    //    **For junior SWEs:**
    //    Use make_shared by default. Use new when you need custom deleters or are wrapping existing raw pointers.
    
    EXPECT_EQ(new_count, 1);
    EXPECT_EQ(makeshared_count, 1);
}

TEST_F(ReferenceCountingTest, MultipleAliases)
{
    long count_after_first = 0;
    long count_after_second = 0;
    long count_after_third = 0;
    
    // TODO: Create p1
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Tracked1");
    // Q: How many Tracked objects exist after this line?
    // A: 1
    // R: Correct.
    //
    // Q: How many control blocks exist?
    // A: 1
    // R: Correct. make_shared creates one control block for the one object.
    
    // TODO: Capture use_count after first creation
    count_after_first = p1.use_count(); // 1
    // Q: Why is use_count() == 1 when only p1 exists?
    // A: Only one reference to the Tracked object
    // R: Correct. One shared_ptr instance = use_count of 1.
    //
    //    **For junior SWEs:**
    //    use_count tracks how many shared_ptr instances own the object. Only p1 exists, so count is 1.
    
    // TODO: Create p2 as copy of p1
    std::shared_ptr<Tracked> p2 = p1;
    // Q: Does p2 create a new Tracked object or share p1's object?
    // A: share
    // R: Correct. Copy constructor shares ownership, does not copy the object.
    //
    // Q: Does p2 create a new control block or share p1's control block?
    // A: share
    // R: Correct. Both p1 and p2 point to the same control block.
    //
    // Q: What operation does the copy constructor perform on the control block?
    // A: Increments the count by 1
    // R: Correct. The control block's use_count is atomically incremented from 1 to 2.
    //
    //    **For junior SWEs:**
    //    Copying a shared_ptr increments the reference count in the shared control block.
    
    // TODO: Capture use_count after second copy
    count_after_second = p2.use_count(); // 2
    // Q: Why do both p1.use_count() and p2.use_count() return 2?
    // A: Because they both reference the same Tracked object
    // R: Correct. More precisely: they both query the same control block, which stores use_count == 2.
    //
    // Q: If you called p1.use_count() at this point, what would it return?
    // A: 2
    // R: Correct. All copies return the same value because they share the control block.
    
    // TODO: Create p3 as copy of p2
    std::shared_ptr<Tracked> p3 = p2;
    // Q: How many shared_ptr instances now reference the same Tracked object?
    // A: 3
    // R: Correct. p1, p2, and p3 all share ownership.
    //
    // Q: How many Tracked objects exist in total?
    // A: 1
    // R: Correct. Only one object, owned by three shared_ptr instances.
    
    // TODO: Capture use_count after third copy
    count_after_third = p3.use_count(); // 3
    // Q: What would p1.use_count() and p2.use_count() return at this point?
    // A: The same as p3, 3
    // R: Correct. All three query the same control block.
    //
    // Q: When will the Tracked object be destroyed?
    // A: When p1, p2 and p3 release their ownership of the Tracked object and the count hits 0
    // R: Correct. When all three go out of scope (or call reset()), use_count reaches 0 and the object is destroyed.
    //
    //    **For junior SWEs:**
    //    The object is destroyed when the last shared_ptr releases ownership.
    //
    // Q: If you wrote p2.reset() here, what would p1.use_count() and p3.use_count() become?
    // A: 2
    // R: Correct. p2.reset() decrements the control block's use_count from 3 to 2. p1 and p3 still share ownership.
    
    EXPECT_EQ(count_after_first, 1);
    EXPECT_EQ(count_after_second, 2);
    EXPECT_EQ(count_after_third, 3);
}

TEST_F(ReferenceCountingTest, SelfAssignment)
{
    long count_before = 0;
    long count_after = 0;
    
    // TODO: Create p1
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Tracked1");
    // Q: What is the initial use_count of p1?
    // A: 1
    // R: Correct.
    
    // TODO: Capture use_count before self-assignment
    count_before = p1.use_count();
    // Q: Why is count_before == 1?
    // A: Refer to previous explanations prior, this is a redundant question
    // R: Acknowledged. Only one shared_ptr instance exists.
    
    // TODO: Assign p1 to itself (p1 = p1)
    p1 = p1;
    // Q: What does the assignment operator do when source and destination are the same object?
    // A: I would guess that there is a self-assignment check in the copy constructor of shared_ptr and is skipped
    // A: If there isn't a self-assignment check, then it probably does a std::move
    // R: Partially correct. The assignment operator (not copy constructor) detects self-assignment and does nothing.
    //    It checks if the control block pointers are the same. If they are, it returns early without any operations.
    //    No move is involved—self-assignment is detected and skipped entirely.
    //
    //    **For junior SWEs:**
    //    The assignment operator checks if source and destination are the same and skips all operations if they are.
    //
    // Q: Does self-assignment increment the reference count?
    // A: No
    // R: Correct. No reference count changes occur during self-assignment.
    //
    // Q: What would happen if shared_ptr's assignment operator didn't check for self-assignment?
    /* A:
        - count goes to 0
        - memory gets released
        - copy from a release Tracked object which is now stale
        - Will result in a double-free segfault
    */
    // R: Correct reasoning. **Detailed sequence without self-assignment check:**
    //    1. Assignment operator decrements old control block: use_count 1 → 0
    //    2. use_count reaches 0 → Tracked object destroyed, control block destroyed
    //    3. Assignment operator tries to increment new control block (same as old, now destroyed)
    //    4. Accessing destroyed control block → undefined behavior (likely crash)
    //    5. p1 now holds a dangling pointer to destroyed control block
    //
    //    This is a use-after-free bug. The self-assignment check prevents this by detecting that source and destination
    //    share the same control block before any operations occur.
    //
    //    **For junior SWEs:**
    //    Without the check, self-assignment would destroy the object, then try to use the destroyed control block. This crashes.
    //
    // QR: Provide a line by line code process flow between numbers 1-3.  I'd like to understand how exactly the memory gets released between 2 and 3
    // RR: **Correction to previous explanation:**
    //     Modern shared_ptr implementations use **increment-first** ordering, which makes self-assignment safe without a check.
    //
    //     **Actual implementation (simplified):**
    //     ```cpp
    //     shared_ptr& operator=(const shared_ptr& rhs) {
    //         // Self-assignment check (optimization, not correctness requirement)
    //         if (control_block_ == rhs.control_block_) {
    //             return *this;
    //         }
    //         
    //         // Save old control block
    //         ControlBlock* old_cb = control_block_;
    //         
    //         // Step 1: Copy new pointers and increment FIRST
    //         control_block_ = rhs.control_block_;
    //         stored_ptr_ = rhs.stored_ptr_;
    //         if (control_block_ != nullptr) {
    //             control_block_->use_count++;  // Increment new
    //         }
    //         
    //         // Step 2: Decrement old LAST
    //         if (old_cb != nullptr) {
    //             old_cb->use_count--;
    //             if (old_cb->use_count == 0) {
    //                 old_cb->deleter(stored_ptr_);  // Destroy object
    //                 if (old_cb->weak_count == 0) {
    //                     delete old_cb;  // Free control block
    //                 }
    //             }
    //         }
    //         
    //         return *this;
    //     }
    //     ```
    //
    //     **Why increment-first prevents the bug:**
    //     Even in self-assignment (p1 = p1), incrementing first ensures use_count never reaches 0 prematurely.
    //     - Increment: 1 → 2
    //     - Decrement: 2 → 1
    //     - Object stays alive throughout
    //
    //     **Why the self-assignment check still exists:**
    //     It's an **optimization** to avoid unnecessary atomic operations (increment + decrement on the same control block).
    //     Without the check, self-assignment is **safe but slower**.
    //
    //     **For junior SWEs:**
    //     Modern shared_ptr increments the new count before decrementing the old count. This prevents premature destruction,
    //     even during self-assignment. The self-assignment check is just an optimization to skip unnecessary work. 
    
    // TODO: Capture use_count after self-assignment
    count_after = p1.use_count();
    // Q: Why does use_count remain 1 after self-assignment?
    // A: Because no code was really executed due to self-assignment skip
    // R: Correct. The self-assignment check returns early, so no increment or decrement occurs.
    //
    // Q: Did the Tracked object get destroyed during self-assignment?
    // A: No
    // R: Correct. The object remains alive and unchanged.
    //
    // Q: Is self-assignment a common pattern in real code?
    // A: Yes, I've fixed one recently myself
    // R: Correct observation. Self-assignment typically occurs in generic code patterns like:
    //    - container[i] = container[j]; (when i == j)
    //    - *ptr1 = *ptr2; (when ptr1 == ptr2)
    //    - obj = std::move(obj); (accidental self-move)
    //
    //    While direct self-assignment (p1 = p1) is rare, indirect self-assignment through references, pointers,
    //    or container indexing is common enough that the standard library must handle it correctly.
    //
    //    **For junior SWEs:**
    //    Self-assignment is rare in direct form but common in generic code. The standard library handles it safely.
    
    EXPECT_EQ(count_before, 1);
    EXPECT_EQ(count_after, 1);
}

TEST_F(ReferenceCountingTest, OwnershipTransferAcrossScopes)
{
    long inner_count = 0;
    long outer_count_before = 0;
    long outer_count_after = 0;
    
    // TODO: Create empty outer shared_ptr
    std::shared_ptr<Tracked> outer;
    // Q: What is the state of a default-constructed shared_ptr?
    // A: outer owns nothing
    // R: Correct. More precisely: control block pointer is nullptr, stored pointer is nullptr.
    //
    // Q: What does outer.get() return?
    // A: nullptr
    // R: Correct.
    //
    //    **For junior SWEs:**
    //    Default-constructed shared_ptr is empty. It owns nothing and get() returns nullptr.
    
    // TODO: Capture outer's use_count (should be 0)
    outer_count_before = outer.use_count();
    // Q: Why does an empty shared_ptr return use_count() == 0?
    // A: Because there is no object ownership of anything
    // R: Correct. More precisely: no control block exists, so use_count() returns 0 by specification.
    
    {
        // TODO: Create inner shared_ptr
        std::shared_ptr<Tracked> inner = std::make_shared<Tracked>("inner");
        // Q: Where is the Tracked object created (stack or heap)?
        // A: heap.  How else would the memory be accessible outside a scope?
        // R: Correct reasoning. make_shared allocates on the heap, which persists beyond the creating scope.
        //
        // Q: What is inner's scope?
        // A: Within these brackets?  I'm not sure I understand the question
        // R: Correct. inner's scope is lines 733-756 (the inner block). When execution reaches line 756, inner's destructor runs.
        //    The question asks: where does inner exist as a variable? Answer: only within the braces.
        //
        //    **For junior SWEs:**
        //    inner exists only within the braces. It's destroyed at the closing brace.
        
        // TODO: Capture inner's use_count
        inner_count = inner.use_count();
        // Q: Why is inner.use_count() == 1?
        // A: Because we actually allocated Tracked and control block onto the heap
        // R: Partially correct. More precisely: because only one shared_ptr instance (inner) references the control block.
        //    The allocation (heap vs stack) doesn't determine use_count—the number of shared_ptr instances does.
        
        // TODO: Assign inner to outer (transfer ownership)
        outer = std::move(inner);
        // Q: After this move, what is inner.use_count()?
        // A: 0
        // R: Correct. inner is now empty.
        //
        // Q: After this move, what is outer.use_count()?
        // A: 1
        // R: Correct. outer now holds the only reference.
        //
        // Q: Does std::move create a copy of the Tracked object?
        // A: No, it moves ownership to outer and then essentially calls a .reset() after the move operation.  Guessing here but provide additional detail
        // R: Correct concept, but mechanism needs precision:
        //    std::move(inner) casts inner to an rvalue reference, enabling the move assignment operator.
        //    The move assignment operator:
        //    1. Transfers inner's control block pointer to outer
        //    2. Transfers inner's stored pointer to outer
        //    3. Sets inner's pointers to nullptr (making it empty)
        //    No reference count changes occur (no increment/decrement) because ownership is transferred, not shared.
        //    No .reset() is called—the move assignment operator directly manipulates the internal pointers.
        //
        //    **For junior SWEs:**
        //    Move transfers the control block pointer from inner to outer. No copy, no reference count change. inner becomes empty.
        //
        // Q: What is the state of inner after the move?
        // A: Owns nothing, default-constructed shared_ptr with no initialization
        // R: Correct. inner is in the moved-from state, equivalent to default-constructed (nullptr control block, nullptr stored pointer).
    }
    // Q: At this closing brace, inner goes out of scope. What happens to the Tracked object?
    // A: It remains alive
    // R: Correct. The object remains alive because outer still holds a reference.
    //
    // Q: Why doesn't the Tracked object get destroyed when inner's destructor runs?
    // A: Because outer now has ownership or reference to it
    // R: Correct. More precisely: inner's destructor runs, but inner is empty (use_count == 0), so no decrement occurs.
    //    outer holds the only reference (use_count == 1), keeping the object alive.
    //
    //    **For junior SWEs:**
    //    inner is empty after the move, so its destructor does nothing. outer keeps the object alive.
    
    // TODO: Capture outer's use_count after inner goes out of scope
    outer_count_after = outer.use_count();
    // Q: Why is outer.use_count() still 1 after inner is destroyed?
    // A: Because it has ownership of it, redundant question
    // R: Acknowledged. outer is the sole owner.
    //
    // Q: When will the Tracked object be destroyed?
    // A: When outer goes out of scope or .reset is called
    // R: Correct. At line 774 (end of test), outer's destructor runs, use_count reaches 0, and the object is destroyed.
    //
    // Q: If you had used outer = inner (copy) instead of outer = std::move(inner), what would outer.use_count() be?
    // A: At that point, 2.  But, then back to 1 when inner went out of scope
    // R: Correct. With copy assignment:
    //    - After outer = inner: both inner and outer reference the same object, use_count == 2
    //    - After inner goes out of scope: inner's destructor decrements, use_count == 1
    //    - Final state: same as move (use_count == 1), but with unnecessary reference count operations
    //
    //    **For junior SWEs:**
    //    Copy would temporarily increase use_count to 2, then back to 1. Move keeps it at 1 throughout (more efficient).
    
    EXPECT_EQ(outer_count_before, 0);
    EXPECT_EQ(inner_count, 1);
    EXPECT_EQ(outer_count_after, 1);
}

// ============================================================================
// FINAL ASSESSMENT: ReferenceCountingTest Suite Completion
// ============================================================================
//
// ## Learning Progression: Before vs. After
//
// ### BEFORE This Test Suite
//
// **Conceptual Understanding:**
// - Understood traditional C++ ownership (single owner, explicit destruction)
// - Minimal shared_ptr experience (used once at previous company)
// - Strong RAII intuition and lifetime reasoning
// - Unfamiliar with distributed ownership model
//
// **Knowledge Gaps:**
// - Control block abstraction (invisible runtime state)
// - Aliasing constructor mechanics (subobject lifetime extension)
// - Allocation patterns (new vs make_shared performance implications)
// - Move vs copy semantics for shared_ptr
// - Self-assignment implementation details
// - Exception safety considerations
//
// **Initial Struggle:**
// - "No classes being directly responsible for resources" - fundamental paradigm shift
// - Control block as separate from pointed-to object
// - Predicted alias_count == 1 instead of 2 (missed control block sharing)
//
// ### AFTER This Test Suite
//
// **Mastered Concepts:**
//
// 1. **Control Block Mechanics**
//    - Understood as separate heap allocation containing use_count, weak_count, deleter
//    - Recognized that use_count() queries the control block, not the object
//    - Grasped that copying shared_ptr shares the control block (no new allocation)
//
// 2. **Allocation Patterns**
//    - new: 2 allocations (object + control block, non-adjacent)
//    - make_shared: 1 allocation (both together, better cache locality)
//    - Exception safety: make_shared prevents leak between allocation and shared_ptr construction
//    - Performance: make_shared ~2x faster due to single allocation
//
// 3. **Move vs. Copy Semantics**
//    - Copy: Shares ownership, increments use_count, zero heap allocations
//    - Move: Transfers ownership, no use_count change, source becomes empty
//    - Move is more efficient for ownership transfer (no atomic operations)
//
// 4. **Aliasing Constructor**
//    - Shares control block with owner, but points to different object (subobject)
//    - Extends lifetime of owner through pointer to member
//    - Critical for safe subobject access beyond owner's scope
//
// 5. **Self-Assignment Safety**
//    - Modern implementations use increment-first ordering (safe without check)
//    - Self-assignment check is optimization (avoids unnecessary atomic ops), not correctness requirement
//    - Corrected initial assumption about decrement-first causing use-after-free
//
// 6. **Ownership Transfer Across Scopes**
//    - Move semantics enable lifetime extension beyond creating scope
//    - Empty shared_ptr (default-constructed) has use_count == 0
//    - Moved-from shared_ptr is equivalent to default-constructed state
//
// **Terminology Precision Improvements:**
// - "Resource" → "Object" (more precise for lifetime discussions)
// - "Memory address space" → "Control block" (correct abstraction)
// - "Allocation" vs "Reference count" (distinct concepts)
// - Assignment operator vs copy constructor (correct operation identification)
//
// **Key Insights Gained:**
//
// 1. **Control block is the invisible complexity** that makes shared_ptr hard to reason about
//    - Can't read one class and know when deletion happens
//    - Lifetime determined by global reference counting state
//
// 2. **Copying shared_ptr is cheap** (pointer copy + atomic increment, no heap allocation)
//    - Challenged initial assumption that copying might be expensive
//
// 3. **make_shared is preferred** unless custom deleters or weak_ptr lifetime issues apply
//    - Performance, exception safety, and cache locality advantages
//
// 4. **Aliasing constructor enables safe subobject pointers**
//    - Prevents dangling pointers to members of destroyed objects
//    - Critical pattern for teaching team about lifetime guarantees
//
// 5. **Self-assignment is safe but inefficient** without the check
//    - Corrected understanding of increment-first ordering
//    - Recognized optimization vs correctness distinction
//
// ## Pedagogical Readiness
//
// **Strengths for Teaching:**
// - Deep mechanical understanding of control block abstraction
// - Can explain allocation patterns with performance implications
// - Understands move vs copy at both conceptual and implementation level
// - Recognizes anti-patterns (p1.reset(p2.get()) → double-deletion)
// - Can articulate "when NOT to use shared_ptr" (unclear ownership, should be unique_ptr)
//
// **Teaching Capabilities:**
// - Can explain to senior engineers: Technical precision with implementation details
// - Can explain to junior engineers: Simplified technical explanations without analogies
// - Can demonstrate with instrumentation: Observable signals confirm mental model
// - Can diagnose misuse: "Just add shared_ptr" symptom-driven reasoning vs invariant-based
//
// **Next Learning Areas:**
// - shared_from_this() mechanics (async callback lifetime extension)
// - weak_ptr and circular reference detection
// - Custom deleters for C resource interop
// - Anti-patterns in legacy codebases (global shared_ptr, pass-by-value hot paths)
//
// ## Assessment: Strong Foundation Established
//
// You've moved from "minimal shared_ptr experience" to "deep mechanical understanding"
// of reference counting, control blocks, and ownership semantics. Your ability to:
// - Catch implementation assumptions (increment-first ordering)
// - Ask precise questions ("Why no control_block_ = nullptr?")
// - Reason about performance (allocation count, cache locality)
// - Identify anti-patterns (double control blocks from raw pointers)
//
// ...demonstrates you're ready to teach these concepts and guide your team toward
// invariant-based reasoning about shared_ptr usage.
//
// The struggle with "no classes directly responsible for resources" was the RIGHT
// struggle—it reveals the fundamental tension between traditional C++ ownership
// and shared_ptr's distributed responsibility model. You've now internalized both
// paradigms and can articulate when each is appropriate.
//
// **Recommendation:** Proceed to ownership patterns (shared_from_this) and weak_ptr
// to complete the foundation for teaching async callback lifetime management.
//
// ============================================================================
