#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <functional>
#include <vector>

class ScopeLifetimePatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// TEST_F(ScopeLifetimePatternsTest, WeakPtrLockBasic)
// {
//     // TODO: Create weak_ptr
//     // YOUR CODE HERE
    
//     bool expired_before = false;
//     // TODO: Check if expired
//     // expired_before = ???
    
//     {
//         // TODO: Create shared_ptr and assign to weak
//         // YOUR CODE HERE
        
//         bool expired_during = false;
//         // TODO: Check if expired during
//         // expired_during = ???
        
//         // TODO: Lock weak_ptr
//         // YOUR CODE HERE
        
//         long use_count = 0;
//         // TODO: Get use_count of locked ptr
//         // use_count = ???
        
//         EXPECT_FALSE(expired_before);
//         EXPECT_FALSE(expired_during);
//         EXPECT_EQ(use_count, 2);
//     }
    
//     bool expired_after = false;
//     // TODO: Check if expired after
//     // expired_after = ???
    
//     // TODO: Try to lock after shared is destroyed
//     // YOUR CODE HERE
    
//     bool locked_after_null = false;
//     // TODO: Check if locked_after is nullptr
//     // locked_after_null = ???
    
//     EXPECT_TRUE(expired_after);
//     EXPECT_TRUE(locked_after_null);
// }

// TEST_F(ScopeLifetimePatternsTest, WeakPtrLockMultipleTimes)
// {
//     // TODO: Create shared_ptr and weak_ptr
//     // YOUR CODE HERE
    
//     // TODO: Lock weak_ptr three times
//     // YOUR CODE HERE
    
//     long use_count = 0;
//     // TODO: Get use_count of shared
//     // use_count = ???
    
//     EXPECT_EQ(use_count, 4);
// }

// TEST_F(ScopeLifetimePatternsTest, LambdaCaptureSharedPtr)
// {
//     long captured_use_count = 0;
    
//     // TODO: Declare callback function
//     // YOUR CODE HERE
    
//     {
//         // TODO: Create shared_ptr
//         // YOUR CODE HERE
        
//         // TODO: Create lambda that captures shared by value
//         // YOUR CODE HERE
        
//         long use_count_before_call = 0;
//         // TODO: Get use_count before calling callback
//         // use_count_before_call = ???
        
//         EXPECT_EQ(use_count_before_call, 2);
//     }
    
//     // TODO: Call callback after shared goes out of scope
//     // YOUR CODE HERE
    
//     EXPECT_EQ(captured_use_count, 1);
// }

// TEST_F(ScopeLifetimePatternsTest, LambdaCaptureWeakPtr)
// {
//     bool was_valid = false;
    
//     // TODO: Declare callback function
//     // YOUR CODE HERE
    
//     {
//         // TODO: Create shared_ptr and weak_ptr
//         // YOUR CODE HERE
        
//         // TODO: Create lambda that captures weak by value
//         // YOUR CODE HERE
        
//         long use_count = 0;
//         // TODO: Get use_count of shared
//         // use_count = ???
        
//         EXPECT_EQ(use_count, 1);
//     }
    
//     // TODO: Call callback after shared is destroyed
//     // YOUR CODE HERE
    
//     EXPECT_FALSE(was_valid);
// }

// TEST_F(ScopeLifetimePatternsTest, LambdaCaptureWeakPtrStillAlive)
// {
//     bool was_valid = false;
//     long captured_use_count = 0;
    
//     // TODO: Create shared_ptr and weak_ptr
//     // YOUR CODE HERE
    
//     // TODO: Create lambda that captures weak
//     // YOUR CODE HERE
    
//     // TODO: Call callback while shared is still alive
//     // YOUR CODE HERE
    
//     EXPECT_TRUE(was_valid);
//     EXPECT_EQ(captured_use_count, 2);
// }

// class CallbackManager
// {
// public:
//     void register_callback(std::function<void()> callback)
//     {
//         callbacks_.push_back(callback);
//     }
    
