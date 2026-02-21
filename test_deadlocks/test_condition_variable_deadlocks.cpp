#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <queue>

// Test Suite 3: Condition Variable + shared_ptr Deadlocks
// Focus: Synchronization primitives with ownership transfer

class ConditionVariableDeadlocksTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Producer-Consumer with Wrong Wait Condition
// ============================================================================

class Queue
{
public:
    Queue()
    : shutdown_(false)
    {
    }
    
    void push(std::shared_ptr<Tracked> item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        cv_.notify_one();
    }
    
    // DEADLOCK VERSION: Wrong wait condition
    std::shared_ptr<Tracked> pop_deadlock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // BUG: Waits forever if queue is empty and no more items coming
        while (queue_.empty())
        {
            cv_.wait(lock);
        }
        
        auto item = queue_.front();
        queue_.pop();
        return item;
    }
    
    void shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cv_.notify_all();
    }
    
    bool is_shutdown() const
    {
        return shutdown_;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
    std::condition_variable& get_cv()
    {
        return cv_;
    }
    
private:
    std::queue<std::shared_ptr<Tracked>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_;
};

TEST_F(ConditionVariableDeadlocksTest, Scenario1_ProducerConsumerWrongWait_Broken)
{
    Queue queue;
    std::atomic<bool> consumer_stuck(false);
    
    // Producer: pushes 2 items
    std::thread producer([&]()
    {
        queue.push(std::make_shared<Tracked>("Item1"));
        queue.push(std::make_shared<Tracked>("Item2"));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.shutdown();
    });
    
    // Consumer: tries to pop 3 items (will block on 3rd)
    std::thread consumer([&]()
    {
        for (int i = 0; i < 3; ++i)
        {
            std::timed_mutex timeout;
            if (!timeout.try_lock_for(std::chrono::milliseconds(200)))
            {
                consumer_stuck = true;
                return;
            }
            
            auto item = queue.pop_deadlock();
            if (!item)
            {
                break;
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    // Q: Why does the consumer thread block forever waiting for the 3rd item?
    // A: 
    // R: 
    
    // Q: What should the wait condition check besides queue.empty()?
    // A: 
    // R: 
}

// FIX VERSION: Implement correct wait condition
std::shared_ptr<Tracked> pop_fixed(Queue& queue)
{
    // TODO: Implement pop with correct wait condition
    // Hint: Check both queue.empty() AND shutdown flag
    
    // YOUR CODE HERE
    
    return nullptr;
}

TEST_F(ConditionVariableDeadlocksTest, Scenario1_ProducerConsumerWrongWait_Fixed)
{
    Queue queue;
    std::atomic<int> items_consumed(0);
    
    std::thread producer([&]()
    {
        queue.push(std::make_shared<Tracked>("Item1"));
        queue.push(std::make_shared<Tracked>("Item2"));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.shutdown();
    });
    
    std::thread consumer([&]()
    {
        while (true)
        {
            auto item = pop_fixed(queue);
            if (!item)
            {
                break;
            }
            items_consumed++;
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(items_consumed.load(), 2);
}

// ============================================================================
// Scenario 2: Notify Before Wait (Lost Wakeup)
// ============================================================================

class Signal
{
public:
    Signal()
    : ready_(false)
    {
    }
    
    // DEADLOCK VERSION: Notify can happen before wait
    void notify_deadlock()
    {
        ready_ = true;
        cv_.notify_one();
    }
    
    void wait_deadlock()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
        std::unique_lock<std::mutex> lock(mutex_);
        // BUG: If notify happened before wait, we wait forever
        cv_.wait(lock);
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
    bool is_ready() const
    {
        return ready_;
    }
    
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool ready_;
};

TEST_F(ConditionVariableDeadlocksTest, Scenario2_NotifyBeforeWait_Broken)
{
    Signal signal;
    std::atomic<bool> waiter_stuck(false);
    
    // Thread 1: Notifies immediately
    std::thread notifier([&]()
    {
        signal.notify_deadlock();
    });
    
    // Thread 2: Waits (but notify already happened)
    std::thread waiter([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            waiter_stuck = true;
            return;
        }
        signal.wait_deadlock();
    });
    
    notifier.join();
    waiter.join();
    
    // Q: Why does the waiter miss the notification?
    // A: 
    // R: 
    
    // Q: What pattern prevents lost wakeups?
    // A: 
    // R: 
}

// FIX VERSION: Implement proper wait with predicate
void wait_fixed(Signal& signal)
{
    // TODO: Implement wait that doesn't miss notifications
    // Hint: Always check condition in a loop with predicate
    
    // YOUR CODE HERE
}

TEST_F(ConditionVariableDeadlocksTest, Scenario2_NotifyBeforeWait_Fixed)
{
    Signal signal;
    
    std::thread notifier([&]()
    {
        signal.notify_deadlock();
    });
    
    std::thread waiter([&]()
    {
        wait_fixed(signal);
    });
    
    notifier.join();
    waiter.join();
    
    EXPECT_TRUE(signal.is_ready());
}

// ============================================================================
// Scenario 3: Deadlock with Nested Condition Variables
// ============================================================================

class TwoStageQueue
{
public:
    TwoStageQueue()
    : stage1_ready_(false)
    , stage2_ready_(false)
    {
    }
    
    // DEADLOCK VERSION: Holds lock while waiting on another CV
    void process_deadlock()
    {
        // Stage 1: Wait for data
        std::unique_lock<std::mutex> lock1(mutex1_);
        cv1_.wait(lock1, [this]() { return stage1_ready_; });
        
        // Stage 2: Hold lock1, try to signal stage2
        std::lock_guard<std::mutex> lock2(mutex2_);
        stage2_ready_ = true;
        cv2_.notify_one();
    }
    
    void reverse_process_deadlock()
    {
        // Stage 2: Wait for stage2
        std::unique_lock<std::mutex> lock2(mutex2_);
        cv2_.wait(lock2, [this]() { return stage2_ready_; });
        
        // Stage 1: Hold lock2, try to signal stage1
        std::lock_guard<std::mutex> lock1(mutex1_);
        stage1_ready_ = true;
        cv1_.notify_one();
    }
    
    void trigger_stage1()
    {
        std::lock_guard<std::mutex> lock(mutex1_);
        stage1_ready_ = true;
        cv1_.notify_one();
    }
    
    void trigger_stage2()
    {
        std::lock_guard<std::mutex> lock(mutex2_);
        stage2_ready_ = true;
        cv2_.notify_one();
    }
    
    std::mutex& get_mutex1()
    {
        return mutex1_;
    }
    
    std::mutex& get_mutex2()
    {
        return mutex2_;
    }
    
private:
    std::mutex mutex1_;
    std::mutex mutex2_;
    std::condition_variable cv1_;
    std::condition_variable cv2_;
    bool stage1_ready_;
    bool stage2_ready_;
};

TEST_F(ConditionVariableDeadlocksTest, Scenario3_NestedConditionVariables_Broken)
{
    TwoStageQueue queue;
    std::atomic<int> deadlock_count(0);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(200)))
        {
            deadlock_count++;
            return;
        }
        queue.process_deadlock();
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(200)))
        {
            deadlock_count++;
            return;
        }
        queue.reverse_process_deadlock();
    });
    
    std::thread t3([&]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        queue.trigger_stage1();
        queue.trigger_stage2();
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: Why does holding one lock while acquiring another cause deadlock here?
    // A: 
    // R: 
    
    // Q: What's the difference between this and simple mutex deadlock?
    // A: 
    // R: 
    
    EXPECT_GT(deadlock_count.load(), 0);
}

// FIX VERSION: Release locks between stages
void process_fixed(TwoStageQueue& queue)
{
    // TODO: Implement deadlock-free two-stage processing
    // Hint: Release lock1 before acquiring lock2
    
    // YOUR CODE HERE
}

void reverse_process_fixed(TwoStageQueue& queue)
{
    // TODO: Implement deadlock-free reverse processing
    
    // YOUR CODE HERE
}

TEST_F(ConditionVariableDeadlocksTest, Scenario3_NestedConditionVariables_Fixed)
{
    TwoStageQueue queue;
    
    std::thread t1([&]() { process_fixed(queue); });
    std::thread t2([&]() { reverse_process_fixed(queue); });
    std::thread t3([&]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        queue.trigger_stage1();
        queue.trigger_stage2();
    });
    
    t1.join();
    t2.join();
    t3.join();
}

// ============================================================================
// Scenario 4: Spurious Wakeup Without Proper Predicate
// ============================================================================

class WorkQueue
{
public:
    WorkQueue()
    : work_available_(false)
    , shutdown_(false)
    {
    }
    
    void add_work(std::shared_ptr<Tracked> work)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        work_ = work;
        work_available_ = true;
        cv_.notify_one();
    }
    
    // DEADLOCK VERSION: Doesn't handle spurious wakeups correctly
    std::shared_ptr<Tracked> get_work_deadlock()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // BUG: Single if instead of while - spurious wakeup causes early return
        if (!work_available_ && !shutdown_)
        {
            cv_.wait(lock);
        }
        
        if (shutdown_)
        {
            return nullptr;
        }
        
        work_available_ = false;
        return work_;
    }
    
    void shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cv_.notify_all();
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
    bool is_shutdown() const
    {
        return shutdown_;
    }
    
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::shared_ptr<Tracked> work_;
    bool work_available_;
    bool shutdown_;
};

TEST_F(ConditionVariableDeadlocksTest, Scenario4_SpuriousWakeupHandling_Broken)
{
    WorkQueue queue;
    std::atomic<int> null_work_count(0);
    
    // Producer: adds work slowly
    std::thread producer([&]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.add_work(std::make_shared<Tracked>("Work1"));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.shutdown();
    });
    
    // Consumers: may get spurious wakeups
    auto consumer_func = [&]()
    {
        for (int i = 0; i < 3; ++i)
        {
            auto work = queue.get_work_deadlock();
            if (!work)
            {
                null_work_count++;
            }
            
            if (queue.is_shutdown())
            {
                break;
            }
        }
    };
    
    std::thread c1(consumer_func);
    std::thread c2(consumer_func);
    std::thread c3(consumer_func);
    
    producer.join();
    c1.join();
    c2.join();
    c3.join();
    
    // Q: What is a spurious wakeup and why does it happen?
    // A: 
    // R: 
    
    // Q: Why must condition variable waits always use a while loop?
    // A: 
    // R: 
    
    // Spurious wakeups may cause unexpected null returns
    EXPECT_GT(null_work_count.load(), 1);
}

// FIX VERSION: Use while loop with proper predicate
std::shared_ptr<Tracked> get_work_fixed(WorkQueue& queue)
{
    // TODO: Implement get_work that handles spurious wakeups correctly
    // Hint: Use while loop and check condition after every wakeup
    
    // YOUR CODE HERE
    
    return nullptr;
}

TEST_F(ConditionVariableDeadlocksTest, Scenario4_SpuriousWakeupHandling_Fixed)
{
    WorkQueue queue;
    std::atomic<int> work_count(0);
    
    std::thread producer([&]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.add_work(std::make_shared<Tracked>("Work1"));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.shutdown();
    });
    
    auto consumer_func = [&]()
    {
        while (true)
        {
            auto work = get_work_fixed(queue);
            if (!work)
            {
                break;
            }
            work_count++;
        }
    };
    
    std::thread c1(consumer_func);
    std::thread c2(consumer_func);
    std::thread c3(consumer_func);
    
    producer.join();
    c1.join();
    c2.join();
    c3.join();
    
    EXPECT_EQ(work_count.load(), 1);
}
