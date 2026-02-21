#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <cstdio>

class AllocationPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(AllocationPatternsTest, MakeSharedVsNew)
{
    long new_count = 0;
    long make_shared_count = 0;

    // Q: When you create a shared_ptr using `new`, how many heap allocations occur?
    // A: 2.  One for the Tracked object and one for the control block, where they might not be sequental
    // R: Correct. The object is allocated by `new Tracked()`, then the shared_ptr constructor allocates a separate control block.
    std::shared_ptr<Tracked> p1(new Tracked("New"));
    new_count = p1.use_count();

    // Q: When you create a shared_ptr using `make_shared`, how many heap allocations occur?
    // A: 1.  One for both the Tracked object and control block.  In this case, they are sequential in memory address heap allocation
    // R: Correct. make_shared allocates a single memory block containing both the control block and the object, placed contiguously.
    std::shared_ptr<Tracked> p2 = std::make_shared<Tracked>("MakeShared");
    make_shared_count = p2.use_count();

    // Q: Both use_count values are 1. What does this tell you about the relationship between allocation strategy and reference counting?
    // A: Allocation strategy: Prefer std::make_shared for performance reasons, unless there is some nuance of where new is preffered.  Provide me an example of this case
    // A: Reference counting: Each have their own control block and Tracked object allocated in the heap.  Not sure what else you're trying to get from this question.
    // R: Good observation that allocation strategy is orthogonal to reference counting—both strategies produce use_count of 1 because only one shared_ptr owns the object.
    // R: Case where `new` is preferred: When you need a custom deleter. make_shared cannot take a custom deleter, so you must use `shared_ptr<T>(new T(), custom_deleter)`.
    // R: Another case: When the object is very large and you have weak_ptrs. With make_shared, the object memory cannot be freed until all weak_ptrs expire (control block and object are in same allocation). With `new`, the object memory is freed when use_count hits 0, even if weak_ptrs still exist.
    EXPECT_EQ(new_count, 1);
    EXPECT_EQ(make_shared_count, 1);

    // Q: Given that `new` requires 2 allocations (object + control block) and `make_shared` requires 1 (combined), what are the performance implications?
    // A: two times performance with make_shared
    // R: Not quite 2x—heap allocation overhead is significant but not the only cost. make_shared is faster (fewer allocator calls, better cache behavior), but the speedup depends on object size and allocation patterns. Typical improvement is 10-50% in allocation-heavy code.
    // R: For junior SWEs: make_shared is faster because it only calls the allocator once instead of twice.
    // Q: What happens to cache locality when the object and control block are allocated separately vs together?
    // A: cache hit chance decreased in cpu L1/L2/L3 caches with the "new" allocation approach
    // R: Correct. With `new`, the object and control block are in different memory locations, potentially causing cache misses when accessing both. With make_shared, they're adjacent in memory, increasing the likelihood both fit in the same cache line or nearby lines.
    // R: For junior SWEs: When data is close together in memory, the CPU can load it faster from cache instead of going to slower RAM.
}


class ThrowingResource
{
public:
    explicit ThrowingResource(bool should_throw)
    : tracked_("Throwing")
    {
        if (should_throw)
        {
            throw std::runtime_error("Construction failed");
        }
    }

private:
    Tracked tracked_;
};

void process_with_new(bool throw_first, bool throw_second)
{
    try
    {
        std::shared_ptr<ThrowingResource> p1(new ThrowingResource(throw_first));
        std::shared_ptr<ThrowingResource> p2(new ThrowingResource(throw_second));
    }
    catch (...)
    {
    }
}

void process_with_make_shared(bool throw_first, bool throw_second)
{
    try
    {
        std::shared_ptr<ThrowingResource> p1 = std::make_shared<ThrowingResource>(throw_first);
        std::shared_ptr<ThrowingResource> p2 = std::make_shared<ThrowingResource>(throw_second);
    }
    catch (...)
    {
    }
}