//     void invoke_all()
//     {
//         for (auto& callback : callbacks_)
//         {
//             callback();
//         }
//     }
    
// private:
//     std::vector<std::function<void()>> callbacks_;
// };

// TEST_F(ScopeLifetimePatternsTest, CallbackWithSharedPtrLifetimeExtension)
// {
//     CallbackManager manager;
    
//     {
//         // TODO: Create shared_ptr
//         // YOUR CODE HERE
        
//         // TODO: Register callback that captures shared
//         // YOUR CODE HERE
//     }
    
//     auto events = EventLog::instance().events();
//     size_t dtor_count = 0;
    
//     for (const auto& event : events)
//     {
//         if (event.find("::dtor") != std::string::npos)
//         {
//             ++dtor_count;
//         }
//     }
    
//     EXPECT_EQ(dtor_count, 0);
// }

// TEST_F(ScopeLifetimePatternsTest, CallbackWithWeakPtrNoLifetimeExtension)
// {
//     CallbackManager manager;
    
//     {
//         // TODO: Create shared_ptr and weak_ptr
//         // YOUR CODE HERE
        
//         // TODO: Register callback that captures weak
//         // YOUR CODE HERE
//     }
    
//     auto events = EventLog::instance().events();
//     size_t dtor_count = 0;
    
//     for (const auto& event : events)
//     {
//         if (event.find("::dtor") != std::string::npos)
//         {
//             ++dtor_count;
//         }
//     }
    
//     EXPECT_EQ(dtor_count, 1);
// }

// TEST_F(ScopeLifetimePatternsTest, NestedLambdaCapture)
// {
//     long outer_use_count = 0;
//     long inner_use_count = 0;
    
//     // TODO: Create shared_ptr
//     // YOUR CODE HERE
    
//     // TODO: Create outer lambda that captures shared
//     // TODO: Inside outer lambda, create inner lambda that also captures shared
//     // YOUR CODE HERE
    
//     // TODO: Call outer lambda
//     // YOUR CODE HERE
    
//     EXPECT_EQ(outer_use_count, 2);
//     EXPECT_EQ(inner_use_count, 3);
// }

// TEST_F(ScopeLifetimePatternsTest, WeakPtrExpiredCheck)
// {
//     // TODO: Create weak_ptr
//     // YOUR CODE HERE
    
//     bool expired_initial = false;
//     // TODO: Check if expired initially
//     // expired_initial = ???
    
//     {
//         // TODO: Create shared_ptr and assign to weak
//         // YOUR CODE HERE
        
//         bool expired_alive = false;
//         // TODO: Check if expired while alive
//         // expired_alive = ???
        
//         EXPECT_TRUE(expired_initial);
//         EXPECT_FALSE(expired_alive);
//     }
    
//     bool expired_final = false;
//     // TODO: Check if expired after shared is destroyed
//     // expired_final = ???
    
//     EXPECT_TRUE(expired_final);
// }

// TEST_F(ScopeLifetimePatternsTest, ConditionalLockInCallback)
// {
//     int invocation_count = 0;
    
//     // TODO: Declare callback function
//     // YOUR CODE HERE
    
//     {
//         // TODO: Create shared_ptr and weak_ptr
//         // YOUR CODE HERE
        
//         // TODO: Create callback with conditional lock
//         // YOUR CODE HERE
        
//         // TODO: Call callback while shared is alive
//         // YOUR CODE HERE
//     }
    
//     // TODO: Call callback after shared is destroyed
//     // YOUR CODE HERE
    
//     EXPECT_EQ(invocation_count, 1);
// }

// TEST_F(ScopeLifetimePatternsTest, MutableLambdaWithSharedPtr)
// {
//     // TODO: Create shared_ptr
//     // YOUR CODE HERE
    
//     long initial_count = 0;
//     // TODO: Get initial use_count
//     // initial_count = ???
    
//     // TODO: Create mutable lambda that captures shared and resets it
//     // YOUR CODE HERE
    
