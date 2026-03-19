// Test Suite: Exception Safety Guarantees (Basic, Strong, No-Throw)
// Estimated Time: 3 hours
// Difficulty: Moderate
// C++ Standard: C++17

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <algorithm>

class ExceptionSafetyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Exception Safety Guarantees Overview
// ============================================================================

// 1. No-throw guarantee: Operation never throws (noexcept)
// 2. Strong guarantee: Operation succeeds or has no effect (commit-or-rollback)
// 3. Basic guarantee: Operation may modify state but no resources leak
// 4. No guarantee: Undefined behavior on exception

// Q: Which guarantee is strongest? Which is weakest?
// A:
// R:

// Q: Can a function provide strong guarantee if it calls functions with only basic guarantee?
// A:
// R:

// ============================================================================
// Basic Exception Safety - No Leaks, But State May Change
// ============================================================================

class BasicSafetyContainer
{
public:
    BasicSafetyContainer()
    {
        EventLog::instance().record("BasicSafetyContainer::ctor");
    }
    
    ~BasicSafetyContainer()
    {
        EventLog::instance().record("BasicSafetyContainer::dtor");
    }
    
    void add_item(std::unique_ptr<Tracked> item, bool throw_after_add)
    {
        EventLog::instance().record("add_item: start");
        
        items_.push_back(std::move(item));
        EventLog::instance().record("add_item: item added");
        
        if (throw_after_add)
        {
            EventLog::instance().record("add_item: throwing exception");
            throw std::runtime_error("Simulated error");
        }
        
        EventLog::instance().record("add_item: success");
    }
    
    size_t size() const { return items_.size(); }

private:
    std::vector<std::unique_ptr<Tracked>> items_;
};

// Q: If add_item throws, is the item added to the container?
// A:
// R:

// Q: Are any resources leaked when add_item throws?
// A:
// R:

TEST_F(ExceptionSafetyTest, BasicSafety_NoLeaks)
{
    // Easy: Basic guarantee ensures no resource leaks
    
    BasicSafetyContainer container;
    
    try
    {
        container.add_item(std::make_unique<Tracked>("Item1"), false);
        EXPECT_EQ(container.size(), 1);
        
        container.add_item(std::make_unique<Tracked>("Item2"), true);
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Container state changed (Item2 was added before exception)
    EXPECT_EQ(container.size(), 2);
    
    // Verify no leaks - both items constructed and will be destroyed with container
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Item1)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Item2)::ctor"), 1);
}

// ============================================================================
// Strong Exception Safety - Commit or Rollback
// ============================================================================

class StrongSafetyContainer
{
public:
    StrongSafetyContainer()
    {
        EventLog::instance().record("StrongSafetyContainer::ctor");
    }
    
    ~StrongSafetyContainer()
    {
        EventLog::instance().record("StrongSafetyContainer::dtor");
    }
    
    void add_item(std::unique_ptr<Tracked> item, bool throw_after_add)
    {
        EventLog::instance().record("add_item: start");
        
        // TODO: Implement strong guarantee using copy-and-swap or similar technique
        // Hint: Modify a temporary, then swap only if successful
        
        std::vector<std::unique_ptr<Tracked>> temp_items;
        for (auto& existing : items_)
        {
            // Can't copy unique_ptr, so this is a simplified example
            // In real code, you'd use a different strategy
        }
        
        if (throw_after_add)
        {
            EventLog::instance().record("add_item: throwing before commit");
            throw std::runtime_error("Simulated error");
        }
        
        items_.push_back(std::move(item));
        EventLog::instance().record("add_item: success");
    }
    
    size_t size() const { return items_.size(); }

private:
    std::vector<std::unique_ptr<Tracked>> items_;
};

// Q: What is the key difference between basic and strong exception safety?
// A:
// R:

// Q: What technique enables strong exception safety?
// A:
// R:

TEST_F(ExceptionSafetyTest, StrongSafety_NoStateChange)
{
    // Moderate: Strong guarantee ensures either success or no observable change
    
    StrongSafetyContainer container;
    
    try
    {
        container.add_item(std::make_unique<Tracked>("Item1"), false);
        EXPECT_EQ(container.size(), 1);
        
        container.add_item(std::make_unique<Tracked>("Item2"), true);
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Container state unchanged (Item2 was NOT added)
    EXPECT_EQ(container.size(), 1);
    
    // Verify Item2 was constructed but not added to container
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Item2)::ctor"), 1);
}

