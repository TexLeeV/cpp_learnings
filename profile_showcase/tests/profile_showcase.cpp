/*
 * PROFILE SHOWCASE
 * ================
 *
 * This file demonstrates how all five Socratic learning profiles behave in the same
 * construct-copy-move scenario:
 * - Junior (SWE I): beginner-friendly with term definitions and gentle guidance
 * - Intermediate (SWE II): concept connections and reasoning-focused prompts
 * - Senior (SWE III): precise technical depth with falsifiable reasoning
 * - Staff (SWE IV): adversarial questioning about failure modes (default)
 * - Principal (SWE V): architecture-level analysis with pathological edge cases
 *
 * Purpose:
 * 1. Show how teaching style adapts to skill level.
 * 2. Help choose the best profile for your learning needs.
 * 3. Set expectations for question depth and response style.
 *
 * Activate a profile in Cursor chat:
 *   "profile: junior" (or intermediate, senior, staff, principal)
 *
 * See `.cursor/rules/socratic-software-engineering.mdc` for full configuration.
 */

#include "move_instrumentation.h"

#include <gtest/gtest.h>
#include <utility>

class RuleOfFive
{
public:
    explicit RuleOfFive(const std::string& name) : tracked_(name)
    {
    }

    RuleOfFive(const RuleOfFive& other) : tracked_(other.tracked_)
    {
    }

    RuleOfFive(RuleOfFive&& other) noexcept : tracked_(std::move(other.tracked_))
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
    // R: In this case, obj4's copy constructor copies the MoveTracked member from obj1, creating an
    // R: independent duplicate.
    // R: Think of it like making a photocopy of a document: you get two separate documents with the
    // R: same content.

    // Q: What happens to obj2 after you write `std::move(obj2)`?
    // A:
    // R: Good start. std::move does not move anything by itself; it tells the compiler it is okay to
    // R: move from this object.
    // R: When the move constructor runs, obj2.tracked_ is moved into obj5.tracked_, leaving
    // R: obj2.tracked_ in a moved-from state.
    // R: Think of it like transferring money between bank accounts: funds move from obj2 to obj5.
    // R: obj2 still exists, but its MoveTracked member is now empty (name_ cleared, moved_from_ set).

    // Q: Why do we need both copy and move operations in the same class?
    // A:
    // R: Not quite. Let's break it down step by step:
    // R: Copy is for keeping the original object while creating a duplicate; both objects stay valid
    // R: and independent.
    // R: Move is for when you no longer need the original value; it can transfer resources instead of
    // R: duplicating them.
    // R: In RuleOfFive, copy creates a new MoveTracked with the same name, while move transfers the
    // R: internal state.
    // R: Analogy: copying duplicates a file, while moving relocates it and leaves the original location
    // R: empty.

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
    // R: Correct. The copy constructor triggers a "copy_ctor" event in EventLog because RuleOfFive's
    // R: copy constructor calls MoveTracked's copy constructor.
    // R: This shows how copy semantics propagate through composition: copying RuleOfFive copies its
    // R: members (MoveTracked).
    // R: EventLog is instrumented at the MoveTracked level, so every RuleOfFive operation appears as a
    // R: MoveTracked operation in the log.

    // Q: What's the relationship between std::move() and the move constructor being called?
    // A:
    // R: Good reasoning about std::move casting to an rvalue reference. The move constructor call is
    // R: slightly more nuanced.
    // R: std::move(obj2) casts obj2 to RuleOfFive&&, so overload resolution prefers the move
    // R: constructor over the copy constructor.
    // R: This improves performance because move can transfer resources instead of duplicating them.
    // R: Chain to remember: std::move -> rvalue cast -> move constructor selected -> MoveTracked state
    // R: transferred.

    // Q: After the move, obj2 is in a moved-from state. What operations are still safe to perform on it?
    // A:
    // R: Not quite. The C++ standard guarantees that moved-from objects are in a "valid but unspecified state."
    // R: The key is understanding what "valid" means: you can safely destroy obj2 and assign a new
    // R: value to obj2.
    // R: Using obj2's current value (for example, reading obj2.name()) is unsafe because its internal
    // R: MoveTracked has been moved-from.
    // R: Reason through this: why does the standard allow destruction but not reliable value access?
    // R: What condition must remain true even after a move?

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
     * - Probes aliasing, guarantees, and subtle mechanics
     * - Challenges incomplete or unfalsifiable assumptions
     */

    RuleOfFive obj1("Object1");
    RuleOfFive obj2("Object2");
    RuleOfFive obj3("Object3");

    EventLog::instance().clear();

    RuleOfFive obj4 = obj1;
    RuleOfFive obj5 = std::move(obj2);