//     long before_call = 0;
//     // TODO: Get use_count before calling callback
//     // before_call = ???
    
//     // TODO: Call callback
//     // YOUR CODE HERE
    
//     long after_call = 0;
//     // TODO: Get use_count after calling callback
//     // after_call = ???
    
//     EXPECT_EQ(initial_count, 1);
//     EXPECT_EQ(before_call, 2);
//     EXPECT_EQ(after_call, 1);
// }

// ============================================================================
// CONTROL BLOCK CORRUPTION SCENARIOS
// ============================================================================
//
// These tests demonstrate how control blocks can become corrupted through
// incorrect usage patterns. Understanding these failure modes is critical
// for debugging production issues.
//
// ============================================================================

TEST_F(ScopeLifetimePatternsTest, ControlBlockCorruption_DoubleControlBlock)
{
    // DANGER: Creating two shared_ptrs from the same raw pointer creates
    // two separate control blocks, both thinking they own the object.
    // When either reaches use_count=0, it deletes the object, leaving the
    // other with a dangling pointer and corrupted control block state.
    
    // Tracked* raw = new Tracked("Dangerous");
    
    // Q: What happens when you create two shared_ptrs from the same raw pointer?
    // A: They both end up thinking they own the object and have a use_count of 1 when it should be 2 for the same/shared control block
    // R: Correct. Each shared_ptr creates its own independent control block, both believing they have exclusive ownership of the object. The use_count in each control block is 1, but they're separate control blocks—not a shared one with count 2.
    // R: For junior SWEs: Think of it like two people each keeping their own count of how many people are using a car, instead of sharing one count. Each thinks "I'm the only one tracking this car" when really both are.
    
    // std::shared_ptr<Tracked> p1(raw);  // Creates control block #1
    
    // Q: At this point, what is p1.use_count()?
    // A: 1
    // R: Correct. p1's control block shows use_count=1 because it's the only shared_ptr referencing that control block.
    // R: For junior SWEs: The control block was just created, and only p1 points to it, so the count is 1.
    
    // DANGER: This creates a SECOND control block for the same object
    // std::shared_ptr<Tracked> p2(raw);  // Creates control block #2
    
    // Q: What is p1.use_count() now? What about p2.use_count()?
    // A: Both are 1
    // R: Correct. p1.use_count() remains 1 (nothing changed its control block), and p2.use_count() is also 1 (it created a new, separate control block). They're tracking independently.
    // R: For junior SWEs: Creating p2 didn't update p1's control block because they don't know about each other—they're completely separate tracking systems for the same object.
    
    // Q: When p1 goes out of scope, what happens to the Tracked object?
    // A: The Tracked object is freed
    // R: Correct. When p1's control block reaches use_count=0, it calls delete on the raw pointer, freeing the object.
    // R: For junior SWEs: p1's control block thinks it owns the object, so when p1 goes away, the control block says "time to clean up" and deletes the object.
    
    // Q: When p2 tries to access the object after p1 destroyed it, what happens?
    // A: A segmentation fault, use-after-free
    // R: Correct. p2's control block still thinks the object is valid (its use_count is still 1), but the memory was already freed by p1. Accessing freed memory is undefined behavior—typically a segfault or memory corruption.
    // R: For junior SWEs: p2 doesn't know p1 deleted the object. It's like having a key to an apartment that was demolished—the key still exists, but using it leads to disaster.
    
    // Q: What is the correct way to create p2 from p1?
    // A: p2 = p1
    // R: Correct. Copy-assigning from p1 makes p2 share the same control block, incrementing use_count to 2. Both pointers coordinate through one control block.
    // R: For junior SWEs: This way, both p1 and p2 use the same tracking system (control block), so they know about each other and coordinate the object's lifetime properly.
    
    // Q: At line 381, both p1 and p2 exist and point to the same object. Each has its own control block. When you call p1.use_count(), what memory does it need to access?
    // A: It's own control block.  use_count does not directly access the object directly
    // R: Correct. use_count() only reads from the control block's reference count field, not the managed object itself.
    
    // Q: Given that p2's control block also points to the same Tracked object, and both control blocks might be trying to manage the same memory, what kind of race condition or memory corruption could occur?
    // A: Unless use_count does try to access the object directly with an underlying mutex lock, this is most likely a deadlock situation but explain the step by step process as to how this can happen
    // R: Actually, after examining the instrumentation code, there are no mutexes—EventLog is just a simple vector. The hang is more subtle. When you create p2 from the raw pointer at line 350, the shared_ptr constructor doesn't call the Tracked constructor again (the object already exists). However, the undefined behavior creates memory corruption. The hang at line 384 when calling use_count() suggests the control block itself may be corrupted—possibly from heap metadata corruption when the same memory region has two conflicting control blocks pointing to it. This isn't a deadlock; it's likely a corrupted pointer or infinite loop in the allocator's internal structures.
    
    // Q: Is this test actually safe to run, or does it invoke undefined behavior before it even reaches the assertions?
    // A: Based on running the test, it hangs completely and is not safe to run
    // R: Correct. This test invokes undefined behavior immediately at line 350 when creating p2 from the same raw pointer. The hang you're seeing is one manifestation of that undefined behavior. This test should be commented out or restructured to demonstrate the concept without actually executing the dangerous pattern.
    
    // FOLLOW-UP: Validating the hang
    // Q: What information would you get from running the test under a debugger (gdb) and interrupting it during the hang? What specific commands would show you the call stack and what each thread is doing?
    // A: - (gdb) set logging file threads.txt
    // A: - (gdb) set logging on
    // A: - (gdb) thread apply all bt
    // A: - (gdb) set logging off
    // R: Correct commands. `thread apply all bt` shows backtraces for all threads, revealing where each is stuck. You'd see the exact function call in the standard library where the hang occurs (likely in control block operations or allocator internals).
    
    // Q: What would AddressSanitizer (ASan) report if you compiled and ran this test with -fsanitize=address? Would it catch the double control block issue before the hang occurs?
    // A: If anything, I would think it gets caught in the "std::shared_ptr<Tracked> p2(raw);" line.  If this is a known malpractice of shared_ptr's I would think the address sanitizers would take that into consideration
    // R: ASan doesn't detect double control blocks at creation time—creating p2 from raw doesn't trigger a detectable memory error yet. ASan would catch the double-free when the first shared_ptr destructor runs and deletes the object, then the second tries to delete it again. The hang prevents you from reaching that point.
    
    // Q: If you added print statements (or EventLog records) immediately before and after line 350 (std::shared_ptr<Tracked> p2(raw)), and before line 393 (p1.use_count()), which print statements would you expect to see before the hang?
    // A: Not sure.  Provide the answer for me.  I care more for shared_ptr behavior over log outputs
    // R: You'd see all three prints: before p2 creation, after p2 creation, and before p1.use_count(). The hang happens during p1.use_count() execution, not before. This tells you that creating p2 succeeds (undefined behavior doesn't always fail immediately), but accessing the corrupted state later triggers the hang.
    
    // Q: What would happen if you ran this test with Valgrind? What specific errors would it report about the memory operations?
    // A: I am not accustomed to using shared_ptr's and what valgrind may report from them.  I would guess that it would complain about two control blocks for the same Tracked object
    // R: Valgrind can't detect "two control blocks for one object" conceptually—that's a logic error, not a memory access error. However, when the first shared_ptr destructor runs and calls delete, then the second tries to delete the same address, Valgrind would report "Invalid free() / delete / delete[]" showing the double-free. Like ASan, it wouldn't catch the issue until destruction, and the hang prevents reaching that point.
    
    // Q: Could you isolate the problem by creating a minimal test that just does: Tracked* raw = new Tracked("X"); shared_ptr<Tracked> p1(raw); shared_ptr<Tracked> p2(raw); long c = p1.use_count(); without any test framework overhead?
    // A: Yes, and easily run with valgrind or sanitizers
    // R: Correct. Minimal reproduction is the best first step for debugging. It eliminates test framework variables and makes the problem easier to analyze with tools. You could also try without Tracked entirely—just `int* raw = new int(42);` to see if the hang is specific to Tracked or a general double control block issue.
    
    // long p1_count = p1.use_count(); // this line of code appears to hang.  Why?
    // long p2_count = p2.use_count();
    
    // EXPECT_EQ(p1_count, 1);  // Each control block thinks it's the only owner
    // EXPECT_EQ(p2_count, 1);  // This is the bug—should be 2 if sharing
}

