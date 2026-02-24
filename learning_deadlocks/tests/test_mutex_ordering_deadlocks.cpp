#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <vector>

// Test Suite 1: Mutex Ordering Deadlocks
// Focus: Classic lock ordering problems with shared_ptr ownership

class MutexOrderingDeadlocksTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Classic Two-Resource Deadlock (A->B vs B->A)
// ============================================================================

class Account
{
public:
    explicit Account(const std::string& name, int balance)
    : data_(std::make_shared<Tracked>(name))
    , balance_(balance)
    {
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
    int balance() const
    {
        return balance_;
    }
    
    void add(int amount)
    {
        balance_ += amount;
    }
    
    void subtract(int amount)
    {
        balance_ -= amount;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::mutex mutex_;
    int balance_;
};

// DEADLOCK VERSION: Incorrect lock ordering
void transfer_deadlock(Account& from, Account& to, int amount)
{
    // Thread 1: locks from, then to
    // Thread 2: locks to, then from
    // Result: DEADLOCK
    std::lock_guard<std::mutex> lock1(from.get_mutex());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock2(to.get_mutex());
    
    from.subtract(amount);
    to.add(amount);
}

TEST_F(MutexOrderingDeadlocksTest, DISABLED_Scenario1_ClassicTwoResourceDeadlock_Broken)
{
    Account acc1("Account1", 1000);
    Account acc2("Account2", 1000);
    
    std::atomic<bool> thread1_stuck(false);
    std::atomic<bool> thread2_stuck(false);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout_mutex;
        if (!timeout_mutex.try_lock_for(std::chrono::milliseconds(100)))
        {
            thread1_stuck = true;
            return;
        }
        transfer_deadlock(acc1, acc2, 100);
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout_mutex;
        if (!timeout_mutex.try_lock_for(std::chrono::milliseconds(100)))
        {
            thread2_stuck = true;
            return;
        }
        transfer_deadlock(acc2, acc1, 50);
    });
    
    t1.join();
    t2.join();
    
    // Q: Why does this deadlock occur even though each thread locks in "from->to" order?
    // A: 
    // R: 
    
    // Q: What is the lock acquisition sequence that causes deadlock?
    // A: 
    // R: 
}

// FIX VERSION: Implement correct lock ordering
void transfer_fixed(Account& from, Account& to, int amount)
{
    // TODO: Implement deadlock-free transfer
    // Hint: Consider ordering locks by address or using std::lock()
    
    // YOUR CODE HERE
}

TEST_F(MutexOrderingDeadlocksTest, Scenario1_ClassicTwoResourceDeadlock_Fixed)
{
    Account acc1("Account1", 1000);
    Account acc2("Account2", 1000);
    
    std::thread t1([&]() { transfer_fixed(acc1, acc2, 100); });
    std::thread t2([&]() { transfer_fixed(acc2, acc1, 50); });
    
    t1.join();
    t2.join();
    
    EXPECT_EQ(acc1.balance(), 950);
    EXPECT_EQ(acc2.balance(), 1050);
}

// ============================================================================
// Scenario 2: Three-Thread Circular Deadlock (A->B->C->A)
// ============================================================================