    // Q: What guarantee must the copy constructor maintain regarding the MoveTracked member's state?
    // A:
    // R: Correct. The guarantee is that the copied object (obj4.tracked_) must match the source
    // R: (obj1.tracked_) in logical state.
    // R: Specifically, obj4.tracked_.name() == obj1.tracked_.name() and both remain non-moved-from
    // R: (moved_from_ is false).
    // R: Observable signal: EventLog shows exactly one "copy_ctor" event, confirming the copy at the
    // R: MoveTracked level.
    // R: The mechanism preserves deep-copy semantics: obj4.tracked_ is distinct from obj1.tracked_,
    // R: not a shared reference.

    // Q: Walk through the move operation line-by-line. At what point does obj2.tracked_ become moved-from?
    // A:
    // R: Correct that it occurs during MoveTracked's move-constructor execution. Your timing argument,
    // R: however, assumes compiler details.
    // R: What would falsify the claim? If obj2.tracked_.name() stayed valid after the move while
    // R: EventLog still recorded "move_ctor".
    // R: The transition occurs inside MoveTracked::MoveTracked(MoveTracked&& other), when
    // R: `name_ = std::move(other.name_)` runs and `other.moved_from_ = true` is set.
    // R: Before that point, obj2.tracked_ is valid; after it, obj2.tracked_ is moved-from.
    // R: RuleOfFive simply delegates to that mechanism.

    // Q: What observable signal in EventLog confirms that no unnecessary copies occurred?
    // A:
    // R: This contradicts observable behavior. EventLog shows exactly one "copy_ctor" and one
    // R: "move_ctor", not zero copies.
    // R: Evidence: obj4 = obj1 triggers copy_ctor (required), and obj5 = std::move(obj2) triggers
    // R: move_ctor (not a copy).
    // R: The signal comes from MoveTracked instrumentation, which logs each constructor call and lets
    // R: us verify copy/move semantics at runtime.
    // R: Key point: "No unnecessary copies" means counts match expectations (1 copy, 1 move), not
    // R: that copies never occur.

    auto events = EventLog::instance().events();
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
}

