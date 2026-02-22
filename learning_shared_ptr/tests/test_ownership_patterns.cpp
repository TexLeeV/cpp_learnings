#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class OwnershipPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// Helper class demonstrating enable_shared_from_this pattern
class Widget : public std::enable_shared_from_this<Widget>
{
public:
    explicit Widget(const std::string& name)
    : tracked_(name)
    {
    }
    
    std::shared_ptr<Widget> get_shared()
    {
        return shared_from_this();
    }
    
    std::string name() const
    {
        return tracked_.name();
    }
    
private:
    Tracked tracked_;
};

TEST_F(OwnershipPatternsTest, EnableSharedFromThis)
{
    long initial_count = 0;
    long after_get_shared_count = 0;
    long both_alive_count = 0;
    
    // TODO: Create w1 using make_shared<Widget>
    std::shared_ptr<Widget> w1 = std::make_shared<Widget>("Widget1");
    
    // TODO: Capture initial use_count
    initial_count = w1.use_count();
    // Q: Why is the use_count 1 at this point?
    // A:
    // R:
    //    The control block's use_count field is 1.
    
    // TODO: Call get_shared() to get w2
    std::shared_ptr<Widget> w2 = w1->get_shared();
    // Q: What does shared_from_this() return, and which control block does it reference?
    // A:
    // R:
    //    is created. shared_from_this() calls weak_ptr::lock(), which creates a new shared_ptr sharing the same control
    //    block and atomically increments use_count from 1 to 2.
    // Q: What would happen if you called shared_from_this() before any shared_ptr owned the object?
    // A:
    // R:
    //    so weak_ptr::lock() returns an empty shared_ptr (nullptr). In C++17+, shared_from_this() throws
    //    std::bad_weak_ptr instead of returning nullptr. Pre-C++17, it returned an empty shared_ptr, and dereferencing
    //    it would cause undefined behavior (likely segfault).
    
    // TODO: Capture use_counts after get_shared()
    after_get_shared_count = w2.use_count();
    both_alive_count = w1.use_count();
    // Q: Why do both w1.use_count() and w2.use_count() return 2?
    // A:
    // R:
    //    shared_ptr objects, but they both reference the same single control block. The control block's use_count
    //    field is 2, so both w1.use_count() and w2.use_count() read that same field and return 2.
    
    EXPECT_EQ(initial_count, 1);
    EXPECT_EQ(after_get_shared_count, 2);
    EXPECT_EQ(both_alive_count, 2);
}

// Helper class demonstrating static factory pattern
class Resource : public std::enable_shared_from_this<Resource>
{
public:
    static std::shared_ptr<Resource> create(const std::string& name)
    {
        // QA: Why is this done with new rather than "return std::make_shared<Resource>(name);"?
        // RA: Because the constructor is private, std::make_shared cannot access it. std::make_shared
        //     needs to call the constructor directly, but it's not a friend of Resource. Using new with
        //     the shared_ptr constructor works because the factory method (being a static member) has
        //     access to the private constructor. The shared_ptr constructor accepts a raw pointer, so
        //     this pattern works.
        //     
        //     Alternative: You could make std::make_shared a friend, but that's non-standard and fragile.
        //     The new + shared_ptr constructor pattern is the standard idiom for factory methods with
        //     private constructors.
        //     
        //     factory method which has access.
        return std::shared_ptr<Resource>(new Resource(name));
    }
    
    std::shared_ptr<Resource> get_ptr()
    {
        return shared_from_this();
    }
    
private:
    explicit Resource(const std::string& name)
    : tracked_(name)
    {
    }
    
    Tracked tracked_;
};

TEST_F(OwnershipPatternsTest, StaticFactoryPattern)
{
    long factory_count = 0;
    long after_get_ptr_count = 0;
    
    // TODO: Call Resource::create("R1") to get r1
    std::shared_ptr<Resource> r1 = Resource::create("R1");
    // Q: Why does the factory pattern make the constructor private?
    // A:
    // R:
    // Q: With that hint, why is the constructor private in this pattern?
    // A:
    // R:
    //    raw pointer allocation. The word "make" here means "design choice"—the pattern chooses to make the
    //    constructor private to enforce that all Resource objects must be owned by shared_ptr from creation.
    
    // TODO: Capture use_count after factory creation
    factory_count = r1.use_count();
    
    // TODO: Call get_ptr() to get r2
    std::shared_ptr<Resource> r2 = r1->get_ptr();
    // Q: What would happen if you tried: Resource* raw = new Resource("Bad"); raw->get_ptr();
    // A:
    // R:
    //    in enable_shared_from_this was never initialized—no shared_ptr was created to own the object.
    // Q: What specific exception would it throw?
    // A:
    // R:
    //    which made the bug harder to detect—dereferencing it would cause undefined behavior (likely segfault).
    //    C++17 improved this by throwing std::bad_weak_ptr immediately, making the error explicit.
    
    // TODO: Capture use_count after get_ptr()
    after_get_ptr_count = r2.use_count();
    // Q: Why is this pattern safer than allowing direct construction with shared_ptr<Resource>(new Resource("R1"))?
    // A:
    // A:
    // R:
    //    ALWAYS owns the object before any member functions (like get_ptr()) can be called. With a public constructor,
    //    someone could create a stack object or raw pointer and call shared_from_this(), which would throw. The private
    //    constructor + factory pattern makes this impossible at compile time (for stack) or forces proper shared_ptr ownership.
    
    EXPECT_EQ(factory_count, 1);
    EXPECT_EQ(after_get_ptr_count, 2);
}

