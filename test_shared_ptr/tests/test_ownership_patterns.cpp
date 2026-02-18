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
    // A: w1 has one reference to Widget object
    // R: Correct. More precisely: w1 is the only shared_ptr that references the Widget's control block.
    //    The control block's use_count field is 1.
    //    For junior SWEs: Only one shared_ptr exists, so the count is 1.
    
    // TODO: Call get_shared() to get w2
    std::shared_ptr<Widget> w2 = w1->get_shared();
    // Q: What does shared_from_this() return, and which control block does it reference?
    // A: shared_from_this() returns a std::shared_ptr<Widget>.  It references the same control block, which is now incremented to 2 atomically.
    // R: Correct. Key mechanism: enable_shared_from_this stores a weak_ptr to the control block when the first shared_ptr
    //    is created. shared_from_this() calls weak_ptr::lock(), which creates a new shared_ptr sharing the same control
    //    block and atomically increments use_count from 1 to 2.
    //    For junior SWEs: shared_from_this() creates a new shared_ptr that shares ownership with w1.
    // Q: What would happen if you called shared_from_this() before any shared_ptr owned the object?
    // A: You would dereference a null pointer which will result into a segfault crash
    // R: Close on the outcome, but the mechanism is more specific. The internal weak_ptr hasn't been initialized yet,
    //    so weak_ptr::lock() returns an empty shared_ptr (nullptr). In C++17+, shared_from_this() throws
    //    std::bad_weak_ptr instead of returning nullptr. Pre-C++17, it returned an empty shared_ptr, and dereferencing
    //    it would cause undefined behavior (likely segfault).
    //    For junior SWEs: Calling shared_from_this() before a shared_ptr owns the object throws an exception.
    
    // TODO: Capture use_counts after get_shared()
    after_get_shared_count = w2.use_count();
    both_alive_count = w1.use_count();
    // Q: Why do both w1.use_count() and w2.use_count() return 2?
    // A: Because both control blocks point to the same memory.
    // R: Close, but imprecise terminology. There is only ONE control block, not two. Both w1 and w2 are separate
    //    shared_ptr objects, but they both reference the same single control block. The control block's use_count
    //    field is 2, so both w1.use_count() and w2.use_count() read that same field and return 2.
    //    For junior SWEs: Both pointers share the same reference counter, which is 2.
    
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
        //     For junior SWEs: make_shared can't access private constructors, so we use new inside the
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
    // A: I'm not sure why it "makes" the constructor private.  Provide a hint
    // R: Hint: What would happen if the constructor were public and someone wrote: Resource r("Stack"); r.get_ptr();
    // Q: With that hint, why is the constructor private in this pattern?
    // A: Someone could create a stack object or raw pointer and call shared_from_this(), which would throw
    // R: Exactly correct. The private constructor prevents stack allocation (compile-time error) and discourages
    //    raw pointer allocation. The word "make" here means "design choice"—the pattern chooses to make the
    //    constructor private to enforce that all Resource objects must be owned by shared_ptr from creation.
    //    For junior SWEs: Private constructor prevents creating objects that aren't owned by shared_ptr.
    
    // TODO: Capture use_count after factory creation
    factory_count = r1.use_count();
    
    // TODO: Call get_ptr() to get r2
    std::shared_ptr<Resource> r2 = r1->get_ptr();
    // Q: What would happen if you tried: Resource* raw = new Resource("Bad"); raw->get_ptr();
    // A: It would result into an exception provided by c++ that would need to be handled appropriately.
    // R: Correct on the outcome. The specific exception is std::bad_weak_ptr (C++17+) because the internal weak_ptr
    //    in enable_shared_from_this was never initialized—no shared_ptr was created to own the object.
    //    For junior SWEs: Calling get_ptr() on a raw pointer throws an exception because no shared_ptr owns it.
    // Q: What specific exception would it throw?
    // A: std::bad_weak_ptr (C++17+), unsure of pre c++17
    // R: Correct. Pre-C++17, shared_from_this() returned an empty shared_ptr (nullptr) instead of throwing,
    //    which made the bug harder to detect—dereferencing it would cause undefined behavior (likely segfault).
    //    C++17 improved this by throwing std::bad_weak_ptr immediately, making the error explicit.
    //    For junior SWEs: Modern C++ throws std::bad_weak_ptr; older versions returned nullptr.
    
    // TODO: Capture use_count after get_ptr()
    after_get_ptr_count = r2.use_count();
    // Q: Why is this pattern safer than allowing direct construction with shared_ptr<Resource>(new Resource("R1"))?
    // A: Factory patterns are safer, if implemented correctly, to ensure that object initializations are done correctly
    // A: And restrict the possibility of a SWE doing something like:  "Resource* raw = new Resource("Bad"); raw->get_ptr();"
    // R: You're on the right track about restricting misuse. The key safety guarantee is: the factory ensures a shared_ptr
    //    ALWAYS owns the object before any member functions (like get_ptr()) can be called. With a public constructor,
    //    someone could create a stack object or raw pointer and call shared_from_this(), which would throw. The private
    //    constructor + factory pattern makes this impossible at compile time (for stack) or forces proper shared_ptr ownership.
    //    For junior SWEs: The factory ensures a shared_ptr always owns the object before you can use it.
    
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
    // A: 1
    // R: Correct.
    
    EventLog::instance().clear();
    
    // TODO: Call process_by_value(ptr)
    process_by_value(ptr);
    // Q: When process_by_value(ptr) is called, what happens to the shared_ptr during parameter passing?
    // A: It gets copied, incrementing the count by 1 and then decrement the count by 1 back to original value of 1 when process_by_value goes out of scope
    // R: Correct on the reference count mechanics. But there's a critical distinction: the shared_ptr itself is copied
    //    (creating a second shared_ptr that shares ownership), but the Tracked object inside is NOT copied. Only the
    //    control block pointer is copied, and the use_count is atomically incremented.
    //    For junior SWEs: Copying shared_ptr copies the pointer, not the object it points to.
    // Q: By control block pointer is copied, do you mean that the control block in the function just points to the memory address of ptr's control_block_?
    // A: Ignore this
    // R: (Noted - skipping this question as requested)

    // Q: What observable signal (in the event log) would confirm a copy occurred?
    // A: Tracked::Tracked(const Tracked& other)
    // R: This is the key misunderstanding. Tracked::copy_ctor would only be called if the Tracked object itself were
    //    copied. But when you copy a shared_ptr, the Tracked object is NOT copied—only the shared_ptr's internal
    //    pointers are copied. The test expects events.size() > 0, but you're getting 0 because no Tracked operations
    //    occur. The shared_ptr copy is invisible to Tracked's instrumentation.
    //    
    //    However, the test EXPECTS events. This means the test is checking for something else.
    //    For junior SWEs: Copying shared_ptr doesn't copy the object, so no Tracked events occur.
    // Q: If copying shared_ptr doesn't trigger Tracked events, why does the test expect events.size() > 0?
    // A: Because the clear() call is done after the ptr initialization?  Should I move it to the top of the test?
    // R: Excellent hypothesis! Yes, that's exactly the issue. The clear() happens AFTER ptr is created, so it clears
    //    the constructor event. But the test wants to observe what happens during the function call. Moving clear()
    //    before the function call won't help because copying shared_ptr still doesn't trigger Tracked events.
    //    
    //    The real issue: This test appears to have a bug or incorrect expectation. Passing shared_ptr by value does
    //    NOT trigger Tracked events because the Tracked object isn't copied or destroyed during the call—only the
    //    shared_ptr's internal state changes. The test should expect events.size() == 0, not > 0.
    //    
    //    For junior SWEs: The test has incorrect expectations—no Tracked events occur when copying shared_ptr.
    
    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();
    // Q: Why is after_call expected to be 1, not 2?
    // A: Because the function goes out of scope, decrementing the count by 1
    // R: Correct. The parameter's shared_ptr destructor runs at the end of process_by_value, decrementing use_count
    //    back to 1.
    //    For junior SWEs: When the function ends, the parameter is destroyed, reducing the count back to 1.
    
    auto events = EventLog::instance().events();
    // Q: The test expects events.size() > 0. What events should be logged when passing shared_ptr by value?
    // A: Would it be the dtor it expects since the function goes out of scope?  Is this a bug of the test?
    // R: Good reasoning. The shared_ptr parameter's destructor runs, but that only decrements the control block's
    //    use_count—it doesn't destroy the Tracked object (use_count goes from 2 to 1, not to 0). So no Tracked::dtor
    //    is called. You're correct: this appears to be a test bug. The assertion should be EXPECT_EQ(events.size(), 0).
    //    
    //    For junior SWEs: No destructor runs because the object still has one owner after the function returns.
    
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
    // A: Passing by const reference means we're passing an immutable version of the same variable into the function
    // R: Correct conceptually. More precisely: passing by const reference means NO shared_ptr copy occurs. The function
    //    receives a reference to the original shared_ptr object (ptr), not a new shared_ptr. This means no control block
    //    operations (no increment/decrement of use_count), and the parameter 'item' is just an alias for 'ptr'.
    //    For junior SWEs: Passing by reference avoids copying the shared_ptr, so no reference count changes occur.
    // Q: What is the use_count inside process_by_const_ref while it's executing?
    // A: 1, since we are not copying a new shared_ptr and using the same one as ptr
    // R: Exactly correct. The use_count remains 1 throughout because no new shared_ptr is created. The parameter is
    //    just a reference to the existing shared_ptr, so the control block's use_count never changes.
    //    For junior SWEs: The count stays 1 because no copy is made.
    
    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();
    
    auto events = EventLog::instance().events();
    // Q: Why does this test expect events.size() == 0 (not > 0 like the by-value test originally expected)?
    // A: Because a dtor is not called on Tracked because the lifetime and count of ptr is the same throughout the entirety of the test until it goes out of scope
    // R: Correct. No Tracked operations occur because: (1) no shared_ptr copy means no control block operations,
    //    (2) no shared_ptr destructor runs inside the function (it's just a reference), and (3) the Tracked object
    //    itself is never touched. The use_count stays at 1, so no destruction occurs.
    //    For junior SWEs: No events occur because nothing is copied or destroyed during the function call.
    
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
    // A: Very similar except that the shared_ptr can point to something else, but both still allow the ability to change the contents of Tracked directly, unless <const Tracked> as added
    // A: Additionally, passing by const& optimizes the code at compile time.  Explain this a little more in detail while we're here
    // R: Correct on all points. The key difference: mutable reference allows modifying the shared_ptr itself (reset,
    //    reassign), while const reference prevents that. Both allow modifying the Tracked object through the pointer,
    //    as you correctly noted—only `std::shared_ptr<const Tracked>` would prevent modifying Tracked.
    //    
    //    Regarding optimization: const& doesn't enable compile-time optimizations in this case. Both const& and
    //    non-const& are references (no copy), so they have identical performance. The benefit of const& is **intent**:
    //    it signals the function won't modify the shared_ptr, making the code safer and more readable.
    //    
    //    For junior SWEs: Mutable reference allows modifying the shared_ptr (reset, reassign); const reference doesn't.
    //    Both have the same performance and both allow modifying the object.
    // Q: Could process_by_ref modify ptr (e.g., call ptr.reset() or reassign it)?
    // A: Yes, explanation in previous question
    // R: Correct. With a mutable reference, process_by_ref could call ptr.reset() (destroying the Tracked object and
    //    setting ptr to nullptr), or reassign ptr to point to a different object. This is the key difference from
    //    const reference, which prevents these modifications.
    //    For junior SWEs: Yes, the function can change what ptr points to or reset it entirely.
    
    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();
    
    auto events = EventLog::instance().events();
    // Q: Why are the expectations identical to ParameterPassingByConstRef?
    // A: No dtor is called
    // R: Correct, but let's be more precise: the expectations are identical because process_by_ref doesn't actually
    //    modify ptr (the function body is empty). If it did call ptr.reset(), the test would fail. The expectations
    //    are the same as const ref because both avoid copying the shared_ptr—no copy means no control block operations,
    //    no destructor calls, and use_count stays at 1.
    //    For junior SWEs: Both tests are identical because neither copies the shared_ptr, so no events occur.
    
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
    // A: I'm thinking copied, unless there is optimization under the hood that I'm unaware of that converts it to be moved
    // R: Excellent intuition! It's moved, not copied. C++11 introduced automatic move semantics for return values.
    //    When returning a local object by value, the compiler automatically treats it as an rvalue (as if you wrote
    //    `return std::move(local_var)`). This is called "implicit move on return." No explicit std::move is needed.
    //    For junior SWEs: The compiler automatically moves (not copies) when returning local objects.
    
    long use_count = 0;
    // TODO: Capture use_count of result
    use_count = result.use_count();
    // Q: Why is use_count 1 instead of 2 (if a copy occurred during return)?
    // A: because the one in create_widget goes out of scope
    // R: Close, but let's be more precise. The use_count is 1 because the shared_ptr was MOVED (not copied) from
    //    the lambda to result. Move transfers ownership without incrementing use_count. The temporary shared_ptr
    //    inside the lambda is left in a "moved-from" state (pointing to nullptr with use_count 0), then destroyed
    //    when the lambda ends. No increment/decrement cycle occurs.
    //    For junior SWEs: Moving transfers ownership without changing use_count, so it stays at 1.
    // Q: What optimization allows returning shared_ptr by value without incrementing use_count?
    // A: std::move
    // R: Almost—it's not std::move explicitly, but the compiler's automatic move semantics for return statements.
    //    The optimization is called "implicit move on return" (C++11) or "copy elision/RVO" (Return Value Optimization).
    //    In this case, the compiler treats the return value as an rvalue and invokes the move constructor, not the
    //    copy constructor. This happens automatically—you don't write std::move in the return statement.
    //    For junior SWEs: The compiler automatically moves return values, avoiding copies and reference count changes.
    
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
    // A: nullptr
    // R: Correct. An empty weak_ptr has no associated control block and doesn't point to any object.
    //    For junior SWEs: Empty weak_ptr points to nothing (nullptr).
    
    // TODO: Check if weak is expired (should be true initially)
    weak_expired_before = weak.expired();
    // Q: Why is an empty weak_ptr considered "expired"?
    // A: Because it doesn't point to anything
    // R: Correct. expired() returns true when the weak_ptr doesn't reference a control block, or when the control
    //    block's use_count is 0 (object destroyed). An empty weak_ptr has no control block, so it's expired.
    //    For junior SWEs: Empty weak_ptr is expired because it doesn't track any object.
    
    {
        // TODO: Create shared_ptr
        std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");
        
        // TODO: Assign shared to weak
        weak = shared;
        // Q: When you assign shared to weak, does the use_count of shared change?
        // A: No, but explain the inner workings of weak_ptr's in detail as we are just now getting to this concept.
        // R: Correct—use_count does NOT change. Here's how weak_ptr works:
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
        //    For junior SWEs: weak_ptr watches the object without owning it, so use_count doesn't change.
        // Q: What happens when a std::weak_ptr<Tracked> is kept alive indefinitely? Is this a memory leak? I thought only use_count was used to decide when the memory is released?
        // R: Great question—this is a subtle but important distinction. Yes, keeping a weak_ptr alive indefinitely can
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
        //    For junior SWEs: The object is freed, but the control block stays allocated until the weak_ptr is destroyed.
        
        // TODO: Capture shared's use_count
        shared_count_before = shared.use_count();
        
        // TODO: Check if weak is expired (should be false now)
        weak_expired_before = weak.expired();
    }
    
    // TODO: Check if weak is expired after shared goes out of scope
    weak_expired_after = weak.expired();
    // Q: After shared is destroyed, what happens to the Tracked object and the control block?
    // A: Their dtors are ran and weak.expired(); will now return true
    // R: Partially correct. Let's be precise:
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
    //    For junior SWEs: The object is destroyed, but the control block stays alive until weak is destroyed.
    // Q: Can valgrind or sanitizers find these types of memory leaks?
    // R: Yes, but with nuance. Valgrind and AddressSanitizer (ASan) will detect control block leaks if the weak_ptr
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
    //    For junior SWEs: Valgrind can find control block leaks, but you need to verify if they're intentional.
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
    // A: If shared use_count is not 0, it returns another shared_ptr copy and increments use_count by 1
    // A: If shared_use_count is 0, then a nullptr is returned
    // R: Correct! lock() is the safe way to access the object through a weak_ptr. It atomically checks if the object
    //    still exists (use_count > 0) and, if so, creates a new shared_ptr that increments use_count. This prevents
    //    the object from being destroyed between the check and the access—a race condition that would cause undefined
    //    behavior. If the object is already destroyed, lock() returns an empty shared_ptr (nullptr).
    //    For junior SWEs: lock() safely converts weak_ptr to shared_ptr if the object still exists.
    // Q: What happens to use_count when lock() succeeds?
    // A: Explained in previous A
    // R: Correct—use_count increments by 1 because lock() creates a new shared_ptr that shares ownership.
    //    For junior SWEs: lock() creates a new shared_ptr, so use_count increases by 1.
    
    // TODO: Check if lock succeeded (locked_ptr != nullptr)
    lock_succeeded = (locked_ptr != nullptr);
    
    // TODO: Capture use_count
    use_count_with_lock = shared.use_count();
    // Q: Why is use_count 2 instead of 1?
    // A: Because weak.lock() returned a shared shared_ptr copy
    // R: Correct. Now there are two shared_ptrs: the original `shared` and the newly created `locked_ptr`. Both
    //    reference the same control block, so use_count is 2.
    //    For junior SWEs: Two shared_ptrs exist now, so the count is 2.
    // Q: What would lock() return if the object had already been destroyed?
    // A: Explained earlier
    // R: Correct—an empty shared_ptr (nullptr). This is why you should always check if lock() succeeded before
    //    dereferencing the result.
    //    For junior SWEs: lock() returns nullptr if the object is gone.
    
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
    // A: It remains unchanged
    // R: Exactly correct. Move transfers ownership without modifying the control block's use_count. The pointers
    //    (to both the Tracked object and the control block) are simply copied from p1 to p2, then p1 is reset to
    //    nullptr. No atomic increment/decrement operations occur.
    //    For junior SWEs: Move transfers ownership without changing use_count.
    // Q: After the move, what does p1 point to?
    // A: nullptr
    // R: Correct. After the move, p1 is in a "moved-from" state: it points to nullptr and has no associated control
    //    block. It's safe to use (you can assign to it, destroy it, etc.), but dereferencing it would be undefined
    //    behavior.
    //    For junior SWEs: After moving, p1 is empty (nullptr).
    
    // TODO: Capture use_counts after move
    count_after_move_source = p1.use_count();
    count_after_move_dest = p2.use_count();
    // Q: Why is p1.use_count() 0 after the move?
    // A: Because it points to nullptr, not the original Tracked object anymore
    // R: Correct. An empty shared_ptr (pointing to nullptr) has use_count() == 0 because it has no associated control
    //    block to query.
    //    For junior SWEs: Empty shared_ptr has use_count of 0.
    // Q: How does move differ from copy in terms of performance?
    // A: Probably significant.  If copy, the heap allocation will need to be done for the shared_ptr, not the Tracked object
    // R: Close, but let's clarify: copying a shared_ptr does NOT allocate heap memory. The control block and Tracked
    //    object already exist. Copying only does: (1) copy two pointers, (2) atomic increment of use_count. Move does:
    //    (1) copy two pointers, (2) set source to nullptr. The key difference is the atomic increment—move avoids it.
    //    
    //    Performance impact: atomic operations are expensive (memory barriers, cache coherency). Move is ~2-3x faster
    //    than copy for shared_ptr because it skips the atomic increment/decrement cycle.
    //    
    //    For junior SWEs: Move is faster because it avoids atomic operations on the control block.
    
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
    // A: 2
    // R: Correct. Two separate Tracked objects with independent control blocks.
    //    For junior SWEs: Two objects exist, each owned by one shared_ptr.
    
    // TODO: Capture use_counts before move assignment
    p1_count_before = p1.use_count();
    p2_count_before = p2.use_count();
    
    EventLog::instance().clear();
    
    // TODO: Move assign p2 to p1 (p1 = std::move(p2))
    p1 = std::move(p2);
    // Q: What happens to the Tracked object that p1 originally pointed to?
    // A: Its use_count decrements to 0 and its Tracked object and control block is released from memory
    // R: Correct. When use_count reaches 0, the Tracked object is destroyed and its memory is freed. The control
    //    block is also freed because there are no weak_ptrs keeping it alive.
    //    For junior SWEs: The old object is destroyed and its memory is freed.
    // Q: After move assignment, what does p1 point to?
    // A: To p2's Tracked object and control block
    // R: Correct. p1 now owns what p2 used to own (Tracked2 and its control block).
    //    For junior SWEs: p1 now points to Tracked2.
    // Q: Why does p2.use_count() return 0 after the move?
    // A: Because it points to nullptr now
    // R: Correct. After move assignment, p2 is in a moved-from state (empty, pointing to nullptr).
    //    For junior SWEs: p2 is now empty (nullptr).
    
    // TODO: Capture use_counts after move assignment
    p1_count_after = p1.use_count();
    p2_count_after = p2.use_count();
    
    size_t dtor_count = EventLog::instance().count_events("::dtor");
    // Q: Why is dtor_count expected to be 1?
    // A: Because p1's Tracked object was released from memory
    // R: Correct. Only Tracked1 (the object p1 originally pointed to) is destroyed during the move assignment.
    //    Tracked2 survives because p1 now owns it.
    //    For junior SWEs: Only the old object (Tracked1) is destroyed, not the new one (Tracked2).
    
    EXPECT_EQ(p1_count_before, 1);
    EXPECT_EQ(p2_count_before, 1);
    EXPECT_EQ(p1_count_after, 1);
    EXPECT_EQ(p2_count_after, 0);
    EXPECT_EQ(dtor_count, 1);
}