// ============================================================================
// Copy-and-Swap Idiom - Strong Exception Safety
// ============================================================================

class SwapSafeContainer
{
public:
    SwapSafeContainer()
    {
        EventLog::instance().record("SwapSafeContainer::ctor");
    }
    
    void add_items(std::vector<std::string> names, bool throw_on_last)
    {
        EventLog::instance().record("add_items: start");
        
        // Create temporary vector
        std::vector<std::unique_ptr<Tracked>> temp;
        
        // Copy existing items (simplified - in real code would need proper copying)
        EventLog::instance().record("add_items: copying existing items");
        
        // Add new items to temporary
        for (size_t i = 0; i < names.size(); ++i)
        {
            if (throw_on_last && i == names.size() - 1)
            {
                EventLog::instance().record("add_items: throwing before commit");
                throw std::runtime_error("Simulated error");
            }
            
            temp.push_back(std::make_unique<Tracked>(names[i]));
            EventLog::instance().record("add_items: added " + names[i] + " to temp");
        }
        
        // Commit: swap only if all operations succeeded
        EventLog::instance().record("add_items: committing swap");
        items_.swap(temp);
        EventLog::instance().record("add_items: success");
    }
    
    size_t size() const { return items_.size(); }

private:
    std::vector<std::unique_ptr<Tracked>> items_;
};

// Q: Why does swap provide strong exception safety?
// A:
// R:

// Q: What happens to the temporary vector when the swap completes?
// A:
// R:

TEST_F(ExceptionSafetyTest, CopyAndSwap_StrongGuarantee)
{
    // Hard: Copy-and-swap idiom provides strong exception safety
    
    SwapSafeContainer container;
    
    // Successful operation
    container.add_items({"Item1", "Item2"}, false);
    EXPECT_EQ(container.size(), 2);
    
    EventLog::instance().clear();
    
    // Failed operation - should not modify container
    try
    {
        container.add_items({"Item3", "Item4"}, true);
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Container state unchanged
    EXPECT_EQ(container.size(), 2);
    
    // Verify Item3 was created in temp but never committed
    EXPECT_GE(EventLog::instance().count_events("Tracked(Item3)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("add_items: committing swap"), 0);
}

// ============================================================================
// No-Throw Guarantee - Operations That Never Fail
// ============================================================================

class NoThrowSwap
{
public:
    explicit NoThrowSwap(int value)
    : value_(value)
    {
        EventLog::instance().record("NoThrowSwap::ctor");
    }
    
    void swap(NoThrowSwap& other) noexcept
    {
        EventLog::instance().record("NoThrowSwap::swap");
        std::swap(value_, other.value_);
    }
    
    int value() const { return value_; }

private:
    int value_;
};

// Q: Why can swap provide a no-throw guarantee?
// A:
// R:

// Q: What operations typically provide no-throw guarantees?
// A:
// R:

TEST_F(ExceptionSafetyTest, NoThrow_SwapGuarantee)
{
    // Easy: Swap operations typically provide no-throw guarantee
    
    NoThrowSwap a(10);
    NoThrowSwap b(20);
    
    // Verify swap is noexcept
    static_assert(noexcept(a.swap(b)), "swap should be noexcept");
    
    a.swap(b);
    
    EXPECT_EQ(a.value(), 20);
    EXPECT_EQ(b.value(), 10);
    EXPECT_EQ(EventLog::instance().count_events("NoThrowSwap::swap"), 1);
}

// ============================================================================
// Exception Safety in Constructors
// ============================================================================

class PartiallyConstructedObject
{
public:
    PartiallyConstructedObject(bool throw_after_first)
    : first_(std::make_unique<Tracked>("First"))
    , second_(nullptr)
    , third_(nullptr)
    {
        EventLog::instance().record("PartiallyConstructedObject::ctor - first initialized");
        
        if (throw_after_first)
        {
            EventLog::instance().record("PartiallyConstructedObject::ctor - throwing");
            throw std::runtime_error("Construction failed");
        }
        
        second_ = std::make_unique<Tracked>("Second");
        EventLog::instance().record("PartiallyConstructedObject::ctor - second initialized");
        
        third_ = std::make_unique<Tracked>("Third");
        EventLog::instance().record("PartiallyConstructedObject::ctor - complete");
    }
    
    ~PartiallyConstructedObject()
    {
        EventLog::instance().record("PartiallyConstructedObject::dtor");
    }

private:
    std::unique_ptr<Tracked> first_;
    std::unique_ptr<Tracked> second_;
    std::unique_ptr<Tracked> third_;
};

// Q: What exception safety guarantee does this constructor provide?
// A:
// R:

// Q: Which members are cleaned up if the constructor throws?
// A:
// R:

TEST_F(ExceptionSafetyTest, Constructor_BasicSafety)
{
    // Moderate: Constructors provide basic safety - initialized members are cleaned up
    
    try
    {
        PartiallyConstructedObject obj(true);
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Verify first_ was initialized and cleaned up
    EXPECT_EQ(EventLog::instance().count_events("Tracked(First)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(First)::dtor"), 1);
    
    // Verify second_ and third_ were never initialized
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Second)::ctor"), 0);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Third)::ctor"), 0);
    
    // Verify destructor was NOT called (object never fully constructed)
    EXPECT_EQ(EventLog::instance().count_events("PartiallyConstructedObject::dtor"), 0);
}

// ============================================================================
// Two-Phase Initialization for Strong Safety
// ============================================================================

class TwoPhaseInit
{
public:
    TwoPhaseInit()
    {
        EventLog::instance().record("TwoPhaseInit::ctor");
    }
    
    ~TwoPhaseInit()
    {
        EventLog::instance().record("TwoPhaseInit::dtor");
    }
    
    void initialize(bool throw_during_init)
    {
        EventLog::instance().record("initialize: start");
        
        // Prepare resources in temporary storage
        auto temp1 = std::make_unique<Tracked>("Temp1");
        auto temp2 = std::make_unique<Tracked>("Temp2");
        
        EventLog::instance().record("initialize: temps created");
        
        if (throw_during_init)
        {
            EventLog::instance().record("initialize: throwing before commit");
            throw std::runtime_error("Initialization failed");
        }
        
        // Commit: move to members only if successful
        first_ = std::move(temp1);
        second_ = std::move(temp2);
        EventLog::instance().record("initialize: committed");
    }
    
    bool is_initialized() const { return first_ != nullptr; }

private:
    std::unique_ptr<Tracked> first_;
    std::unique_ptr<Tracked> second_;
};

// Q: What exception safety guarantee does initialize() provide?
// A:
// R:

// Q: What happens to temp1 and temp2 if the exception is thrown?
// A:
// R:

TEST_F(ExceptionSafetyTest, TwoPhaseInit_StrongSafety)
{
    // Hard: Two-phase initialization can provide strong guarantee
    
    TwoPhaseInit obj;
    EXPECT_FALSE(obj.is_initialized());
    
    // Successful initialization
    obj.initialize(false);
    EXPECT_TRUE(obj.is_initialized());
    EXPECT_EQ(EventLog::instance().count_events("initialize: committed"), 1);
    
    EventLog::instance().clear();
    
    // Failed initialization on new object
    TwoPhaseInit obj2;
    try
    {
        obj2.initialize(true);
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Object remains uninitialized (strong guarantee)
    EXPECT_FALSE(obj2.is_initialized());
    
    // Verify temps were created and destroyed (no leak)
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Temp1)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Temp2)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Temp1)::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Temp2)::dtor"), 1);
    
    // Verify commit never happened
    EXPECT_EQ(EventLog::instance().count_events("initialize: committed"), 0);
}

// ============================================================================
// std::vector Strong Exception Safety
// ============================================================================

// Q: What exception safety guarantee does std::vector::push_back provide?
// A:
// R:

// Q: How does std::vector achieve strong guarantee during reallocation?
// A:
// R:

TEST_F(ExceptionSafetyTest, Vector_StrongGuarantee)
{
    // Moderate: std::vector provides strong exception safety
    
    std::vector<std::unique_ptr<Tracked>> vec;
    
    vec.push_back(std::make_unique<Tracked>("V1"));
    vec.push_back(std::make_unique<Tracked>("V2"));
    
    size_t original_size = vec.size();
    EXPECT_EQ(original_size, 2);
    
    // If push_back throws (e.g., allocation failure), vector is unchanged
    // We can't easily simulate allocation failure, but the guarantee exists
    
    // Q: What happens to the vector's contents if push_back throws during reallocation?
    // A:
    // R:
}