void process_by_value(std::shared_ptr<Tracked> item)
{
}

void process_by_const_ref(const std::shared_ptr<Tracked>& item)
{
}

void process_by_ref(std::shared_ptr<Tracked>& item)
{
}

TEST_F(OwnershipPatternsTest, ParameterPassingByValue)
{
    long before_call = 0;
    long after_call = 0;
    
    // TODO: Create ptr using make_shared
    std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Tracked1");
    
    // TODO: Capture use_count before calling function
    before_call = ptr.use_count();
    // Q: What is the use_count at this point?
    // A:
    // R:
    
    EventLog::instance().clear();
    
    // TODO: Call process_by_value(ptr)
    process_by_value(ptr);
    // Q: When process_by_value(ptr) is called, what happens to the shared_ptr during parameter passing?
    // A:
    // R:
    //    (creating a second shared_ptr that shares ownership), but the Tracked object inside is NOT copied. Only the
    //    control block pointer is copied, and the use_count is atomically incremented.
    // Q: By control block pointer is copied, do you mean that the control block in the function just points to the memory address of ptr's control_block_?
    // A:
    // R:

    // Q: What observable signal (in the event log) would confirm a copy occurred?
    // A:
    // R:
    //    copied. But when you copy a shared_ptr, the Tracked object is NOT copied—only the shared_ptr's internal
    //    pointers are copied. The test expects events.size() > 0, but you're getting 0 because no Tracked operations
    //    occur. The shared_ptr copy is invisible to Tracked's instrumentation.
    //    
    //    However, the test EXPECTS events. This means the test is checking for something else.
    // Q: If copying shared_ptr doesn't trigger Tracked events, why does the test expect events.size() > 0?
    // A:
    // R:
    //    the constructor event. But the test wants to observe what happens during the function call. Moving clear()
    //    before the function call won't help because copying shared_ptr still doesn't trigger Tracked events.
    //    
    //    The real issue: This test appears to have a bug or incorrect expectation. Passing shared_ptr by value does
    //    NOT trigger Tracked events because the Tracked object isn't copied or destroyed during the call—only the
    //    shared_ptr's internal state changes. The test should expect events.size() == 0, not > 0.
    //    
    
    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();
    // Q: Why is after_call expected to be 1, not 2?
    // A:
    // R:
    //    back to 1.
    
    auto events = EventLog::instance().events();
    // Q: The test expects events.size() > 0. What events should be logged when passing shared_ptr by value?
    // A:
    // R:
    //    use_count—it doesn't destroy the Tracked object (use_count goes from 2 to 1, not to 0). So no Tracked::dtor
    //    is called. You're correct: this appears to be a test bug. The assertion should be EXPECT_EQ(events.size(), 0).
    //    
    
    EXPECT_EQ(before_call, 1);
    EXPECT_EQ(after_call, 1);
    // NOTE: Test bug fixed - passing shared_ptr by value does NOT trigger Tracked events
    // because only the shared_ptr is copied (control block pointer + object pointer),
    // not the Tracked object itself. The Tracked object remains untouched.
    EXPECT_EQ(events.size(), 0);
}