TEST_F(ScopeLifetimePatternsTest, ControlBlockCorruption_StackObject)
{
    // DANGER: Creating a shared_ptr to a stack object means the control block
    // will try to delete the object when use_count reaches 0, but stack objects
    // are automatically destroyed when they go out of scope. This causes:
    // 1. Double destruction (stack + shared_ptr deleter)
    // 2. Deleting non-heap memory (undefined behavior)
    
    // Q: What happens if you create a shared_ptr to a stack object?
    // A: It will result in a dangling pointer when the object on the stack goes out of scope
    // R: Partially correct. It does create a dangling situation, but more precisely: the shared_ptr will attempt to call `delete` on stack memory when its destructor runs, which is undefined behavior (typically a crash). The "dangling pointer" aspect is secondary to the invalid delete operation.
    
    // Q: When does the stack object get destroyed?
    // A: When it goes out of scope
    // R: Correct. Stack objects are automatically destroyed when execution leaves their scope.
    
    // Q: When does the shared_ptr try to destroy the object?
    // A: When it goes out of scope
    // R: Correct. When `sp` goes out of scope and use_count reaches 0, the control block's deleter is invoked, which calls `delete` on the pointer.
    
    // Q: What is the result of trying to delete a stack object?
    // A: delete &stack_obj, double free segfault
    // R: Close, but not quite. It's not a "double free"—it's an "invalid free" (deleting memory that wasn't allocated with `new`). The crash happens because you're calling `delete` on stack memory, which is undefined behavior. The stack destructor running afterward is a separate issue (double destruction), but the first `delete` itself is already invalid.
    
    // DANGER: This is undefined behavior
    // Tracked stack_obj("StackObject");
    // std::shared_ptr<Tracked> sp(&stack_obj);  // WRONG: Will try to delete stack memory
    
    // When p goes out of scope, it will call delete on stack memory → crash
    // When stack_obj goes out of scope, destructor runs again → double destruction
    
    // CORRECT: Only create shared_ptr from heap-allocated objects
    std::shared_ptr<Tracked> p = std::make_shared<Tracked>("HeapObject");
    
    EXPECT_EQ(p.use_count(), 1);
}

