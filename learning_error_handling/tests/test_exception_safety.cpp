// Test Suite: Exception Safety Guarantees (Basic, Strong, No-Throw)
// Estimated Time: 3 hours
// Difficulty: Moderate


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

class CopyAndSwapAssignment
{
public:
    CopyAndSwapAssignment()
    : data_(nullptr)
    {
        EventLog::instance().record("CopyAndSwapAssignment::ctor");
    }
    
    explicit CopyAndSwapAssignment(const std::string& name)
    : data_(std::make_unique<Tracked>(name))
    {
        EventLog::instance().record("CopyAndSwapAssignment::ctor(name)");
    }
    
    ~CopyAndSwapAssignment()
    {
        EventLog::instance().record("CopyAndSwapAssignment::dtor");
    }
    
    CopyAndSwapAssignment& operator=(const CopyAndSwapAssignment& other)
    {
        EventLog::instance().record("operator=: copy-and-swap start");
        
        if (this != &other)
        {
            // TODO: Create a temporary copy, then swap
            std::unique_ptr<Tracked> temp;
            if (other.data_)
            {
                EventLog::instance().record("operator=: creating temp copy");
                temp = std::make_unique<Tracked>(other.data_->name());
            }
            
            EventLog::instance().record("operator=: swapping");
            data_.swap(temp);
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

// Q: What happens to the old data when the swap completes?
// A:
// R:

TEST_F(ExceptionSafetyTest, CopyAndSwap_StrongGuarantee)
{
    // Hard: Copy-and-swap idiom provides strong guarantee for assignment
    
    CopyAndSwapAssignment a1("A1");
    CopyAndSwapAssignment a2("A2");
    
    EventLog::instance().clear();
    
    a1 = a2;
    
    EXPECT_TRUE(a1.has_data());
    EXPECT_EQ(EventLog::instance().count_events("operator=: creating temp copy"), 1);
    EXPECT_EQ(EventLog::instance().count_events("operator=: swapping"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(A1)::dtor"), 1);
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
        
        EXPECT_EQ(txn.committed_count(), 0);
        EXPECT_EQ(EventLog::instance().count_events("Transaction::commit - success"), 0);
    }
    
    EXPECT_EQ(EventLog::instance().count_events("Transaction::dtor - rolling back"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Op1)::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Op2)::dtor"), 1);
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