TEST_F(AllocationPatternsTest, ExceptionSafetyMakeShared)
{
    EventLog::instance().clear();

    // Q: When p2's construction throws, what happens to p1? Walk through the stack unwinding.
    // A: even though p1 didn't throw, allocation is still done in the heap but then p2's throw occurs and is caught in catch p1 will then release its resources
    // R: Correct. When p2's constructor throws, the exception propagates out of the try block. During stack unwinding, p1's destructor runs automatically (it's a fully constructed local variable), which decrements the reference count to 0 and destroys the ThrowingResource.
    // R: For junior SWEs: Stack unwinding means C++ automatically calls destructors for all fully-constructed objects when leaving a scope due to an exception.
    // Q: At what point in the statement `std::shared_ptr<T> p1(new T(true))` could an exception leave a memory leak?
    // A: If heap allocation of the control block is done first before the T object allocation in the heap.
    // R: Close, but the order is reversed. The danger is: `new T(true)` allocates and constructs T first, then the shared_ptr constructor allocates the control block. If the control block allocation throws (out of memory), the T object is already allocated but not owned by anything—memory leak.
    // R: This is why `make_shared` is safer: it's a single allocation that's either fully successful or fully rolled back.
    // R: For junior SWEs: With `new T()`, if creating the control block fails, the T object is orphaned in memory with no way to free it.
    process_with_make_shared(false, true);

    auto events = EventLog::instance().events();
    size_t ctor_count = 0;
    size_t dtor_count = 0;

    for (const auto& event : events)
    {
        if (event.find("::ctor") != std::string::npos)
        {
            ++ctor_count;
        }
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }

    // Q: The test expects ctor_count == dtor_count. What does this verify about exception safety?
    // A: "What comes up, must come down" -> "What gets allocated, must be de-allocated" when unwinding the stack to release resources upon an exception
    // R: Correct principle. This verifies strong exception safety: every successfully constructed object (ctor logged) is properly destroyed (dtor logged), even when an exception occurs mid-function. No leaks, no orphaned resources.
    // R: For junior SWEs: If you create 2 objects but only destroy 1, you have a memory leak. This test proves both get cleaned up.
    EXPECT_EQ(ctor_count, dtor_count);
}

struct FileDeleter
{
    void operator()(FILE* file) const
    {
        if (file)
        {
            EventLog::instance().record("FileDeleter::operator() closing file");
            std::fclose(file);
        }
    }
};

TEST_F(AllocationPatternsTest, CustomDeleterFileHandle)
{
    {
        FILE* file = std::tmpfile();

        // Q: Why can't you use make_shared with a custom deleter?
        // A: Because make_shared does not support custom deleters.  The workaround is using make_shared<T>(new T(), custom_deleter)
        // R: Correct that make_shared doesn't support custom deleters. However, your workaround syntax is incorrect—you can't pass a deleter to make_shared at all. The correct approach is `shared_ptr<T>(new T(), custom_deleter)` (what you're already doing in this test).
        // R: The reason: make_shared allocates object + control block in one memory block. Custom deleters need to be stored in the control block, but make_shared's signature doesn't accept them.
        // R: For junior SWEs: make_shared is a template function that doesn't take a deleter parameter, so you must use the shared_ptr constructor instead.
        // Q: Where is the FileDeleter stored—in the control block or with the shared_ptr object itself?
        // A: I would guess with the control block.  Not sure the reasoning why, though.  Explain
        // R: Correct—stored in the control block. Reasoning: When you copy a shared_ptr, all copies must use the same deleter. If the deleter were in the shared_ptr object itself, each copy would need its own deleter copy, wasting memory. By storing it once in the control block, all shared_ptr copies reference the same deleter.
        // R: Additionally, the shared_ptr object must have a fixed size regardless of deleter type (type erasure), so the deleter can't be stored inline.
        // R: For junior SWEs: The control block is shared by all copies, so storing the deleter there means only one copy exists for all shared_ptrs pointing to the same object.
        std::shared_ptr<FILE> file_ptr(file, FileDeleter());
        long use_count = file_ptr.use_count();

        EXPECT_EQ(use_count, 1);
    }

    auto events = EventLog::instance().events();
    bool deleter_called = false;

    for (const auto& event : events)
    {
        if (event.find("FileDeleter::operator()") != std::string::npos)
        {
            deleter_called = true;
        }
    }

    // Q: When exactly is the custom deleter invoked relative to the control block's destruction?
    // A: After the destruction of the control_block signifying no references exist for the File object
    // R: Almost—the sequence is: (1) use_count reaches 0, (2) deleter is invoked to destroy the object, (3) weak_count is checked, (4) if weak_count is also 0, control block itself is destroyed. The deleter runs *before* the control block is destroyed, not after.
    // R: The control block must survive the deleter call because the deleter is stored inside it.
    // R: For junior SWEs: The deleter lives in the control block, so the control block can't be destroyed until after the deleter finishes running.
    EXPECT_TRUE(deleter_called);
}

