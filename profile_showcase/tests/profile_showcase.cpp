/*
 * PROFILE SHOWCASE
 * ================
 * 
 * This file demonstrates the behavioral differences between all five Socratic learning profiles
 * in the AI teaching system. Each profile adapts its teaching style to match different skill levels:
 * 
 * - Junior (SWE I):       Beginner-friendly with term definitions and gentle guidance
 * - Intermediate (SWE II): Concept connections and reasoning-focused questions
 * - Senior (SWE III):      Precise technical depth with falsifiable claims
 * - Staff (SWE IV):        Adversarial questioning about failure modes (Default)
 * - Principal (SWE V):     Architecture-level with pathological edge cases
 * 
 * PURPOSE:
 * --------
 * Use this file to:
 * 1. See how the AI adapts its teaching style to your skill level
 * 2. Choose which profile best matches your learning needs
 * 3. Understand what to expect from each profile's question depth and response style
 * 
 * STRUCTURE:
 * ----------
 * All five TEST_F blocks use the SAME scenario (construct, copy, move) but ask DIFFERENT questions
 * and provide DIFFERENT response styles. This makes the profile differences immediately visible.
 * 
 * TO ACTIVATE A PROFILE:
 * ----------------------
 * In Cursor chat, state: "profile: junior" (or intermediate, senior, staff, principal)
 * See .cursor/rules/socratic-software-engineering.mdc for all configuration options.
 */

#include "move_instrumentation.h"
#include <gtest/gtest.h>
#include <utility>

class RuleOfFive
{
public:
    explicit RuleOfFive(const std::string& name)
    : tracked_(name)
    {
    }

    RuleOfFive(const RuleOfFive& other)
    : tracked_(other.tracked_)
    {
    }

    RuleOfFive(RuleOfFive&& other) noexcept
    : tracked_(std::move(other.tracked_))
    {
    }

    RuleOfFive& operator=(const RuleOfFive& other)
    {
        if (this != &other)
        {
            tracked_ = other.tracked_;
        }
        return *this;
    }

    RuleOfFive& operator=(RuleOfFive&& other) noexcept
    {
        if (this != &other)
        {
            tracked_ = std::move(other.tracked_);
        }
        return *this;
    }

    ~RuleOfFive()
    {
    }

    std::string name() const
    {
        return tracked_.name();
    }

private:
    MoveTracked tracked_;
};

class ProfileShowcaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(ProfileShowcaseTest, JuniorProfile)
{
    /*
     * JUNIOR PROFILE (SWE I)
     * - Beginner-friendly explanations with term definitions
     * - Proactive hints and scaffolding questions
     * - ALL code filled in (no TODOs left blank)
     * - Avoids multi-step reasoning leaps
     */

    RuleOfFive obj1("Object1");
    RuleOfFive obj2("Object2");
    RuleOfFive obj3("Object3");

    EventLog::instance().clear();

    RuleOfFive obj4 = obj1;
    RuleOfFive obj5 = std::move(obj2);

    // Q: What is a copy constructor? When is it called?
    // A:
    // R: Correct. A copy constructor creates a new object as a copy of an existing object.
    // R: It's called when you initialize a new object from an existing one, like `RuleOfFive obj4 = obj1`.
    // R: In this case, obj4's copy constructor copies the MoveTracked member from obj1, creating an independent duplicate.
    // R: Think of it like making a photocopy of a document—you have two separate documents with the same content.

    // Q: What happens to obj2 after you write `std::move(obj2)`?
    // A:
    // R: Good start. std::move doesn't actually move anything by itself—it just tells the compiler "it's okay to move from this object."
    // R: However, when the move constructor runs, obj2.tracked_ gets moved into obj5.tracked_, leaving obj2.tracked_ in a "moved-from" state.
    // R: Think of it like transferring money between bank accounts: the money moves from obj2's account to obj5's account, leaving obj2's account empty.
    // R: obj2 still exists as a variable, but its internal MoveTracked member is now empty (name_ is cleared, moved_from_ flag is set).

    // Q: Why do we need both copy and move operations in the same class?
    // A:
    // R: Not quite. Let's break it down step by step:
    // R: Copy is used when you want to keep the original object and create a duplicate. Both objects remain valid and independent.
    // R: Move is used when you don't need the original object anymore. It's more efficient because it "steals" the resources instead of duplicating them.
    // R: In RuleOfFive, copy means creating a new MoveTracked with the same name, while move means transferring ownership of the internal state.
    // R: Real-world analogy: Copying is like duplicating a file (original stays intact), moving is like renaming/relocating a file (original location is empty).

    auto events = EventLog::instance().events();
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
}

TEST_F(ProfileShowcaseTest, IntermediateProfile)
{
    /*
     * INTERMEDIATE PROFILE (SWE II)
     * - Simplified explanations connecting concepts explicitly
     * - Ladder hints after 2 attempts, one test at a time
     * - Gradual edge case introduction
     * - Questions require reasoning over memorization
     */

    RuleOfFive obj1("Object1");
    RuleOfFive obj2("Object2");
    RuleOfFive obj3("Object3");

    EventLog::instance().clear();

    RuleOfFive obj4 = obj1;
    RuleOfFive obj5 = std::move(obj2);

    // Q: How does the copy constructor affect the EventLog? What events get recorded?
    // A:
    // R: Correct. The copy constructor triggers a "copy_ctor" event in EventLog because RuleOfFive's copy constructor calls MoveTracked's copy constructor.
    // R: This demonstrates how copy semantics propagate through composition—when you copy the containing object (RuleOfFive), it automatically copies its members (MoveTracked).
    // R: The connection to understand: EventLog is instrumented at the MoveTracked level, so every RuleOfFive operation shows up as a MoveTracked operation in the log.

    // Q: What's the relationship between std::move() and the move constructor being called?
    // A:
    // R: Good reasoning about std::move casting to an rvalue reference. However, the move constructor invocation is more nuanced.
    // R: std::move(obj2) casts obj2 to RuleOfFive&& (rvalue reference), which makes the compiler prefer the move constructor over the copy constructor during overload resolution.
    // R: How does this relate to performance? The move constructor can "steal" resources instead of duplicating them, which is why it's more efficient than copying.
    // R: Think about the chain: std::move → rvalue cast → compiler selects move constructor → MoveTracked's state is transferred.

    // Q: After the move, obj2 is in a moved-from state. What operations are still safe to perform on it?
    // A:
    // R: Not quite. The C++ standard guarantees that moved-from objects are in a "valid but unspecified state."
    // R: The key is understanding what "valid" means: you can safely call obj2's destructor, and you can assign a new value to obj2.
    // R: However, trying to use obj2's current value (like reading obj2.name()) is unsafe because its internal MoveTracked has been moved-from.
    // R: Try reasoning about this: Why does the standard allow destructors but not value access? What invariant must remain even after a move?

    auto events = EventLog::instance().events();
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
}

TEST_F(ProfileShowcaseTest, SeniorProfile)
{
    /*
     * SENIOR PROFILE (SWE III)
     * - Precise technical depth, no hints
     * - Strict verification with falsifiable reasoning
     * - Probes aliasing, invariants, and subtle mechanics
     * - Challenges incomplete or unfalsifiable assumptions
     */

    RuleOfFive obj1("Object1");
    RuleOfFive obj2("Object2");
    RuleOfFive obj3("Object3");

    EventLog::instance().clear();

    RuleOfFive obj4 = obj1;
    RuleOfFive obj5 = std::move(obj2);

    // Q: What invariant must the copy constructor maintain regarding the MoveTracked member's state?
    // A:
    // R: Correct. The invariant is that the copied object (obj4.tracked_) must have the same logical state as the source (obj1.tracked_).
    // R: Specifically: obj4.tracked_.name() == obj1.tracked_.name() and both are non-moved-from (moved_from_ flag is false).
    // R: Observable signal: EventLog shows exactly one "copy_ctor" event, confirming the copy occurred at the MoveTracked level.
    // R: The mechanism preserves deep copy semantics—obj4.tracked_ is a distinct object from obj1.tracked_, not a shared reference.

    // Q: Walk through the move operation line-by-line. At what point does obj2.tracked_ become moved-from?
    // A:
    // R: Correct that it occurs during MoveTracked's move constructor execution. However, your reasoning about the timing assumes the compiler's implementation details.
    // R: What would falsify this claim? If obj2.tracked_.name() were still valid after the move, but EventLog shows "move_ctor" was called.
    // R: The precise moment: Inside MoveTracked::MoveTracked(MoveTracked&& other), when `name_ = std::move(other.name_)` executes and `other.moved_from_ = true` is set.
    // R: Before this line, obj2.tracked_ is valid. After this line, obj2.tracked_ is moved-from. The RuleOfFive move constructor simply delegates to this mechanism.

    // Q: What observable signal in EventLog confirms that no unnecessary copies occurred?
    // A:
    // R: This contradicts the observable behavior. EventLog shows exactly 1 "copy_ctor" and 1 "move_ctor" event, not zero copies.
    // R: The evidence: obj4 = obj1 triggers copy_ctor (necessary), obj5 = std::move(obj2) triggers move_ctor (not a copy).
    // R: What mechanism produces this signal? The instrumentation in MoveTracked logs each constructor call, allowing us to verify copy/move semantics at runtime.
    // R: The key insight: "No unnecessary copies" means the count matches expectations (1 copy, 1 move), not that copies don't exist.

    auto events = EventLog::instance().events();
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
}