TEST_F(ScopeLifetimePatternsTest, ControlBlockCorruption_SharedFromThisBeforeShared)
{
    // DANGER: Calling shared_from_this() before the object is owned by a
    // shared_ptr results in:
    // 1. Bad_weak_ptr exception (C++17+)
    // 2. Undefined behavior (C++11/14)
    // 3. Corrupted control block state
    
    class BadWidget : public std::enable_shared_from_this<BadWidget>
    {
    public:
        std::shared_ptr<BadWidget> get_self()
        {
            // Q: What happens if you call shared_from_this() before the object is owned by a shared_ptr?
            // A: In newer versions, it would throw a runtime_error shared ptr cast
            // A: In older versions, it will result in undefined behavior and eventual crash
            // R: Correct. C++17+ throws std::bad_weak_ptr exception. C++11/14 has undefined behavior because the internal weak_ptr is uninitialized, so calling lock() on it produces unpredictable results.
            
            // Q: What is stored in the internal weak_ptr before a shared_ptr owns the object?
            // A: 0?
            // R: Close. The internal weak_ptr is default-constructed (empty), which means it doesn't point to any control block—it's essentially null/empty. When you call shared_from_this(), it tries to lock() this empty weak_ptr, which either throws bad_weak_ptr (C++17+) or returns an empty shared_ptr that causes undefined behavior when used (C++11/14).
            
            return shared_from_this();  // DANGER if called before shared_ptr owns this
        }
    };
    
    // DANGER: Creating object on stack or with new, then calling shared_from_this()
    // BadWidget* raw = new BadWidget();
    // auto self = raw->get_self();  // THROWS bad_weak_ptr or undefined behavior
    
    // CORRECT: Create with shared_ptr first, then call shared_from_this()
    auto widget = std::make_shared<BadWidget>();
    auto self = widget->get_self();  // Safe: widget already owns the object
    
    EXPECT_EQ(widget.use_count(), 2);  // widget + self
}