class Resource
{
public:
    explicit Resource(const std::string& name)
    : tracked_(name)
    {
    }

private:
    Tracked tracked_;
};

struct ResourceDeleter
{
    void operator()(Resource* res) const
    {
        EventLog::instance().record("ResourceDeleter::operator() called");
        delete res;
    }
};

TEST_F(AllocationPatternsTest, CustomDeleterWithTrackedObject)
{
    {
        // Q: What is the sequence of operations when the shared_ptr is destroyed? (deleter call vs object destructor)
        // A: deteler called first then destructor of object
        // R: Correct. Sequence: (1) control block invokes ResourceDeleter::operator(), (2) deleter calls `delete res`, (3) `delete` invokes ~Resource(), (4) ~Resource() invokes ~Tracked() for the member.
        // R: For junior SWEs: The deleter runs first, and inside the deleter, `delete` triggers the object's destructor chain.
        std::shared_ptr<Resource> res_ptr(new Resource("Res1"), ResourceDeleter());
        long use_count = res_ptr.use_count();

        EXPECT_EQ(use_count, 1);
    }

    auto events = EventLog::instance().events();
    bool deleter_called = false;
    bool dtor_called = false;

    for (const auto& event : events)
    {
        if (event.find("ResourceDeleter::operator()") != std::string::npos)
        {
            deleter_called = true;
        }
        if (event.find("::dtor") != std::string::npos)
        {
            dtor_called = true;
        }
    }

    // Q: The deleter calls `delete res`. What happens inside `delete` that triggers the Tracked destructor?
    // A: implicit ~Resource dtor
    // R: Correct. `delete res` does two things: (1) calls ~Resource() destructor, (2) frees the memory. The ~Resource() destructor automatically calls ~Tracked() for the `tracked_` member (compiler-generated destructor behavior for members).
    // R: For junior SWEs: `delete` first runs the destructor, then frees memory. The destructor automatically destroys all member variables.
    EXPECT_TRUE(deleter_called);
    EXPECT_TRUE(dtor_called);
}

TEST_F(AllocationPatternsTest, ArrayAllocationWithCustomDeleter)
{
    {
        // Q: Why must you use a custom deleter for arrays? What happens if you use the default deleter with `new T[]`?
        // A: Because each object instance of the array needs to be manually managed and is too complex for the compiler to provide a viable default deleter for you
        // R: Not quite. The issue is simpler: the default deleter uses `delete ptr`, which is incorrect for arrays allocated with `new T[]`. Using `delete` on an array (instead of `delete[]`) causes undefined behavior—typically only the first element's destructor runs, leaking the other elements.
        // R: The compiler can't provide the right default because shared_ptr doesn't know whether the pointer came from `new T` or `new T[]`.
        // R: For junior SWEs: `new T[]` requires `delete[]`, but shared_ptr's default deleter uses `delete`. You must provide a custom deleter that uses `delete[]`.
        // Q: What is the difference between `delete ptr` and `delete[] ptr`?
        // A: delete ptr: deletes at that current memory address location.  Must be done for each Tracked instance
        // A: delete[] ptr: deletes the actual array itself and releases its resources
        // A: Provide me with the memory address layout of new Tracked[3]{Tracked("Arr1"), Tracked("Arr2"), Tracked("Arr3")}
        // R: Close, but let me clarify the mechanics. Both `delete` and `delete[]` free memory, but they differ in destructor calls:
        // R: - `delete ptr`: Calls destructor for ONE object at ptr, then frees memory
        // R: - `delete[] ptr`: Calls destructor for ALL objects in the array (compiler tracks count), then frees memory
        // R: Memory layout for `new Tracked[3]`:
        // R: ```
        // R: [array_size_metadata: 3] [Tracked("Arr1")] [Tracked("Arr2")] [Tracked("Arr3")]
        // R:  ^-- hidden metadata       ^-- ptr points here
        // R: ```
        // R: `delete[]` uses the hidden metadata to know how many destructors to call. `delete` doesn't check metadata, so it only destroys one object.
        // R: For junior SWEs: `delete[]` knows to call 3 destructors because the compiler stores the array size in hidden metadata before the first element.
        std::shared_ptr<Tracked> arr_ptr(
            new Tracked[3]{Tracked("Arr1"), Tracked("Arr2"), Tracked("Arr3")},
            LoggingArrayDeleter<Tracked>("ArrayDeleter")
        );
        long use_count = arr_ptr.use_count();

        EXPECT_EQ(use_count, 1);
    }

    auto events = EventLog::instance().events();
    bool array_deleter_called = false;

    for (const auto& event : events)
    {
        if (event.find("ArrayDeleter::operator()") != std::string::npos)
        {
            array_deleter_called = true;
        }
    }

    EXPECT_TRUE(array_deleter_called);
}