class Resource
{
public:
    explicit Resource(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    std::timed_mutex& get_mutex()
    {
        return mutex_;
    }
    
    std::shared_ptr<Tracked> get_data()
    {
        return data_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::timed_mutex mutex_;
};

// DEADLOCK VERSION: Circular wait
bool process_two_resources_deadlock(Resource& r1, Resource& r2)
{
    if (!r1.get_mutex().try_lock_for(std::chrono::milliseconds(50)))
    {
        return false;
    }
    std::lock_guard<std::timed_mutex> lock1(r1.get_mutex(), std::adopt_lock);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    if (!r2.get_mutex().try_lock_for(std::chrono::milliseconds(50)))
    {
        return false;
    }
    std::lock_guard<std::timed_mutex> lock2(r2.get_mutex(), std::adopt_lock);
    
    // Process resources
    std::shared_ptr<Tracked> d1 = r1.get_data();
    std::shared_ptr<Tracked> d2 = r2.get_data();
    
    return true;
}

TEST_F(MutexOrderingDeadlocksTest, DISABLED_Scenario2_ThreeThreadCircularDeadlock_Broken)
{
    Resource r1("R1");
    Resource r2("R2");
    Resource r3("R3");
    
    std::atomic<int> deadlock_count(0);
    
    // Thread 1: R1 -> R2
    std::thread t1([&]()
    {
        if (!process_two_resources_deadlock(r1, r2))
        {
            deadlock_count++;
        }
    });
    
    // Thread 2: R2 -> R3
    std::thread t2([&]()
    {
        if (!process_two_resources_deadlock(r2, r3))
        {
            deadlock_count++;
        }
    });
    
    // Thread 3: R3 -> R1
    std::thread t3([&]()
    {
        if (!process_two_resources_deadlock(r3, r1))
        {
            deadlock_count++;
        }
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: What circular wait condition exists in this scenario?
    // A: 
    // R: 
    
    // Q: Why does adding a third thread make deadlock more likely than with two threads?
    // A: 
    // R: 
    
    EXPECT_GT(deadlock_count.load(), 0);
}

// FIX VERSION: Implement deadlock-free processing
bool process_two_resources_fixed(Resource& r1, Resource& r2)
{
    // TODO: Implement deadlock-free resource processing
    // Hint: Establish a global ordering for all resources
    
    // YOUR CODE HERE
    
    return false;
}

TEST_F(MutexOrderingDeadlocksTest, Scenario2_ThreeThreadCircularDeadlock_Fixed)
{
    Resource r1("R1");
    Resource r2("R2");
    Resource r3("R3");
    
    std::atomic<int> success_count(0);
    
    std::thread t1([&]()
    {
        if (process_two_resources_fixed(r1, r2))
        {
            success_count++;
        }
    });
    
    std::thread t2([&]()
    {
        if (process_two_resources_fixed(r2, r3))
        {
            success_count++;
        }
    });
    
    std::thread t3([&]()
    {
        if (process_two_resources_fixed(r3, r1))
        {
            success_count++;
        }
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    EXPECT_EQ(success_count.load(), 3);
}

// ============================================================================
// Scenario 3: Nested Lock Acquisition with shared_ptr
// ============================================================================

class Container
{
public:
    explicit Container(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    void set_next(std::shared_ptr<Container> next)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        next_ = next;
    }
    
    std::shared_ptr<Container> get_next()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return next_;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::shared_ptr<Container> next_;
    std::mutex mutex_;
};

// DEADLOCK VERSION: Nested lock acquisition
void link_containers_deadlock(std::shared_ptr<Container> c1, std::shared_ptr<Container> c2)
{
    std::lock_guard<std::mutex> lock1(c1->get_mutex());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Acquiring c2's lock while holding c1's lock
    c1->set_next(c2);
}

TEST_F(MutexOrderingDeadlocksTest, DISABLED_Scenario3_NestedLockAcquisition_Broken)
{
    auto c1 = std::make_shared<Container>("C1");
    auto c2 = std::make_shared<Container>("C2");
    
    std::atomic<bool> deadlock_detected(false);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_detected = true;
            return;
        }
        link_containers_deadlock(c1, c2);
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_detected = true;
            return;
        }
        link_containers_deadlock(c2, c1);
    });
    
    t1.join();
    t2.join();
    
    // Q: How does nested locking (lock within lock) create deadlock risk?
    // A: 
    // R: 
    
    // Q: What role does shared_ptr play in this deadlock scenario?
    // A: 
    // R: 
}

// FIX VERSION: Implement safe nested locking
void link_containers_fixed(std::shared_ptr<Container> c1, std::shared_ptr<Container> c2)
{
    // TODO: Implement deadlock-free container linking
    // Hint: Consider lock ordering or std::lock for multiple mutexes
    
    // YOUR CODE HERE
}

TEST_F(MutexOrderingDeadlocksTest, Scenario3_NestedLockAcquisition_Fixed)
{
    auto c1 = std::make_shared<Container>("C1");
    auto c2 = std::make_shared<Container>("C2");
    
    std::thread t1([&]() { link_containers_fixed(c1, c2); });
    std::thread t2([&]() { link_containers_fixed(c2, c1); });
    
    t1.join();
    t2.join();
    
    EXPECT_TRUE(c1->get_next() != nullptr || c2->get_next() != nullptr);
}

// ============================================================================
// Scenario 4: Lock Hierarchy Violation
// ============================================================================

class HierarchicalResource
{
public:
    HierarchicalResource(const std::string& name, int level)
    : data_(std::make_shared<Tracked>(name))
    , level_(level)
    {
    }
    
    int level() const
    {
        return level_;
    }
    
    std::timed_mutex& get_mutex()
    {
        return mutex_;
    }
    
    void use_with(HierarchicalResource& other)
    {
        // Simulated work
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
private:
    std::shared_ptr<Tracked> data_;
    int level_;
    std::timed_mutex mutex_;
};

// DEADLOCK VERSION: Violates lock hierarchy
bool acquire_hierarchical_deadlock(HierarchicalResource& r1, HierarchicalResource& r2)
{
    // Incorrect: locks in order of acquisition, not hierarchy level
    if (!r1.get_mutex().try_lock_for(std::chrono::milliseconds(50)))
    {
        return false;
    }
    std::lock_guard<std::timed_mutex> lock1(r1.get_mutex(), std::adopt_lock);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    if (!r2.get_mutex().try_lock_for(std::chrono::milliseconds(50)))
    {
        return false;
    }
    std::lock_guard<std::timed_mutex> lock2(r2.get_mutex(), std::adopt_lock);
    
    r1.use_with(r2);
    return true;
}

TEST_F(MutexOrderingDeadlocksTest, DISABLED_Scenario4_LockHierarchyViolation_Broken)
{
    HierarchicalResource high("High", 10);
    HierarchicalResource low("Low", 1);
    
    std::atomic<int> failure_count(0);
    
    // Thread 1: high -> low (correct hierarchy)
    std::thread t1([&]()
    {
        if (!acquire_hierarchical_deadlock(high, low))
        {
            failure_count++;
        }
    });
    
    // Thread 2: low -> high (violates hierarchy)
    std::thread t2([&]()
    {
        if (!acquire_hierarchical_deadlock(low, high))
        {
            failure_count++;
        }
    });
    
    // Thread 3: high -> low (correct hierarchy)
    std::thread t3([&]()
    {
        if (!acquire_hierarchical_deadlock(high, low))
        {
            failure_count++;
        }
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: What is a lock hierarchy and why does it prevent deadlocks?
    // A: 
    // R: 
    
    // Q: Which thread(s) violate the hierarchy in this scenario?
    // A: 
    // R: 
    
    EXPECT_GT(failure_count.load(), 0);
}

// FIX VERSION: Implement hierarchy-respecting locks
bool acquire_hierarchical_fixed(HierarchicalResource& r1, HierarchicalResource& r2)
{
    // TODO: Implement lock acquisition that respects hierarchy levels
    // Hint: Always lock higher level before lower level
    
    // YOUR CODE HERE
    
    return false;
}

TEST_F(MutexOrderingDeadlocksTest, Scenario4_LockHierarchyViolation_Fixed)
{
    HierarchicalResource high("High", 10);
    HierarchicalResource low("Low", 1);
    
    std::atomic<int> success_count(0);
    
    std::thread t1([&]()
    {
        if (acquire_hierarchical_fixed(high, low))
        {
            success_count++;
        }
    });
    
    std::thread t2([&]()
    {
        if (acquire_hierarchical_fixed(low, high))
        {
            success_count++;
        }
    });
    
    std::thread t3([&]()
    {
        if (acquire_hierarchical_fixed(high, low))
        {
            success_count++;
        }
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    EXPECT_EQ(success_count.load(), 3);
}