TEST_F(ScopeLifetimePatternsTest, ControlBlockCorruption_DeletedObjectAccess)
{
    // DANGER: Holding a raw pointer after the shared_ptr is destroyed
    // leaves you with a dangling pointer. Accessing it corrupts the control
    // block's view of reality (it thinks object is deleted, but you're using it).
    
    Tracked* raw_ptr = nullptr;
    
    {
        std::shared_ptr<Tracked> p = std::make_shared<Tracked>("Temporary");
        raw_ptr = p.get();  // Get raw pointer
        
        // Q: What is the lifetime of the pointer returned by get()?
        // A: Whenever p goes out of scope or is manually released wit p.reset()
        // R: Correct. The pointer returned by get() is valid only as long as at least one shared_ptr owns the object. Once all shared_ptrs are destroyed or reset, the object is deleted and the pointer becomes dangling.
        
        // Q: What happens to raw_ptr when p goes out of scope?
        // A: The object at the memory address location is freed but is not set to nullptr and will be treated as non-null in conditional if statements
        // R: Correct. raw_ptr still holds the address value (not set to nullptr), but the memory at that address has been freed. This is a classic dangling pointer—the pointer itself is unchanged, but what it points to is gone.
        
        // p goes out of scope here, object is deleted
    }
    
    // Q: What happens if you dereference raw_ptr now?
    // A: Segfault
    // R: Possibly, but not guaranteed. It's undefined behavior, which means: (1) might segfault, (2) might return garbage data, (3) might appear to work if memory hasn't been reused yet. The unpredictability makes it dangerous—you can't rely on it crashing.
    
    // Q: What happens if you try to create a new shared_ptr from raw_ptr?
    // A: No segfault but will be default-constructed
    // R: Incorrect. Creating `shared_ptr<Tracked> p2(raw_ptr)` does NOT create a default-constructed shared_ptr. It creates a NEW control block for the already-deleted object (double control block problem). This is undefined behavior—you're creating a control block for freed memory. When p2 destructor runs, it will try to delete already-freed memory (double-free).
    
    // DANGER: raw_ptr is now dangling
    // Accessing it is undefined behavior:
    // - Might crash
    // - Might return garbage
    // - Might appear to work (most dangerous—silent corruption)
    
    // std::string name = raw_ptr->name();  // UNDEFINED BEHAVIOR
    
    // Creating new shared_ptr from dangling pointer:
    // std::shared_ptr<Tracked> p2(raw_ptr);  // DOUBLE CONTROL BLOCK + dangling pointer
    
    EXPECT_TRUE(raw_ptr != nullptr);  // Pointer value unchanged
    // But the object it points to is DELETED
}