TEST_F(AllocationPatternsTest, DeleterTypeErasure)
{
    struct Deleter1
    {
        void operator()(Tracked* ptr) const
        {
            EventLog::instance().record("Deleter1::operator()");
            delete ptr;
        }
    };

    struct Deleter2
    {
        void operator()(Tracked* ptr) const
        {
            EventLog::instance().record("Deleter2::operator()");
            delete ptr;
        }
    };

    // Q: Both Deleter1 and Deleter2 have different types. How can p1 and p2 have the same type `shared_ptr<Tracked>`?
    // A: What do you mean by "different types"?
    // R: Deleter1 and Deleter2 are different C++ types (different struct definitions). Yet both p1 and p2 are `shared_ptr<Tracked>` (same type). This is called "type erasure."
    // R: The deleter type is NOT part of shared_ptr's type signature. Compare with unique_ptr: `unique_ptr<T, Deleter1>` vs `unique_ptr<T, Deleter2>` are different types.
    // R: shared_ptr achieves this by storing the deleter polymorphically in the control block (virtual function or function pointer), so the shared_ptr object itself doesn't need to know the deleter's type.
    // R: For junior SWEs: shared_ptr hides the deleter type inside the control block, so all shared_ptr<Tracked> have the same type regardless of deleter.
    std::shared_ptr<Tracked> p1(new Tracked("T1"), Deleter1());
    std::shared_ptr<Tracked> p2(new Tracked("T2"), Deleter2());

    // Q: When you assign p2 to p1, what happens to p1's original object and which deleter is invoked?
    // A: p1's original object is deleted by the custom deleted Deleter1() when its use_count goes to 0, then the control block is released in p1 and then a copy of std::shared_ptr for Tracked("T2") is created on the stack
    // A: Incrementing the count to 2
    // R: Correct sequence. Details: (1) p1's control block use_count drops to 0, (2) Deleter1::operator() is invoked on Tracked("T1"), (3) p1's control block is destroyed, (4) p1 is assigned p2's control block pointer, (5) p2's control block use_count increments from 1 to 2.
    // R: For junior SWEs: Assignment releases p1's old object (using Deleter1), then makes p1 share p2's object (incrementing count to 2).
    // Q: After assignment, p1 now points to p2's object. Which deleter does p1's control block store—Deleter1 or Deleter2?
    // A: Deleter2
    // R: Correct. After `p1 = p2`, p1 no longer has its own control block—it shares p2's control block, which contains Deleter2. Both p1 and p2 now point to the same control block with Deleter2 stored in it.
    // R: For junior SWEs: p1 and p2 now share the same control block, so they both use Deleter2.
    p1 = p2;

    auto events = EventLog::instance().events();
    bool deleter1_called = false;

    for (const auto& event : events)
    {
        if (event.find("Deleter1::operator()") != std::string::npos)
        {
            deleter1_called = true;
        }
    }

    EXPECT_TRUE(deleter1_called);
}