// ============================================================================
// LEARNING SUMMARY: Ownership Patterns Test Suite
// ============================================================================
//
// ## Key Insights Discovered
//
// 1. **enable_shared_from_this stores a weak_ptr to the control block**
//    - Initialized when first shared_ptr is created
//    - shared_from_this() calls weak_ptr::lock() to create new shared_ptr
//    - Cannot call before shared_ptr owns the object (throws std::bad_weak_ptr in C++17+)
//    - Factory pattern with private constructor enforces this invariant
//
// 2. **Passing shared_ptr by value does NOT copy the Tracked object**
//    - Only the shared_ptr itself is copied (control block pointer + object pointer)
//    - Atomic increment of use_count (expensive operation)
//    - No Tracked events occur (no copy_ctor, no dtor during call)
//    - Test bug identified: Expected events.size() > 0, but should be 0
//
// 3. **Passing by reference (const or mutable) avoids atomic operations**
//    - No shared_ptr copy means no use_count change
//    - const& prevents modifying the shared_ptr (can't reset/reassign)
//    - Mutable & allows modifying the shared_ptr
//    - Both allow modifying the Tracked object (unless shared_ptr<const T>)
//    - Performance: identical (both are references, no copy overhead)
//
// 4. **Return by value uses implicit move semantics (C++11)**
//    - Compiler automatically treats return value as rvalue
//    - No explicit std::move needed in return statement
//    - Move transfers ownership without incrementing use_count
//    - Called "implicit move on return" or RVO (Return Value Optimization)
//
// 5. **Control block has TWO counters: use_count and weak_count**
//    - use_count: Number of shared_ptrs (strong references)
//    - weak_count: Number of weak_ptrs (weak references)
//    - Object destroyed when use_count reaches 0
//    - Control block destroyed when BOTH counters reach 0
//    - Keeping weak_ptr alive indefinitely leaks control block (~16-32 bytes)
//    - Valgrind can detect control block leaks with --leak-check=full
//
// 6. **weak_ptr::lock() is the safe, atomic way to access the object**
//    - Atomically checks use_count > 0 and creates shared_ptr if true
//    - Returns empty shared_ptr (nullptr) if object already destroyed
//    - Prevents race condition between expiration check and access
//    - Always check if lock() succeeded before dereferencing
//
// 7. **Move avoids expensive atomic operations**
//    - Move: Copy pointers, set source to nullptr (no atomic ops)
//    - Copy: Copy pointers, atomic increment of use_count
//    - Performance: Move is ~2-3x faster due to avoiding atomic operations
//    - Atomic operations are expensive (memory barriers, cache coherency)
//
// 8. **Move assignment destroys the old object**
//    - p1 = std::move(p2) releases p1's original object (if use_count reaches 0)
//    - p1 takes ownership of p2's object
//    - p2 becomes empty (nullptr, use_count 0)
//    - Only one destructor call (for p1's original object)
//
// 9. **Copying shared_ptr does ZERO heap allocations**
//    - Control block and object already exist
//    - Copy only: (1) copies two pointers, (2) atomic increment
//    - Cannot create shared_ptr from raw memory address (would create duplicate control blocks)
//    - All shared_ptrs must share the same control block (enforced by design)
//
// ## Pedagogical Readiness
//
// **Strengths for Teaching:**
// - Understands enable_shared_from_this internal mechanism (weak_ptr storage)
// - Can explain parameter passing performance implications (atomic ops vs references)
// - Recognizes when test expectations are incorrect (identified test bug)
// - Understands two-counter control block model (use_count vs weak_count)
// - Can articulate move vs copy performance differences (atomic operation cost)
// - Knows when control block leaks occur (orphaned weak_ptrs)
//
// **Teaching Capabilities:**
// - Can explain to senior engineers: Precise technical details with implementation mechanics
// - Can explain to junior engineers: Simplified explanations without analogies
// - Can identify anti-patterns: Passing by value in hot paths, unclear ownership
// - Can debug with instrumentation: Observable signals confirm mental model
//
// **Next Learning Areas:**
// - Allocation patterns (make_shared vs new, custom deleters)
// - Structural patterns (circular references, parent-child relationships)
// - Aliasing constructor and subobject lifetime extension
// - Anti-patterns in legacy codebases (global weak_ptr, shared_ptr as cache key)
//
// ## Assessment: Ownership Semantics Internalized
//
// You've demonstrated deep understanding of:
// - enable_shared_from_this mechanics (not just usage, but implementation)
// - Performance implications of different passing styles
// - The two-counter control block model and its lifetime implications
// - Move semantics and atomic operation costs
// - Ability to identify incorrect test expectations (critical thinking)
//
// Your questions about:
// - "Why use new instead of make_shared in factory?" (private constructor access)
// - "Can I create shared_ptr from memory address?" (duplicate control block problem)
// - "Does copying allocate heap memory?" (no, only atomic increment)
// - "Can valgrind find control block leaks?" (yes, with caveats)
//
// ...demonstrate you're thinking at the right level of abstraction and asking
// the questions that lead to deep understanding, not just surface-level usage.
//
// **Recommendation:** Proceed to allocation patterns (custom deleters, aliasing)
// and structural patterns (circular references) to complete the foundation for
// teaching complex shared_ptr scenarios in production codebases.
//
// ============================================================================