TEST_F(ScopeLifetimePatternsTest, ControlBlockCorruption_WeakPtrUseAfterControlBlockDeleted)
{
    // DANGER: The control block is deleted when both use_count and weak_count
    // reach 0. If you somehow access a weak_ptr after its control block is
    // deleted (e.g., through memory corruption or use-after-free), you get
    // undefined behavior.
    
    std::weak_ptr<Tracked>* weak_ptr_ptr = nullptr;
    
    {
        auto p = std::make_shared<Tracked>("Temp");
        std::weak_ptr<Tracked> weak = p;
        
        // Q: When is the control block deleted?
        // A: When weak goes out of scope and p goes out of scope
        // R: Correct. The control block is deleted when BOTH use_count=0 (p destroyed) AND weak_count=0 (weak destroyed). In this test, both happen at line 560 when the scope ends.
        
        // Q: What keeps the control block alive after p is destroyed?
        // A: weak variable
        // R: Correct. Even after p is destroyed and use_count reaches 0 (object is deleted), the control block itself remains alive as long as weak_count > 0. The weak_ptr keeps the control block alive so it can track whether the object still exists via expired().
        
        // Store pointer to weak_ptr (DANGER: for demonstration only)
        weak_ptr_ptr = &weak;
        
        // weak goes out of scope here
        // Control block's weak_count drops to 0
        // Control block is DELETED
    }
    
    // Q: What happens if you try to access weak_ptr_ptr now?
    // A: Undefined behavior, segfault
    // R: Correct. weak_ptr_ptr points to stack memory that's been destroyed (the weak variable is gone), AND the control block it referenced is also deleted. Accessing either is undefined behavior—double use-after-free (stack object + heap control block both gone).
    
    // DANGER: weak_ptr_ptr points to deleted stack memory
    // AND the control block it referenced is also deleted
    
    // Accessing it is undefined behavior:
    // bool expired = weak_ptr_ptr->expired();  // CRASH or garbage
    
    // This scenario is rare but can happen with:
    // - Memory corruption
    // - Use-after-free bugs
    // - Incorrect lifetime management in complex systems
    
    EXPECT_TRUE(weak_ptr_ptr != nullptr);  // Pointer unchanged, but object deleted
}

// ============================================================================
// SUMMARY: How Control Blocks Get Corrupted
// ============================================================================
//
// 1. **Double Control Block**: Two shared_ptrs from same raw pointer
//    - Each thinks it's the sole owner
//    - First to reach use_count=0 deletes object
//    - Second has dangling pointer and corrupted state
//    - Fix: Always copy shared_ptr, never create from raw pointer twice
//
// 2. **Stack Object**: shared_ptr to stack-allocated object
//    - Control block tries to delete non-heap memory
//    - Stack destructor also runs
//    - Double destruction + invalid delete
//    - Fix: Only create shared_ptr from heap objects
//
// 3. **shared_from_this() Too Early**: Before shared_ptr owns object
//    - Internal weak_ptr is uninitialized
//    - Throws bad_weak_ptr or undefined behavior
//    - Fix: Ensure object is owned by shared_ptr before calling
//
// 4. **Dangling Raw Pointer**: Using get() after shared_ptr destroyed
//    - Raw pointer outlives the object
//    - Accessing deleted memory
//    - Creating new shared_ptr from it makes double control block
//    - Fix: Never store raw pointers beyond shared_ptr lifetime
//
// 5. **Control Block Use-After-Free**: Accessing weak_ptr after control block deleted
//    - Control block deleted when use_count=0 AND weak_count=0
//    - Accessing weak_ptr after this is undefined behavior
//    - Fix: Ensure weak_ptr lifetime doesn't exceed control block
//
// ============================================================================
// DEBUGGING CORRUPTED CONTROL BLOCKS
// ============================================================================
//
// Symptoms:
// - Crashes in shared_ptr destructor
// - Double-free detected by allocator
// - Heap corruption errors
// - use_count() returns unexpected values
// - Segfaults when accessing object through shared_ptr
//
// Tools:
// - AddressSanitizer (detects use-after-free, double-free)
// - Valgrind (detects memory errors)
// - GDB: `p *ptr._M_refcount._M_pi` (inspect control block)
// - Enable debug allocator (detects heap corruption)
//
// Prevention:
// - Never create shared_ptr from raw pointer twice
// - Never create shared_ptr to stack objects
// - Always use make_shared when possible
// - Use enable_shared_from_this for self-references
// - Never store raw pointers from get() long-term
// - Use weak_ptr for non-owning references
//
// ============================================================================