TEST_F(AllocationPatternsTest, AliasingWithCustomDeleter)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };

    struct ContainerDeleter
    {
        void operator()(Container* ptr) const
        {
            EventLog::instance().record("ContainerDeleter::operator()");
            delete ptr;
        }
    };

    long alias_count = 0;

    {
        std::shared_ptr<Container> owner(new Container("Owner"), ContainerDeleter());

        // Q: The alias points to `&owner->member` (a Tracked*), but shares the control block with owner. Which deleter is stored in the control block?
        // A: ContainerDeleter()
        // R: Correct. The aliasing constructor shares the control block (including the deleter) from owner. The control block stores ContainerDeleter, which operates on Container*, not Tracked*.
        // R: For junior SWEs: The alias borrows the control block from owner, so it uses owner's deleter (ContainerDeleter).
        std::shared_ptr<Tracked> alias(owner, &owner->member);
        alias_count = alias.use_count();

        // Q: After owner.reset(), only alias remains. When alias is destroyed, what gets deleted—the Tracked member or the Container?
        // A: Both.  First starts with Container that then deletes Tracked when ContainerDeleter is called when use_count hits 0
        // R: Correct. Key insight: The control block stores the ORIGINAL pointer (Container*), not the alias pointer (Tracked*). When use_count hits 0, ContainerDeleter::operator() receives the Container*, deletes the entire Container, which destroys the Tracked member inside it.
        // R: This is the magic of aliasing: alias points to the member, but the deleter operates on the owner. This prevents dangling pointers to subobjects.
        // R: For junior SWEs: Even though alias points to the member, the deleter deletes the whole Container, ensuring the member stays valid as long as alias exists.
        owner.reset();
    }

    auto events = EventLog::instance().events();
    bool container_deleter_called = false;

    for (const auto& event : events)
    {
        if (event.find("ContainerDeleter::operator()") != std::string::npos)
        {
            container_deleter_called = true;
        }
    }

    EXPECT_EQ(alias_count, 2);
    EXPECT_TRUE(container_deleter_called);
}

TEST_F(AllocationPatternsTest, SharedDeleterBetweenPointers)
{
    struct SharedDeleter
    {
        void operator()(Tracked* ptr) const
        {
            EventLog::instance().record("SharedDeleter::operator()");
            delete ptr;
        }
    };

    std::shared_ptr<Tracked> p1(new Tracked("Shared"), SharedDeleter());

    // Q: When you copy p1 to p2, do they share the same control block? What about the deleter stored in it?
    // A: They share the same control block.  The deleter stays the same
    // R: Correct. Copying a shared_ptr shares the control block (pointer copy + increment use_count). The deleter is stored once in the control block, so both p1 and p2 reference the same SharedDeleter instance.
    // R: For junior SWEs: Copying shared_ptr doesn't copy the control block or deleter—they all point to the same one.
    std::shared_ptr<Tracked> p2 = p1;
    long use_count = p1.use_count();

    // Q: After p1.reset(), the use_count drops to 1. Why isn't the deleter called yet?
    // A: Because use_count is still 1 at this point and deleter isn't called until use_count hits 0 to release the Tracked object
    // R: Correct. The deleter is invoked only when use_count transitions from 1 to 0. Since p2 still holds a reference, use_count is 1, and the object remains alive. The deleter will be called when p2 is destroyed (at scope exit).
    // R: For junior SWEs: The object stays alive as long as ANY shared_ptr references it (use_count > 0). Deleter runs when the LAST reference is released.
    p1.reset();

    auto events_after_reset = EventLog::instance().events();
    bool deleter_called_early = false;

    for (const auto& event : events_after_reset)
    {
        if (event.find("SharedDeleter::operator()") != std::string::npos)
        {
            deleter_called_early = true;
        }
    }

    EXPECT_EQ(use_count, 2);
    EXPECT_FALSE(deleter_called_early);
}