// ============================================================================
// Exception Safety in Assignment Operators
// ============================================================================

class AssignmentSafety
{
public:
    AssignmentSafety()
    : data_(nullptr)
    {
        EventLog::instance().record("AssignmentSafety::ctor");
    }
    
    explicit AssignmentSafety(const std::string& name)
    : data_(std::make_unique<Tracked>(name))
    {
        EventLog::instance().record("AssignmentSafety::ctor(name)");
    }
    
    ~AssignmentSafety()
    {
        EventLog::instance().record("AssignmentSafety::dtor");
    }
    
    // Copy assignment - basic safety (may leak on exception)
    AssignmentSafety& operator=(const AssignmentSafety& other)
    {
        EventLog::instance().record("operator=: copy assignment start");
        
        if (this != &other)
        {
            // Dangerous: delete first, then allocate
            data_.reset();
            EventLog::instance().record("operator=: old data deleted");
            
            if (other.data_)
            {
                // If this throws, we've already deleted our data!
                data_ = std::make_unique<Tracked>(other.data_->name());
                EventLog::instance().record("operator=: new data created");
            }
        }
        
        return *this;
    }
    
    // Move assignment - no-throw guarantee
    AssignmentSafety& operator=(AssignmentSafety&& other) noexcept
    {
        EventLog::instance().record("operator=: move assignment");
        
        if (this != &other)
        {
            data_ = std::move(other.data_);
        }
        
        return *this;
    }
    
    bool has_data() const { return data_ != nullptr; }

private:
    std::unique_ptr<Tracked> data_;
};

// Q: What exception safety guarantee does the copy assignment operator provide?
// A:
// R:

// Q: What exception safety guarantee does the move assignment operator provide?
// A:
// R:

TEST_F(ExceptionSafetyTest, Assignment_SafetyGuarantees)
{
    // Hard: Assignment operators have different safety guarantees
    
    AssignmentSafety a1("A1");
    AssignmentSafety a2("A2");
    
    // Move assignment - no-throw
    static_assert(noexcept(a1 = std::move(a2)), "Move assignment should be noexcept");
    
    EventLog::instance().clear();
    
    AssignmentSafety a3("A3");
    AssignmentSafety a4("A4");
    
    // Copy assignment - may throw
    a3 = a4;
    
    EXPECT_EQ(EventLog::instance().count_events("operator=: old data deleted"), 1);
    EXPECT_EQ(EventLog::instance().count_events("operator=: new data created"), 1);
}

// ============================================================================
// Copy-and-Swap for Strong Assignment Safety
// ============================================================================

class StrongAssignmentSafety
{
public:
    StrongAssignmentSafety()
    : data_(nullptr)
    {
        EventLog::instance().record("StrongAssignmentSafety::ctor");
    }
    
    explicit StrongAssignmentSafety(const std::string& name)
    : data_(std::make_unique<Tracked>(name))
    {
        EventLog::instance().record("StrongAssignmentSafety::ctor(name)");
    }
    
    ~StrongAssignmentSafety()
    {
        EventLog::instance().record("StrongAssignmentSafety::dtor");
    }
    
    // Copy assignment using copy-and-swap idiom
    StrongAssignmentSafety& operator=(const StrongAssignmentSafety& other)
    {
        EventLog::instance().record("operator=: copy-and-swap start");
        
        if (this != &other)
        {
            // TODO: Create a temporary copy, then swap
            // This provides strong guarantee: if copy throws, original is unchanged
            
            std::unique_ptr<Tracked> temp;
            if (other.data_)
            {
                EventLog::instance().record("operator=: creating temp copy");
                temp = std::make_unique<Tracked>(other.data_->name());
            }
            
            // Swap is no-throw
            EventLog::instance().record("operator=: swapping");
            data_.swap(temp);
            
            // temp (holding old data) destroyed here
        }
        
        return *this;
    }
    
    bool has_data() const { return data_ != nullptr; }

private:
    std::unique_ptr<Tracked> data_;
};

// Q: Why does copy-and-swap provide strong exception safety?
// A:
// R:

// Q: What is the performance trade-off of copy-and-swap?
// A:
// R:

TEST_F(ExceptionSafetyTest, CopyAndSwapAssignment_StrongGuarantee)
{
    // Hard: Copy-and-swap idiom provides strong guarantee for assignment
    
    StrongAssignmentSafety a1("A1");
    StrongAssignmentSafety a2("A2");
    
    EventLog::instance().clear();
    
    // Copy assignment
    a1 = a2;
    
    EXPECT_TRUE(a1.has_data());
    
    // Verify copy-and-swap sequence
    EXPECT_EQ(EventLog::instance().count_events("operator=: creating temp copy"), 1);
    EXPECT_EQ(EventLog::instance().count_events("operator=: swapping"), 1);
    
    // Verify old A1 data was destroyed (in temp)
    EXPECT_EQ(EventLog::instance().count_events("Tracked(A1)::dtor"), 1);
}

// ============================================================================
// Exception Safety in Algorithms
// ============================================================================

void process_items(std::vector<std::unique_ptr<Tracked>>& items, bool throw_on_third)
{
    EventLog::instance().record("process_items: start");
    
    size_t count = 0;
    for (auto& item : items)
    {
        ++count;
        EventLog::instance().record("process_items: processing item " + std::to_string(count));
        
        if (throw_on_third && count == 3)
        {
            EventLog::instance().record("process_items: throwing on item 3");
            throw std::runtime_error("Processing failed");
        }
    }
    
    EventLog::instance().record("process_items: complete");
}

// Q: What exception safety guarantee does process_items provide?
// A:
// R:

// Q: Are the items in the vector still valid after the exception?
// A:
// R:

TEST_F(ExceptionSafetyTest, Algorithms_BasicSafety)
{
    // Moderate: Algorithms typically provide basic exception safety
    
    std::vector<std::unique_ptr<Tracked>> items;
    items.push_back(std::make_unique<Tracked>("Item1"));
    items.push_back(std::make_unique<Tracked>("Item2"));
    items.push_back(std::make_unique<Tracked>("Item3"));
    items.push_back(std::make_unique<Tracked>("Item4"));
    
    EventLog::instance().clear();
    
    try
    {
        process_items(items, true);
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Items are still valid (basic guarantee)
    EXPECT_EQ(items.size(), 4);
    
    // Verify processing stopped at item 3
    EXPECT_EQ(EventLog::instance().count_events("process_items: processing item 1"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_items: processing item 2"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_items: processing item 3"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_items: processing item 4"), 0);
}

// ============================================================================
// Transactional Operations - Strong Guarantee Pattern
// ============================================================================

class Transaction
{
public:
    Transaction()
    {
        EventLog::instance().record("Transaction::ctor");
    }
    
    ~Transaction()
    {
        if (!committed_)
        {
            EventLog::instance().record("Transaction::dtor - rolling back");
            rollback();
        }
        else
        {
            EventLog::instance().record("Transaction::dtor - already committed");
        }
    }
    
    void add_operation(std::unique_ptr<Tracked> item)
    {
        EventLog::instance().record("Transaction::add_operation");
        pending_.push_back(std::move(item));
    }
    
    void commit(bool throw_on_commit)
    {
        EventLog::instance().record("Transaction::commit - start");
        
        if (throw_on_commit)
        {
            EventLog::instance().record("Transaction::commit - throwing");
            throw std::runtime_error("Commit failed");
        }
        
        // Move pending operations to committed
        for (auto& item : pending_)
        {
            committed_items_.push_back(std::move(item));
        }
        pending_.clear();
        
        committed_ = true;
        EventLog::instance().record("Transaction::commit - success");
    }
    
    void rollback()
    {
        EventLog::instance().record("Transaction::rollback");
        pending_.clear();
    }
    
    size_t committed_count() const { return committed_items_.size(); }

private:
    std::vector<std::unique_ptr<Tracked>> pending_;
    std::vector<std::unique_ptr<Tracked>> committed_items_;
    bool committed_ = false;
};

// Q: What happens to pending operations if commit() throws?
// A:
// R:

// Q: What happens to pending operations if the Transaction is destroyed without commit()?
// A:
// R:

TEST_F(ExceptionSafetyTest, Transaction_StrongGuarantee)
{
    // Hard: Transaction pattern provides strong exception safety
    
    {
        Transaction txn;
        txn.add_operation(std::make_unique<Tracked>("Op1"));
        txn.add_operation(std::make_unique<Tracked>("Op2"));
        
        EventLog::instance().clear();
        
        try
        {
            txn.commit(true);
            FAIL() << "Should have thrown exception";
        }
        catch (const std::runtime_error&)
        {
            // Exception caught
        }
        
        // Transaction not committed
        EXPECT_EQ(txn.committed_count(), 0);
        EXPECT_EQ(EventLog::instance().count_events("Transaction::commit - success"), 0);
        
        // Destructor will rollback
    }
    
    // Verify rollback happened
    EXPECT_EQ(EventLog::instance().count_events("Transaction::dtor - rolling back"), 1);
    
    // Verify operations were destroyed
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Op1)::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Op2)::dtor"), 1);
}

// ============================================================================
// Exception Safety with Multiple Operations
// ============================================================================

class MultiOperationContainer
{
public:
    MultiOperationContainer()
    {
        EventLog::instance().record("MultiOperationContainer::ctor");
    }
    
    void add_multiple(std::vector<std::string> names, bool throw_on_last)
    {
        EventLog::instance().record("add_multiple: start");
        
        // Basic safety: add items one by one
        for (size_t i = 0; i < names.size(); ++i)
        {
            if (throw_on_last && i == names.size() - 1)
            {
                EventLog::instance().record("add_multiple: throwing on last item");
                throw std::runtime_error("Failed on last item");
            }
            
            items_.push_back(std::make_unique<Tracked>(names[i]));
            EventLog::instance().record("add_multiple: added " + names[i]);
        }
        
        EventLog::instance().record("add_multiple: success");
    }
    
    size_t size() const { return items_.size(); }

private:
    std::vector<std::unique_ptr<Tracked>> items_;
};

// Q: What exception safety guarantee does add_multiple provide?
// A:
// R:

// Q: How would you modify add_multiple to provide strong guarantee?
// A:
// R:

TEST_F(ExceptionSafetyTest, MultiOperation_BasicVsStrong)
{
    // Hard: Understanding the difference between basic and strong guarantees
    
    MultiOperationContainer container;
    
    try
    {
        container.add_multiple({"A", "B", "C", "D"}, true);
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }
    
    // Basic guarantee: first 3 items were added (state changed)
    EXPECT_EQ(container.size(), 3);
    
    // Verify partial completion
    EXPECT_EQ(EventLog::instance().count_events("Tracked(A)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(B)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(C)::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(D)::ctor"), 0);
    
    // Q: For strong guarantee, what would container.size() be after the exception?
    // A:
    // R:
}

// ============================================================================
// No-Throw Operations - Destructors and Swap
// ============================================================================

class NoThrowOperations
{
public:
    explicit NoThrowOperations(int value)
    : value_(value)
    {
        EventLog::instance().record("NoThrowOperations::ctor");
    }
    
    // Destructor must never throw
    ~NoThrowOperations() noexcept
    {
        EventLog::instance().record("NoThrowOperations::dtor");
        // Even if cleanup fails, must not throw
    }
    
    // Swap must be no-throw for strong exception safety
    void swap(NoThrowOperations& other) noexcept
    {
        EventLog::instance().record("NoThrowOperations::swap");
        std::swap(value_, other.value_);
    }
    
    int value() const { return value_; }

private:
    int value_;
};

// Q: Why must destructors never throw exceptions?
// A:
// R:

// Q: What happens if a destructor throws during stack unwinding?
// A:
// R:

TEST_F(ExceptionSafetyTest, NoThrow_DestructorAndSwap)
{
    // Moderate: Destructors and swap must provide no-throw guarantee
    
    // Verify destructor is noexcept
    {
        NoThrowOperations obj(42);
    }
    EXPECT_EQ(EventLog::instance().count_events("NoThrowOperations::dtor"), 1);
    
    // Verify swap is noexcept
    NoThrowOperations a(10);
    NoThrowOperations b(20);
    
    static_assert(noexcept(a.swap(b)), "swap must be noexcept");
    
    a.swap(b);
    EXPECT_EQ(a.value(), 20);
    EXPECT_EQ(b.value(), 10);
}

// ============================================================================
// Exception Safety Summary
// ============================================================================

// Q: Which exception safety guarantee is most common in the standard library?
// A:
// R:

// Q: When is it acceptable to provide only basic exception safety?
// A:
// R:

// Q: What operations must always provide no-throw guarantee?
// A:
// R:
