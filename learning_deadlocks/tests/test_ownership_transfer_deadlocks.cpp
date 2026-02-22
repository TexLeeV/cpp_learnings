#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <functional>

// Test Suite 4: Ownership Transfer + Lock Inversion Deadlocks
// Focus: shared_ptr transfer between threads with synchronization

class OwnershipTransferDeadlocksTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Lock Inversion During Ownership Transfer
// ============================================================================

class Mailbox
{
public:
    explicit Mailbox(const std::string& name)
    : name_(name)
    {
    }
    
    void send(std::shared_ptr<Tracked> msg, Mailbox& recipient)
    {
        // Lock sender first
        std::lock_guard<std::mutex> lock1(mutex_);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        // Then lock recipient
        std::lock_guard<std::mutex> lock2(recipient.mutex_);
        recipient.messages_.push_back(msg);
    }
    
    std::shared_ptr<Tracked> receive()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (messages_.empty())
        {
            return nullptr;
        }
        auto msg = messages_.back();
        messages_.pop_back();
        return msg;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::string name_;
    std::vector<std::shared_ptr<Tracked>> messages_;
    std::mutex mutex_;
};

TEST_F(OwnershipTransferDeadlocksTest, DISABLED_Scenario1_LockInversionDuringTransfer_Broken)
{
    Mailbox mb1("MB1");
    Mailbox mb2("MB2");
    Mailbox mb3("MB3");
    
    std::atomic<int> deadlock_count(0);
    
    // Thread 1: MB1 -> MB2
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        mb1.send(std::make_shared<Tracked>("Msg1"), mb2);
    });
    
    // Thread 2: MB2 -> MB3
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        mb2.send(std::make_shared<Tracked>("Msg2"), mb3);
    });
    
    // Thread 3: MB3 -> MB1
    std::thread t3([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        mb3.send(std::make_shared<Tracked>("Msg3"), mb1);
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: How does ownership transfer (shared_ptr) interact with lock ordering?
    // A: 
    // R: 
    
    // Q: What makes this different from simple data transfer without shared_ptr?
    // A: 
    // R: 
    
    EXPECT_GT(deadlock_count.load(), 0);
}

// FIX VERSION: Implement deadlock-free message sending
void send_fixed(Mailbox& sender, std::shared_ptr<Tracked> msg, Mailbox& recipient)
{
    // TODO: Implement deadlock-free send operation
    // Hint: Order locks by address or use std::lock()
    
    // YOUR CODE HERE
}

TEST_F(OwnershipTransferDeadlocksTest, Scenario1_LockInversionDuringTransfer_Fixed)
{
    Mailbox mb1("MB1");
    Mailbox mb2("MB2");
    Mailbox mb3("MB3");
    
    std::thread t1([&]()
    {
        send_fixed(mb1, std::make_shared<Tracked>("Msg1"), mb2);
    });
    
    std::thread t2([&]()
    {
        send_fixed(mb2, std::make_shared<Tracked>("Msg2"), mb3);
    });
    
    std::thread t3([&]()
    {
        send_fixed(mb3, std::make_shared<Tracked>("Msg3"), mb1);
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    EXPECT_TRUE(mb1.receive() != nullptr || mb2.receive() != nullptr || mb3.receive() != nullptr);
}

// ============================================================================
// Scenario 2: Callback with Lock Held
// ============================================================================

class EventHandler
{
public:
    explicit EventHandler(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    void set_callback(std::function<void(std::shared_ptr<Tracked>)> cb)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callback_ = cb;
    }
    
    // DEADLOCK VERSION: Calls callback while holding lock
    void trigger_deadlock(std::shared_ptr<Tracked> event)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        if (callback_)
        {
            callback_(event);
        }
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::function<void(std::shared_ptr<Tracked>)> callback_;
    std::mutex mutex_;
};

TEST_F(OwnershipTransferDeadlocksTest, DISABLED_Scenario2_CallbackWithLockHeld_Broken)
{
    EventHandler h1("H1");
    EventHandler h2("H2");
    
    // H1's callback triggers H2
    h1.set_callback([&](std::shared_ptr<Tracked> event)
    {
        h2.trigger_deadlock(event);
    });
    
    // H2's callback triggers H1
    h2.set_callback([&](std::shared_ptr<Tracked> event)
    {
        h1.trigger_deadlock(event);
    });
    
    std::atomic<int> deadlock_count(0);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        h1.trigger_deadlock(std::make_shared<Tracked>("Event1"));
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        h2.trigger_deadlock(std::make_shared<Tracked>("Event2"));
    });
    
    std::thread t3([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        h1.trigger_deadlock(std::make_shared<Tracked>("Event3"));
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: Why is calling unknown code (callbacks) while holding a lock dangerous?
    // A: 
    // R: 
    
    // Q: How does shared_ptr ownership transfer in callbacks affect deadlock risk?
    // A: 
    // R: 
    
    EXPECT_GT(deadlock_count.load(), 0);
}

// FIX VERSION: Release lock before callback
void trigger_fixed(EventHandler& handler, std::shared_ptr<Tracked> event)
{
    // TODO: Implement deadlock-free callback triggering
    // Hint: Copy callback, release lock, then invoke
    
    // YOUR CODE HERE
}

TEST_F(OwnershipTransferDeadlocksTest, Scenario2_CallbackWithLockHeld_Fixed)
{
    EventHandler h1("H1");
    EventHandler h2("H2");
    
    h1.set_callback([&](std::shared_ptr<Tracked> event)
    {
        trigger_fixed(h2, event);
    });
    
    h2.set_callback([&](std::shared_ptr<Tracked> event)
    {
        trigger_fixed(h1, event);
    });
    
    std::thread t1([&]()
    {
        trigger_fixed(h1, std::make_shared<Tracked>("Event1"));
    });
    
    std::thread t2([&]()
    {
        trigger_fixed(h2, std::make_shared<Tracked>("Event2"));
    });
    
    std::thread t3([&]()
    {
        trigger_fixed(h1, std::make_shared<Tracked>("Event3"));
    });
    
    t1.join();
    t2.join();
    t3.join();
}

// ============================================================================
// Scenario 3: Shared State with Multiple Accessors
// ============================================================================

class SharedState
{
public:
    explicit SharedState(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    std::shared_ptr<Tracked> get_data()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_;
    }
    
    void set_data(std::shared_ptr<Tracked> new_data)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        data_ = new_data;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::mutex mutex_;
};

// DEADLOCK VERSION: Swap with wrong lock order
void swap_state_deadlock(SharedState& s1, SharedState& s2)
{
    std::lock_guard<std::mutex> lock1(s1.get_mutex());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock2(s2.get_mutex());
    
    auto temp = s1.get_data();
    s1.set_data(s2.get_data());
    s2.set_data(temp);
}

TEST_F(OwnershipTransferDeadlocksTest, DISABLED_Scenario3_SharedStateSwap_Broken)
{
    SharedState s1("State1");
    SharedState s2("State2");
    SharedState s3("State3");
    
    std::atomic<int> deadlock_count(0);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        swap_state_deadlock(s1, s2);
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        swap_state_deadlock(s2, s3);
    });
    
    std::thread t3([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        swap_state_deadlock(s3, s1);
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: Why is swapping shared_ptr ownership particularly deadlock-prone?
    // A: 
    // R: 
    
    // Q: What happens to reference counts during a swap operation?
    // A: 
    // R: 
    
    EXPECT_GT(deadlock_count.load(), 0);
}

// FIX VERSION: Implement deadlock-free swap
void swap_state_fixed(SharedState& s1, SharedState& s2)
{
    // TODO: Implement deadlock-free state swap
    // Hint: Use std::lock() or consistent lock ordering
    
    // YOUR CODE HERE
}

TEST_F(OwnershipTransferDeadlocksTest, Scenario3_SharedStateSwap_Fixed)
{
    SharedState s1("State1");
    SharedState s2("State2");
    SharedState s3("State3");
    
    std::thread t1([&]() { swap_state_fixed(s1, s2); });
    std::thread t2([&]() { swap_state_fixed(s2, s3); });
    std::thread t3([&]() { swap_state_fixed(s3, s1); });
    
    t1.join();
    t2.join();
    t3.join();
}

// ============================================================================
// Scenario 4: Lazy Initialization Race with Lock
// ============================================================================

class LazyResource
{
public:
    LazyResource()
    {
    }
    
    // DEADLOCK VERSION: Double-checked locking done wrong
    std::shared_ptr<Tracked> get_deadlock(LazyResource& other)
    {
        if (!resource_)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            if (!resource_)
            {
                // Initialization depends on other resource (creates lock ordering issue)
                auto other_res = other.get_deadlock(*this);
                resource_ = std::make_shared<Tracked>("Resource");
            }
        }
        return resource_;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
    std::shared_ptr<Tracked> get_resource() const
    {
        return resource_;
    }
    
private:
    std::shared_ptr<Tracked> resource_;
    std::mutex mutex_;
};

TEST_F(OwnershipTransferDeadlocksTest, DISABLED_Scenario4_LazyInitializationRace_Broken)
{
    LazyResource r1;
    LazyResource r2;
    LazyResource r3;
    
    std::atomic<int> deadlock_count(0);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(200)))
        {
            deadlock_count++;
            return;
        }
        r1.get_deadlock(r2);
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(200)))
        {
            deadlock_count++;
            return;
        }
        r2.get_deadlock(r3);
    });
    
    std::thread t3([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(200)))
        {
            deadlock_count++;
            return;
        }
        r3.get_deadlock(r1);
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: Why is lazy initialization with dependencies particularly deadlock-prone?
    // A: 
    // R: 
    
    // Q: How does shared_ptr's atomic reference counting help or hurt here?
    // A: 
    // R: 
    
    EXPECT_GT(deadlock_count.load(), 0);
}

// FIX VERSION: Remove circular dependency or use proper initialization order
std::shared_ptr<Tracked> get_fixed(LazyResource& resource)
{
    // TODO: Implement deadlock-free lazy initialization
    // Hint: Either remove circular dependency or initialize in predetermined order
    
    // YOUR CODE HERE
    
    return nullptr;
}

TEST_F(OwnershipTransferDeadlocksTest, Scenario4_LazyInitializationRace_Fixed)
{
    LazyResource r1;
    LazyResource r2;
    LazyResource r3;
    
    std::thread t1([&]() { get_fixed(r1); });
    std::thread t2([&]() { get_fixed(r2); });
    std::thread t3([&]() { get_fixed(r3); });
    
    t1.join();
    t2.join();
    t3.join();
    
    EXPECT_TRUE(r1.get_resource() != nullptr);
}