TEST_F(OwnershipPatternsTest, ParameterPassingByConstRef)
{
    long before_call = 0;
    long after_call = 0;
    
    // TODO: Create ptr using make_shared
    std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Tracked1");
    
    // TODO: Capture use_count before calling function
    before_call = ptr.use_count();
    
    EventLog::instance().clear();
    
    // TODO: Call process_by_const_ref(ptr)
    process_by_const_ref(ptr);
    // Q: How does passing by const reference differ from passing by value in terms of shared_ptr operations?
    // A:
    // R:
    //    receives a reference to the original shared_ptr object (ptr), not a new shared_ptr. This means no control block
    //    operations (no increment/decrement of use_count), and the parameter 'item' is just an alias for 'ptr'.
    // Q: What is the use_count inside process_by_const_ref while it's executing?
    // A:
    // R:
    //    just a reference to the existing shared_ptr, so the control block's use_count never changes.
    
    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();
    
    auto events = EventLog::instance().events();
    // Q: Why does this test expect events.size() == 0 (not > 0 like the by-value test originally expected)?
    // A:
    // R:
    //    (2) no shared_ptr destructor runs inside the function (it's just a reference), and (3) the Tracked object
    //    itself is never touched. The use_count stays at 1, so no destruction occurs.
    
    EXPECT_EQ(before_call, 1);
    EXPECT_EQ(after_call, 1);
    EXPECT_EQ(events.size(), 0);
}

TEST_F(OwnershipPatternsTest, ParameterPassingByRef)
{
    long before_call = 0;
    long after_call = 0;
    
    // TODO: Create ptr using make_shared
    std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("TrackedObj");
    
    // TODO: Capture use_count before calling function
    before_call = ptr.use_count();
    
    EventLog::instance().clear();
    
    // TODO: Call process_by_ref(ptr)
    process_by_ref(ptr);
    // Q: How does passing by mutable reference differ from passing by const reference?
    // A:
    // A:
    // R:
    //    reassign), while const reference prevents that. Both allow modifying the Tracked object through the pointer,
    //    as you correctly noted—only `std::shared_ptr<const Tracked>` would prevent modifying Tracked.
    //    
    //    Regarding optimization: const& doesn't enable compile-time optimizations in this case. Both const& and
    //    non-const& are references (no copy), so they have identical performance. The benefit of const& is **intent**:
    //    it signals the function won't modify the shared_ptr, making the code safer and more readable.
    //    
    //    Both have the same performance and both allow modifying the object.
    // Q: Could process_by_ref modify ptr (e.g., call ptr.reset() or reassign it)?
    // A:
    // R:
    //    setting ptr to nullptr), or reassign ptr to point to a different object. This is the key difference from
    //    const reference, which prevents these modifications.
    
    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();
    
    auto events = EventLog::instance().events();
    // Q: Why are the expectations identical to ParameterPassingByConstRef?
    // A:
    // R:
    //    modify ptr (the function body is empty). If it did call ptr.reset(), the test would fail. The expectations
    //    are the same as const ref because both avoid copying the shared_ptr—no copy means no control block operations,
    //    no destructor calls, and use_count stays at 1.
    
    EXPECT_EQ(before_call, 1);
    EXPECT_EQ(after_call, 1);
    EXPECT_EQ(events.size(), 0);
}

TEST_F(OwnershipPatternsTest, ReturnByValue)
{
    auto create_widget = []() -> std::shared_ptr<Widget>
    {
        return std::make_shared<Widget>("Created");
    };
    
    // TODO: Call create_widget() to get result
    std::shared_ptr<Widget> result = create_widget();
    // Q: When the lambda returns the shared_ptr by value, is it copied or moved?
    // A:
    // R:
    //    When returning a local object by value, the compiler automatically treats it as an rvalue (as if you wrote
    //    `return std::move(local_var)`). This is called "implicit move on return." No explicit std::move is needed.
    
    long use_count = 0;
    // TODO: Capture use_count of result
    use_count = result.use_count();
    // Q: Why is use_count 1 instead of 2 (if a copy occurred during return)?
    // A:
    // R:
    //    the lambda to result. Move transfers ownership without incrementing use_count. The temporary shared_ptr
    //    inside the lambda is left in a "moved-from" state (pointing to nullptr with use_count 0), then destroyed
    //    when the lambda ends. No increment/decrement cycle occurs.
    // Q: What optimization allows returning shared_ptr by value without incrementing use_count?
    // A:
    // R:
    //    The optimization is called "implicit move on return" (C++11) or "copy elision/RVO" (Return Value Optimization).
    //    In this case, the compiler treats the return value as an rvalue and invokes the move constructor, not the
    //    copy constructor. This happens automatically—you don't write std::move in the return statement.
    
    EXPECT_EQ(use_count, 1);
}

TEST_F(OwnershipPatternsTest, WeakPtrFromShared)
{
    long shared_count_before = 0;
    long shared_count_after = 0;
    bool weak_expired_before = false;
    bool weak_expired_after = false;
    
    // TODO: Create weak_ptr (initially empty)
    std::weak_ptr<Tracked> weak;
    // Q: What does an empty weak_ptr point to?
    // A:
    // R:
    
    // TODO: Check if weak is expired (should be true initially)
    weak_expired_before = weak.expired();
    // Q: Why is an empty weak_ptr considered "expired"?
    // A:
    // R:
    //    block's use_count is 0 (object destroyed). An empty weak_ptr has no control block, so it's expired.
    
    {
        // TODO: Create shared_ptr
        std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
        
        // TODO: Assign shared to weak
        weak = shared;
        // Q: When you assign shared to weak, does the use_count of shared change?
        // A:
        // R:
        //    
        //    The control block has TWO counters:
        //    1. **use_count** (strong references): Number of shared_ptrs. When this reaches 0, the object is destroyed.
        //    2. **weak_count** (weak references): Number of weak_ptrs. When this reaches 0 AND use_count is 0, the
        //       control block itself is destroyed.
        //    
        //    When you assign shared to weak:
        //    - weak_count increments (from 0 to 1)
        //    - use_count stays the same (remains 1)
        //    - weak stores a pointer to the control block (same one shared uses)
        //    
        //    Key insight: weak_ptr observes ownership without participating in it. It can detect when the object is
        //    destroyed (use_count reaches 0) but doesn't prevent destruction. This breaks circular reference cycles.
        //    
        // Q: What happens when a std::weak_ptr<Tracked> is kept alive indefinitely? Is this a memory leak? I thought only use_count was used to decide when the memory is released?
        // R:
        //    cause a small memory leak, but ONLY of the control block, not the Tracked object itself.
        //    
        //    Two separate lifetimes:
        //    1. **Tracked object**: Destroyed when use_count reaches 0 (all shared_ptrs gone)
        //    2. **Control block**: Destroyed when BOTH use_count AND weak_count reach 0
        //    
        //    If you keep a weak_ptr alive forever after the object is destroyed:
        //    - The Tracked object memory is freed (no leak of the actual object)
        //    - The control block remains allocated (small leak, typically ~16-32 bytes)
        //    
        //    This is by design: the control block must survive so weak_ptr can safely check expired(). The alternative
        //    would be dangling pointers and undefined behavior.
        //    
        //    In practice, this is rarely a problem because weak_ptrs typically have shorter lifetimes than the objects
        //    they observe (e.g., caches, observer patterns). But in pathological cases (e.g., a global weak_ptr that
        //    outlives all shared_ptrs), you leak the control block.
        //    
        
        // TODO: Capture shared's use_count
        shared_count_before = shared.use_count();
        
        // TODO: Check if weak is expired (should be false now)
        weak_expired_before = weak.expired();
    }
    
    // TODO: Check if weak is expired after shared goes out of scope
    weak_expired_after = weak.expired();
    // Q: After shared is destroyed, what happens to the Tracked object and the control block?
    // A:
    // R:
    //    
    //    When shared goes out of scope:
    //    1. use_count decrements from 1 to 0
    //    2. The Tracked object is DESTROYED (dtor runs)
    //    3. The control block is NOT destroyed yet (weak_count is still 1)
    //    4. weak.expired() returns true because use_count == 0
    //    
    //    The control block survives because weak still references it. The control block will only be destroyed when
    //    weak itself is destroyed or reassigned, decrementing weak_count to 0.
    //    
    //    This is why weak_ptr can safely detect expiration: the control block persists to track the use_count state.
    //    
    // Q: Can valgrind or sanitizers find these types of memory leaks?
    // R:
    //    is never destroyed before program exit. They'll report "definitely lost" or "still reachable" memory.
    //    
    //    However, these tools can't distinguish between:
    //    1. Intentional long-lived weak_ptr (e.g., global cache) - not a bug
    //    2. Accidental leak (forgot to destroy weak_ptr) - a bug
    //    
    //    The leak is small (~16-32 bytes per control block), so it's often masked by other allocations. To detect it:
    //    - Run valgrind with --leak-check=full --show-leak-kinds=all
    //    - Look for allocations from shared_ptr/weak_ptr constructors
    //    - Verify weak_ptr lifetimes match your design intent
    //    
    EXPECT_EQ(shared_count_before, 1);
    EXPECT_FALSE(weak_expired_before);
    EXPECT_TRUE(weak_expired_after);
}