TEST_F(ProfileShowcaseTest, StaffProfile)
{
    /*
     * STAFF PROFILE (SWE IV) - DEFAULT
     * - Mechanism-focused, adversarial questioning
     * - Explores invariants, failure modes, multi-object ownership
     * - Requires explicit reasoning about system consequences
     * - Emphasizes observable signals over intuition
     */

    RuleOfFive obj1("Object1");
    RuleOfFive obj2("Object2");
    RuleOfFive obj3("Object3");

    EventLog::instance().clear();

    RuleOfFive obj4 = obj1;
    RuleOfFive obj5 = std::move(obj2);

    // Q: What invariant breaks if the copy constructor throws after partially copying the MoveTracked member? Walk through the failure mode.
    // A:
    // R: Correct. The invariant is exception safety: either the object is fully constructed or it doesn't exist at all (strong guarantee).
    // R: Now consider: what happens if MoveTracked's copy constructor throws after allocating memory but before setting name_? The partially-constructed obj4 would have invalid state.
    // R: The failure mode cascade: (1) MoveTracked copy ctor throws, (2) RuleOfFive copy ctor propagates exception, (3) obj4 is never fully constructed, (4) no destructor runs for obj4.
    // R: What invariant prevents this? MoveTracked's copy constructor must be exception-safe—if it throws, no resources are leaked because nothing was allocated yet.
    // R: Observable signal: EventLog would show "copy_ctor" start but no corresponding destruction, indicating the failure point.

    // Q: Consider the ownership graph after `obj5 = std::move(obj2)`. Which objects hold valid MoveTracked instances, and what happens if obj2's destructor runs?
    // A:
    // R: You've identified the states correctly. However, the failure mode is more subtle: obj2's destructor will definitely run when it goes out of scope.
    // R: Ownership graph: obj1.tracked_ (valid), obj2.tracked_ (moved-from), obj3.tracked_ (valid), obj4.tracked_ (valid copy of obj1), obj5.tracked_ (valid, stolen from obj2).
    // R: What happens to obj2.tracked_'s destructor? It runs safely because MoveTracked's destructor handles moved-from state (empty name_, no resources to release).
    // R: The critical invariant: moved-from objects must remain destructible. If obj2.tracked_'s destructor tried to release non-existent resources, it would crash.
    // R: Observable signal: EventLog shows 5 construction events (3 initial + 1 copy + 1 move) and eventually 5 destructor calls when all objects go out of scope.

    // Q: What observable signal distinguishes a moved-from MoveTracked from a default-constructed one? Why does this distinction matter for lifetime correctness?
    // A:
    // R: This reasoning doesn't account for the failure mode when code tries to use the object's value.
    // R: Walk through the signals: (1) Moved-from MoveTracked has is_moved_from() == true, default-constructed has is_moved_from() == false. (2) Moved-from has name() == "", default may have name() == "".
    // R: The distinction matters because moved-from state indicates "this object was valid but got pillaged"—you know it had a value that was stolen.
    // R: Default-constructed means "this object was never given a meaningful value"—it's empty by design, not because something was taken.
    // R: What observable signal confirms this? EventLog shows the history: moved-from objects have a "move_ctor" in their past, default-constructed objects have only "::ctor" with default args.
    // R: Why lifetime correctness? If you accidentally use a moved-from object thinking it's default-constructed, you violate the class's contract. The flag prevents this confusion.

    auto events = EventLog::instance().events();
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
}