TEST_F(ProfileShowcaseTest, StaffProfile)
{
    /*
     * STAFF PROFILE (SWE IV) - DEFAULT
     * - Mechanism-focused, adversarial questioning
     * - Explores guarantees, failure modes, multi-object ownership
     * - Requires explicit reasoning about system consequences
     * - Emphasizes observable signals over intuition
     */

    RuleOfFive obj1("Object1");
    RuleOfFive obj2("Object2");
    RuleOfFive obj3("Object3");

    EventLog::instance().clear();

    RuleOfFive obj4 = obj1;
    RuleOfFive obj5 = std::move(obj2);

    // Q: What guarantee breaks if the copy constructor throws after partially copying the MoveTracked
    // Q: member? Walk through the failure mode.
    // A:
    // R: Correct. The guarantee is exception safety: either construction completes or the object does
    // R: not exist (strong guarantee).
    // R: Consider MoveTracked's copy constructor throwing after allocation but before setting name_.
    // R: Then obj4 is only partially constructed.
    // R: Failure cascade: (1) MoveTracked copy constructor throws, (2) RuleOfFive copy constructor
    // R: propagates, (3) obj4 is never fully constructed, (4) obj4's destructor does not run.
    // R: What requirement prevents leaks? MoveTracked copy construction must be exception-safe so a
    // R: throw does not leak resources.
    // R: Observable signal: EventLog would show copy-construction started but no matching completion or
    // R: destruction at that point.

    // Q: Consider the ownership graph after `obj5 = std::move(obj2)`. Which objects hold valid MoveTracked instances,
    // and what happens if obj2's destructor runs?
    // A:
    // R: You identified the states correctly. The subtle point is that obj2's destructor still runs at
    // R: scope exit.
    // R: Ownership graph: obj1.tracked_ (valid), obj2.tracked_ (moved-from), obj3.tracked_ (valid),
    // R: obj4.tracked_ (valid copy of obj1), obj5.tracked_ (valid, moved from obj2).
    // R: obj2.tracked_'s destructor is safe because MoveTracked handles moved-from state (empty name_,
    // R: no owned resources to release).
    // R: Critical requirement: moved-from objects must remain destructible. If destruction attempted to
    // R: free non-existent resources, it would crash.
    // R: Observable signal: EventLog shows five construction events (3 initial + 1 copy + 1 move) and
    // R: eventually five destructor calls.

    // Q: What observable signal distinguishes a moved-from MoveTracked from a default-constructed one? Why does this
    // distinction matter for lifetime correctness?
    // A:
    // R: This misses the failure mode that appears when code tries to use the current value.
    // R: Signals: (1) moved-from MoveTracked has is_moved_from() == true, while default-constructed
    // R: has is_moved_from() == false; (2) both may have name() == "".
    // R: The distinction matters because moved-from means "value was previously transferred," while
    // R: default-constructed means "value was never set."
    // R: Observable signal: EventLog history differs. Moved-from objects have "move_ctor" in their
    // R: history; default-constructed objects only show default "::ctor".
    // R: Lifetime impact: treating moved-from as default-constructed can violate usage expectations.
    // R: The moved_from_ flag helps avoid that confusion.

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

    // Q: What implicit contract does noexcept on the move constructor establish between RuleOfFive and std::vector's
    // reallocation strategy? What pathological scenario violates this contract?
    // A:
    // R: Correct. The noexcept contract is: std::vector can move elements during reallocation without
    // R: threatening exception safety.
    // R: Pathological case: vector reallocates and the 5th element move throws; elements 1-4 are in the
    // R: new buffer, 5-10 remain in the old buffer. Strong exception safety is no longer recoverable.
    // R: Observable signals if the contract fails:
    // R: (1) std::is_nothrow_move_constructible<RuleOfFive>::value is false,
    // R: (2) std::vector falls back to copy construction (EventLog shows copy_ctor during reallocation),
    // R: (3) performance drops from O(n) moves to O(n) copies.
    // R: Systemic effect: container operations become pessimistically copy-heavy across the codebase.
    // R: Architecture-level requirement: resource-managing move constructors should be noexcept or
    // R: support robust rollback on throw.
    // R: RuleOfFive delegates to MoveTracked, which is noexcept, preserving this contract.

    // Q: If MoveTracked's move constructor threw an exception mid-operation, how would the RuleOfFive's move
    // constructor restore exception safety? What systemic lifetime hazards emerge if it fails?
    // A:
    // R: Surface-level correct. Architecture-level hazard is more subtle than "exception propagates."
    // R: Pathological case: RuleOfFive move construction calls `tracked_(std::move(other.tracked_))`.
    // R: If this throws after transferring name_ but before setting moved_from_, other.tracked_ becomes
    // R: internally inconsistent.
    // R: Systemic lifetime hazards:
    // R: (1) partially moved source state,
    // R: (2) source destructor still runs,
    // R: (3) cleanup logic may run on inconsistent ownership data,
    // R: (4) double-free or use-after-free depending on resource model.
    // R: At scale (for example, vector of 10,000 elements with failure at 5,000), rollback becomes
    // R: practically impossible without explicit exception strategy in move construction.
    // R: Observable signals:
    // R: (1) EventLog shows move-construction start without completion,
    // R: (2) destructor sees contradictory state (moved_from_ false while name_.empty() true),
    // R: (3) sanitizers report use-after-move or lifetime misuse.
    // R: RuleOfFive cannot "unmove" tracked_ once transfer starts, so resource-managing move
    // R: constructors should be noexcept.

    // Q: Consider aliasing: if RuleOfFive stored `MoveTracked* tracked_` instead of `MoveTracked tracked_`, what
    // architecture-level ownership guarantee breaks during move assignment, and what observable signals would expose
    // dangling pointers?
    // A:
    // R: This overlooks the systemic hazard of pointer-based ownership with move semantics.
    // Pathological case with `MoveTracked* tracked_`: Move assignment `obj5 = std::move(obj2)` would transfer the
    // pointer: `obj5.tracked_ = obj2.tracked_; obj2.tracked_ = nullptr;`.
    // R: Which implicit contract breaks? Lifetime coupling under reassignment.
    // R: If obj5 already owns a pointer before move-assignment, overwriting it without deleting leaks the
    // R: old allocation.
    // R: Lifetime graph:
    // R: (1) obj5 owns allocation A,
    // R: (2) move-assignment redirects obj5 to allocation B from obj2,
    // R: (3) obj2 becomes nullptr,
    // R: (4) destructors delete B and nullptr, but A is leaked.
    // R: Observable signals:
    // R: (1) ctor without matching dtor in EventLog for the leaked allocation,
    // R: (2) Valgrind reports "definitely lost" memory,
    // R: (3) AddressSanitizer reports leaks at process exit.
    // R: Architecture-level requirement from RAII (Resource Acquisition Is Initialization):
    // R: move-assignment with raw pointers must release current ownership before overwrite.
    // R: By-value members (`MoveTracked tracked_`) avoid this class of bug automatically through normal
    // R: destruction.

    auto events = EventLog::instance().events();
    EXPECT_EQ(EventLog::instance().count_events("copy_ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("move_ctor"), 1);
}