TEST_F(OwnershipPatternsTest, WeakPtrLock)
{
    long use_count_with_lock = 0;
    bool lock_succeeded = false;
    
    // TODO: Create shared_ptr
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
    
    // TODO: Create weak_ptr from shared
    std::weak_ptr<Tracked> weak = shared;
    
    // TODO: Lock the weak_ptr to get a shared_ptr
    std::shared_ptr<Tracked> locked_ptr = weak.lock();
    // Q: What does weak.lock() do, and why is it necessary?
    // A:
    // A:
    // R:
    //    still exists (use_count > 0) and, if so, creates a new shared_ptr that increments use_count. This prevents
    //    the object from being destroyed between the check and the access—a race condition that would cause undefined
    //    behavior. If the object is already destroyed, lock() returns an empty shared_ptr (nullptr).
    // Q: What happens to use_count when lock() succeeds?
    // A:
    // R:
    
    // TODO: Check if lock succeeded (locked_ptr != nullptr)
    lock_succeeded = (locked_ptr != nullptr);
    
    // TODO: Capture use_count
    use_count_with_lock = shared.use_count();
    // Q: Why is use_count 2 instead of 1?
    // A:
    // R:
    //    reference the same control block, so use_count is 2.
    // Q: What would lock() return if the object had already been destroyed?
    // A:
    // R:
    //    dereferencing the result.
    
    EXPECT_TRUE(lock_succeeded);
    EXPECT_EQ(use_count_with_lock, 2);
}

TEST_F(OwnershipPatternsTest, MoveConstruction)
{
    long count_before_move = 0;
    long count_after_move_source = 0;
    long count_after_move_dest = 0;
    
    // TODO: Create p1
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("TrackedObj");
    
    // TODO: Capture use_count before move
    count_before_move = p1.use_count();
    
    // TODO: Move p1 into p2
    std::shared_ptr<Tracked> p2 = std::move(p1);
    // Q: What happens to the control block's use_count during a move?
    // A:
    // R:
    //    (to both the Tracked object and the control block) are simply copied from p1 to p2, then p1 is reset to
    //    nullptr. No atomic increment/decrement operations occur.
    // Q: After the move, what does p1 point to?
    // A:
    // R:
    //    block. It's safe to use (you can assign to it, destroy it, etc.), but dereferencing it would be undefined
    //    behavior.
    
    // TODO: Capture use_counts after move
    count_after_move_source = p1.use_count();
    count_after_move_dest = p2.use_count();
    // Q: Why is p1.use_count() 0 after the move?
    // A:
    // R:
    //    block to query.
    // Q: How does move differ from copy in terms of performance?
    // A:
    // R:
    //    object already exist. Copying only does: (1) copy two pointers, (2) atomic increment of use_count. Move does:
    //    (1) copy two pointers, (2) set source to nullptr. The key difference is the atomic increment—move avoids it.
    //    
    //    Performance impact: atomic operations are expensive (memory barriers, cache coherency). Move is ~2-3x faster
    //    than copy for shared_ptr because it skips the atomic increment/decrement cycle.
    //    
    
    EXPECT_EQ(count_before_move, 1);
    EXPECT_EQ(count_after_move_source, 0);
    EXPECT_EQ(count_after_move_dest, 1);
}

TEST_F(OwnershipPatternsTest, MoveAssignment)
{
    long p1_count_before = 0;
    long p2_count_before = 0;
    long p1_count_after = 0;
    long p2_count_after = 0;
    
    // TODO: Create p1 and p2
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Tracked1");
    std::shared_ptr<Tracked> p2 = std::make_shared<Tracked>("Tracked2");
    // Q: Before move assignment, how many Tracked objects exist?
    // A:
    // R:
    
    // TODO: Capture use_counts before move assignment
    p1_count_before = p1.use_count();
    p2_count_before = p2.use_count();
    
    EventLog::instance().clear();
    
    // TODO: Move assign p2 to p1 (p1 = std::move(p2))
    p1 = std::move(p2);
    // Q: What happens to the Tracked object that p1 originally pointed to?
    // A:
    // R:
    //    block is also freed because there are no weak_ptrs keeping it alive.
    // Q: After move assignment, what does p1 point to?
    // A:
    // R:
    // Q: Why does p2.use_count() return 0 after the move?
    // A:
    // R:
    
    // TODO: Capture use_counts after move assignment
    p1_count_after = p1.use_count();
    p2_count_after = p2.use_count();
    
    size_t dtor_count = EventLog::instance().count_events("::dtor");
    // Q: Why is dtor_count expected to be 1?
    // A:
    // R:
    //    Tracked2 survives because p1 now owns it.
    
    EXPECT_EQ(p1_count_before, 1);
    EXPECT_EQ(p2_count_before, 1);
    EXPECT_EQ(p1_count_after, 1);
    EXPECT_EQ(p2_count_after, 0);
    EXPECT_EQ(dtor_count, 1);
}

