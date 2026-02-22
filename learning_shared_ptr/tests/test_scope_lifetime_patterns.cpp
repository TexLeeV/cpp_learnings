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
    // A:
    // R:
    
    // std::shared_ptr<Tracked> p1(raw);  // Creates control block #1
    
    // Q: At this point, what is p1.use_count()?
    // A:
    // R:
    
    // DANGER: This creates a SECOND control block for the same object
    // std::shared_ptr<Tracked> p2(raw);  // Creates control block #2
    
    // Q: What is p1.use_count() now? What about p2.use_count()?
    // A:
    // R:
    
    // Q: When p1 goes out of scope, what happens to the Tracked object?
    // A:
    // R:
    
    // Q: When p2 tries to access the object after p1 destroyed it, what happens?
    // A:
    // R:
    
    // Q: What is the correct way to create p2 from p1?
    // A:
    // R:
    
    // Q: At line 381, both p1 and p2 exist and point to the same object. Each has its own control block. When you call p1.use_count(), what memory does it need to access?
    // A:
    // R:
    
    // Q: Given that p2's control block also points to the same Tracked object, and both control blocks might be trying to manage the same memory, what kind of race condition or memory corruption could occur?
    // A:
    // R:
    
    // Q: Is this test actually safe to run, or does it invoke undefined behavior before it even reaches the assertions?
    // A:
    // R:
    
    // FOLLOW-UP: Validating the hang
    // Q: What information would you get from running the test under a debugger (gdb) and interrupting it during the hang? What specific commands would show you the call stack and what each thread is doing?
    // A:
    // A:
    // A:
    // A:
    // R:
    
    // Q: What would AddressSanitizer (ASan) report if you compiled and ran this test with -fsanitize=address? Would it catch the double control block issue before the hang occurs?
    // A:
    // R:
    
    // Q: If you added print statements (or EventLog records) immediately before and after line 350 (std::shared_ptr<Tracked> p2(raw)), and before line 393 (p1.use_count()), which print statements would you expect to see before the hang?
    // A:
    // R:
    
    // Q: What would happen if you ran this test with Valgrind? What specific errors would it report about the memory operations?
    // A:
    // R:
    
    // Q: Could you isolate the problem by creating a minimal test that just does: Tracked* raw = new Tracked("X"); shared_ptr<Tracked> p1(raw); shared_ptr<Tracked> p2(raw); long c = p1.use_count(); without any test framework overhead?
    // A:
    // R:
    
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
    // A:
    // R:
    
    // Q: When does the stack object get destroyed?
    // A:
    // R:
    
    // Q: When does the shared_ptr try to destroy the object?
    // A:
    // R:
    
    // Q: What is the result of trying to delete a stack object?
    // A:
    // R:
    
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
            // A:
            // A:
            // R:
            
            // Q: What is stored in the internal weak_ptr before a shared_ptr owns the object?
            // A:
            // R:
            
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
        // A:
        // R:
        
        // Q: What happens to raw_ptr when p goes out of scope?
        // A:
        // R:
        
        // p goes out of scope here, object is deleted
    }
    
    // Q: What happens if you dereference raw_ptr now?
    // A:
    // R:
    
    // Q: What happens if you try to create a new shared_ptr from raw_ptr?
    // A:
    // R:
    
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
        // A:
        // R:
        
        // Q: What keeps the control block alive after p is destroyed?
        // A:
        // R:
        
        // Store pointer to weak_ptr (DANGER: for demonstration only)
        weak_ptr_ptr = &weak;
        
        // weak goes out of scope here
        // Control block's weak_count drops to 0
        // Control block is DELETED
    }
    
    // Q: What happens if you try to access weak_ptr_ptr now?
    // A:
    // R:
    
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