// ============================================================================
// ALLOCATION PATTERNS TEST SUITE - LEARNING ASSESSMENT
// ============================================================================
//
// ## Core Concepts Covered
//
// ### 1. Allocation Strategies
// - **make_shared vs new**: 1 allocation (combined) vs 2 allocations (separate)
// - **Performance**: make_shared is 10-50% faster (fewer allocator calls, better cache locality)
// - **Exception safety**: make_shared is atomic (no leak window), new has leak risk if control block allocation fails
// - **When to use new**: Custom deleters (make_shared doesn't support them), large objects with weak_ptrs (earlier memory release)
//
// ### 2. Custom Deleters
// - **Storage location**: Always stored in control block (not in shared_ptr object)
// - **Type erasure**: Deleter type is erased from shared_ptr's type signature via polymorphism
//   - shared_ptr<T> can hold any deleter type (unlike unique_ptr<T, Deleter>)
//   - Enables assignment between shared_ptrs with different deleters
//   - Enables storage in same container regardless of deleter type
// - **Invocation timing**: Deleter called when use_count hits 0, before control block destruction
// - **Deleter sharing**: Copied shared_ptrs share the same deleter instance in control block
//
// ### 3. Array Handling
// - **delete vs delete[]**: delete calls ONE destructor, delete[] calls ALL destructors (reads hidden array metadata)
// - **Array metadata**: Compiler stores array size before first element for delete[] to use
// - **Custom array deleters**: Required because default deleter uses delete (not delete[])
// - **Memory layout**: [array_size_metadata: N] [Element0] [Element1] ... [ElementN-1]
//
// ### 4. Deleter Type Erasure Mechanism
// - **Problem**: Different deleter types (Deleter1, Deleter2) need same shared_ptr<T> type
// - **Solution**: Control block uses polymorphic base class with virtual destroy_object()
// - **Implementation**: Template derived class stores specific deleter, base class pointer hides type
// - **Trade-off**: Flexibility (any deleter type) vs overhead (virtual call, heap allocation)
// - **Contrast with unique_ptr**: unique_ptr<T, Deleter> has deleter in type signature (no type erasure)
//
// ### 5. Aliasing with Custom Deleters
// - **Key insight**: Aliasing constructor shares control block (including deleter) from owner
// - **Deleter target**: Control block stores ORIGINAL pointer (owner), not alias pointer (member)
// - **Lifetime guarantee**: When alias is destroyed, deleter operates on owner, destroying entire object
// - **Purpose**: Prevents dangling pointers to subobjects by keeping owner alive
//
// ### 6. Deleter Lifecycle
// - **Sequence**: use_count → 0 → deleter invoked → object destroyed → control block destroyed (if weak_count == 0)
// - **Control block survival**: Must survive deleter call because deleter is stored inside it
// - **Shared deleter**: All copied shared_ptrs reference same deleter instance in shared control block
//
// ## Memory Model Understanding
//
// ### Stack vs Heap Layout
// ```
// Stack:
// ┌─────────────────────────────────┐
// │ shared_ptr<T> p1 (~16 bytes)    │
// │ ├─ ptr_to_object                │
// │ └─ ptr_to_control_block ────┐   │
// └─────────────────────────────┼───┘
//                               │
// Heap:                         │
// ┌─────────────────────────────▼───┐
// │ Control Block                   │
// │ ├─ use_count                    │
// │ ├─ weak_count                   │
// │ ├─ ptr_to_object (for deleter)  │
// │ └─ Deleter instance (inline)    │
// └─────────────────────────────────┘
//          │
//          ▼
// ┌─────────────────────────────────┐
// │ T object                        │
// └─────────────────────────────────┘
// ```
//
// ### Copying shared_ptr
// - Creates new shared_ptr object on stack (separate variable)
// - Copies both pointers (ptr_to_object, ptr_to_control_block)
// - Increments use_count in shared control block
// - Does NOT copy object, control block, or deleter
//
// ## Exception Safety
//
// ### make_shared Exception Safety
// - Single allocation is atomic: either both succeed or both fail
// - No leak window between object and control block allocation
// - Strong exception guarantee
//
// ### new Exception Safety Risk
// ```cpp
// shared_ptr<T> p(new T());  // DANGER: Two-step process
// // Step 1: new T() allocates object
// // Step 2: shared_ptr constructor allocates control block
// // If Step 2 throws, Step 1's allocation is orphaned → leak
// ```
//
// ## Type Erasure Deep Dive
//
// ### Why Type Erasure Matters
// ```cpp
// // Without type erasure (unique_ptr):
// unique_ptr<T, Deleter1> p1;
// unique_ptr<T, Deleter2> p2;
// p1 = p2;  // ERROR: Different types!
//
// // With type erasure (shared_ptr):
// shared_ptr<T> p1(new T(), Deleter1());
// shared_ptr<T> p2(new T(), Deleter2());
// p1 = p2;  // OK: Same type, deleter hidden in control block
// ```
//
// ### Implementation Mechanism
// ```cpp
// // Conceptual model (simplified)
// class ControlBlockBase {
//     virtual void destroy_object() = 0;  // Pure virtual
//     int use_count;
//     int weak_count;
//     void* ptr_to_object;
// };
//
// template<typename Deleter>
// class ControlBlockWithDeleter : public ControlBlockBase {
//     Deleter deleter_;
//     void destroy_object() override {
//         deleter_(static_cast<T*>(ptr_to_object));
//     }
// };
//
// class shared_ptr<T> {
//     T* ptr_;
//     ControlBlockBase* control_block_;  // No deleter type here!
// };
// ```
//
// ### Debugging Implications
// - Stack traces show concrete deleter type in control block frames
// - shared_ptr object itself doesn't reveal deleter type without inspecting control block
// - GDB: `p *p1._M_refcount._M_pi` shows control block type with deleter
//
// ## Cache Locality Impact
//
// ### make_shared (Better Locality)
// ```
// [Control Block][Object]  ← Adjacent in memory, likely same cache line
// ```
//
// ### new (Worse Locality)
// ```
// [Control Block] ... [Object]  ← Separate allocations, possibly far apart
// ```
// - Accessing both requires potentially two cache misses
// - Typical improvement: 10-50% in allocation-heavy code
//
// ## Pedagogical Assessment
//
// ### What This Test Suite Teaches
// 1. **Allocation trade-offs**: Performance vs flexibility (make_shared vs new)
// 2. **Deleter mechanics**: Storage, invocation, type erasure, sharing
// 3. **Memory model**: Stack/heap layout, control block structure, pointer relationships
// 4. **Exception safety**: Leak windows, atomic operations, strong guarantees
// 5. **Type system design**: Type erasure via polymorphism, trade-offs vs unique_ptr
// 6. **Array handling**: delete[] mechanics, metadata, custom deleters
// 7. **Aliasing subtleties**: Deleter target vs pointer target, lifetime extension
//
// ### Key Insights for Senior Engineers
// - **Control block is the invisible complexity**: Lifetime, deleter storage, type erasure all hidden here
// - **Type erasure enables flexibility**: Same type regardless of deleter, but costs virtual call overhead
// - **Aliasing is powerful but subtle**: Pointer target ≠ deleter target, enables subobject safety
// - **Exception safety matters**: make_shared's atomicity prevents rare but real leaks
// - **Cache locality is measurable**: Adjacent allocations improve performance in hot paths
//
// ### Common Misconceptions Addressed
// - ❌ "make_shared is always better" → No: custom deleters, large objects with weak_ptrs
// - ❌ "Deleter is stored in shared_ptr object" → No: stored in control block for sharing
// - ❌ "delete[] is just a loop" → No: uses hidden metadata, single operation
// - ❌ "Type erasure is magic" → No: standard polymorphism via virtual functions
// - ❌ "Aliasing changes deleter" → No: shares control block, preserves original deleter
//
// ### Real-World Applications
// - **C resource management**: FILE*, malloc/free, custom cleanup (sockets, handles)
// - **Array wrapping**: Dynamic arrays with proper cleanup (delete[])
// - **Subobject lifetime**: Aliasing to members while keeping container alive
// - **Polymorphic containers**: Store shared_ptrs with different deleters in same vector
// - **Exception-safe factories**: make_shared for atomic allocation
//
// ## Contrast with unique_ptr
//
// | Aspect | shared_ptr | unique_ptr |
// |--------|-----------|------------|
// | Deleter in type | No (type erasure) | Yes (template parameter) |
// | Assignment between deleters | ✓ Allowed | ✗ Different types |
// | Storage in containers | ✓ Same type | ✗ Need type-erased wrapper |
// | Overhead | Virtual call, control block | Zero (deleter inlined) |
// | Flexibility | High (any deleter) | Low (fixed at compile time) |
//
// ## Summary
//
// This test suite builds deep understanding of:
// - How allocation strategy affects performance, safety, and flexibility
// - Where and how custom deleters are stored and invoked
// - Type erasure mechanism enabling deleter flexibility
// - Memory layout and pointer relationships in shared_ptr
// - Exception safety guarantees and leak prevention
// - Array handling mechanics and metadata
// - Aliasing constructor's interaction with custom deleters
//
// These patterns are fundamental to resource management in modern C++ and appear
// frequently in production codebases dealing with C interop, legacy APIs, and
// complex ownership scenarios.
//
// ============================================================================