TEST_F(ProfileShowcaseTest, PrincipalProfile)
{
    /*
     * PRINCIPAL PROFILE (SWE V)
     * - Architecture-level reasoning about ownership and lifetime
     * - Pathological edge cases, aliasing traps, systemic consequences
     * - Requires justification with observable signals
     * - Deep mechanism-level investigation of every scenario
     */

    RuleOfFive obj1("Object1");
    RuleOfFive obj2("Object2");
    RuleOfFive obj3("Object3");

    EventLog::instance().clear();

    RuleOfFive obj4 = obj1;
    RuleOfFive obj5 = std::move(obj2);

    // Q: What implicit contract does noexcept on the move constructor establish between RuleOfFive and std::vector's reallocation strategy? What pathological scenario violates this contract?
    // A:
    // R: Correct. The contract noexcept establishes is: "std::vector can safely use move semantics during reallocation without exception safety concerns."
    // R: In pathological case where move throws: std::vector is reallocating from old buffer to new buffer. If the 5th element's move constructor throws, elements 1-4 are in the new buffer, 5-10 are in the old buffer. Strong exception guarantee is violated—you can't restore the original state.
    // R: Observable signals if contract is violated: (1) std::is_nothrow_move_constructible<RuleOfFive>::value would be false, (2) std::vector would fall back to copy constructor (EventLog would show copy_ctor instead of move_ctor during reallocation), (3) performance degrades from O(n) moves to O(n) copies.
    // R: The systemic consequence: Without noexcept, every std::vector operation becomes pessimistically safe, copying instead of moving. This cascades through the entire codebase—any container holding RuleOfFive pays the copy penalty.
    // R: Architecture-level invariant: Move constructors that manage resources must either be noexcept OR must restore source object state on throw. RuleOfFive delegates to MoveTracked, which is noexcept, preserving this contract.

    // Q: If MoveTracked's move constructor threw an exception mid-operation, how would the RuleOfFive's move constructor restore exception safety? What systemic lifetime hazards emerge if it fails?
    // A:
    // R: Surface-level correct. Architecture-level: the hazard is more insidious than just "exception propagates."
    // R: Consider the pathological scenario: RuleOfFive move constructor calls `tracked_(std::move(other.tracked_))`. If this throws after MoveTracked has transferred name_ but before setting moved_from_ flag, what state is other.tracked_ in?
    // R: Systemic lifetime hazards: (1) other.tracked_ has invalid state (name_ is moved but flag is unset), (2) other's destructor still runs because other exists on the stack, (3) destructor operates on inconsistent state (thinks it owns resources that were stolen), (4) double-free or use-after-free depending on resource type.
    // R: What happens when std::vector holds 10,000 RuleOfFive objects and the 5,000th move throws? First 4,999 are moved (sources are zombies), element 5,000 is partially moved (source is corrupted), elements 5,001-10,000 are untouched. Cleanup is impossible without explicit exception handling in the move constructor.
    // R: Observable signals that would expose this: (1) EventLog shows "move_ctor" start but no completion, (2) Destructor runs on corrupted object (moved_from_ == false but name_.empty() == true), (3) Sanitizers would detect use-after-move if the contract is violated.
    // R: The implicit contract RuleOfFive can't restore on throw: it has no way to "unmove" tracked_ once std::move() succeeds. This is why move constructors MUST be noexcept for types managing resources.

    // Q: Consider aliasing: if RuleOfFive stored `MoveTracked* tracked_` instead of `MoveTracked tracked_`, what architecture-level ownership invariant breaks during move assignment, and what observable signals would expose dangling pointers?
    // A:
    // R: This overlooks the systemic hazard of pointer-based ownership with move semantics.
    // R: Pathological case with `MoveTracked* tracked_`: Move assignment `obj5 = std::move(obj2)` would transfer the pointer: `obj5.tracked_ = obj2.tracked_; obj2.tracked_ = nullptr;`.
    // R: What implicit contract breaks? Lifetime coupling: obj2's destructor runs `delete tracked_`, but tracked_ is nullptr (safe). obj5's destructor also runs `delete tracked_`, deleting the actual object. So far, correct.
    // R: But consider the failure mode: What if obj5 already pointed to a MoveTracked before the move assignment? The old obj5.tracked_ is leaked because we overwrote the pointer without deleting it first.
    // R: Walk through the lifetime graph: (1) obj5 initially holds MoveTracked("Object5"), tracked_ points to heap allocation A. (2) Move assignment: obj5.tracked_ = obj2.tracked_ (now points to allocation B), allocation A is leaked. (3) obj2.tracked_ = nullptr. (4) Both destructors run: obj5 deletes B (correct), obj2 deletes nullptr (safe), but A is never deleted.
    // R: Observable signals exposing this: (1) EventLog shows "::ctor" for Object5 but no corresponding "::dtor" (allocation A leaked), (2) Valgrind reports "definitely lost" memory, (3) ASAN detects the leak at program exit, (4) use_count would be wrong if using shared_ptr semantics.
    // R: Architecture-level invariant violated: RAII requires destructor to release resources. With raw pointer members, move assignment must explicitly `delete tracked_` before overwriting. By-value members (MoveTracked tracked_) handle this automatically via compiler-generated destructors—this is why composition is safer than pointer ownership.

    auto events = EventLog::instance().events();
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
}




