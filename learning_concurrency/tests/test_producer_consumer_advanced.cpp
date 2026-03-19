// Test Suite: Producer-Consumer Advanced Patterns
// Estimated Time: 4 hours
// Difficulty: Moderate
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>
#include <optional>

class ProducerConsumerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic Producer-Consumer with Unbounded Queue - Easy
// ============================================================================

template<typename T>
class UnboundedQueue
{
public:
    void push(T value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
            EventLog::instance().record("UnboundedQueue::push()");
        }
        cv_.notify_one();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty(); });
        EventLog::instance().record("UnboundedQueue::pop()");
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

TEST_F(ProducerConsumerTest, BasicProducerConsumer)
{
    UnboundedQueue<int> queue;
    constexpr int num_items = 10;
    std::atomic<int> consumed{0};

    std::thread producer([&queue, num_items]()
    {
        for (int i = 0; i < num_items; ++i)
        {
            queue.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    std::thread consumer([&queue, &consumed, num_items]()
    {
        for (int i = 0; i < num_items; ++i)
        {
            int val = queue.pop();
            consumed.fetch_add(1);
        }
    });

    producer.join();
    consumer.join();

    // Q: What happens if the consumer calls pop() when the queue is empty?
    // Q: What mechanism prevents busy-waiting?
    // A:
    // R:

    EXPECT_EQ(consumed.load(), num_items);
    EXPECT_EQ(EventLog::instance().count_events("::push()"), num_items);
    EXPECT_EQ(EventLog::instance().count_events("::pop()"), num_items);
}

// ============================================================================
// TEST 2: Multiple Producers, Multiple Consumers - Moderate
// ============================================================================

TEST_F(ProducerConsumerTest, MultipleProducersConsumers)
{
    UnboundedQueue<int> queue;
    constexpr int num_producers = 3;
    constexpr int num_consumers = 3;
    constexpr int items_per_producer = 10;
    std::atomic<int> total_consumed{0};

    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p)
    {
        producers.emplace_back([&queue, p, items_per_producer]()
        {
            for (int i = 0; i < items_per_producer; ++i)
            {
                queue.push(p * 100 + i);
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c)
    {
        consumers.emplace_back([&queue, &total_consumed, items_per_producer, num_producers]()
        {
            int items_to_consume = (items_per_producer * num_producers) / num_consumers;
            for (int i = 0; i < items_to_consume; ++i)
            {
                queue.pop();
                total_consumed.fetch_add(1);
            }
        });
    }

    for (auto& p : producers)
    {
        p.join();
    }

    for (auto& c : consumers)
    {
        c.join();
    }

    // Q: With multiple consumers calling pop(), what ensures only one consumer
    // Q: receives each item? What happens inside condition_variable::wait()?
    // A:
    // R:

    EXPECT_EQ(total_consumed.load(), num_producers * items_per_producer);
}

// ============================================================================
// TEST 3: Bounded Queue with Blocking - Moderate
// ============================================================================

template<typename T>
class BoundedQueue
{
public:
    explicit BoundedQueue(size_t capacity)
    : capacity_(capacity)
    {
    }

    void push(T value)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this]() { return queue_.size() < capacity_; });
        queue_.push(std::move(value));
        EventLog::instance().record("BoundedQueue::push()");
        not_empty_.notify_one();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this]() { return !queue_.empty(); });
        T value = std::move(queue_.front());
        queue_.pop();
        EventLog::instance().record("BoundedQueue::pop()");
        not_full_.notify_one();
        return value;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
};

TEST_F(ProducerConsumerTest, BoundedQueueBlocking)
{
    BoundedQueue<int> queue(5);
    std::atomic<bool> producer_blocked{false};
    std::atomic<int> items_produced{0};

    std::thread fast_producer([&queue, &producer_blocked, &items_produced]()
    {
        for (int i = 0; i < 10; ++i)
        {
            if (i == 5)
            {
                producer_blocked.store(true);
            }
            queue.push(i);
            items_produced.fetch_add(1);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_TRUE(producer_blocked.load());
    EXPECT_LE(items_produced.load(), 6);

    std::thread consumer([&queue]()
    {
        for (int i = 0; i < 10; ++i)
        {
            queue.pop();
        }
    });

    fast_producer.join();
    consumer.join();

    // Q: After the queue reaches capacity (5 items), the producer blocks in push().
    // Q: What condition_variable predicate must become true before push() proceeds?
    // A:
    // R:

    // Q: When pop() removes an item, it calls not_full_.notify_one(). Why is this
    // Q: necessary? What thread is waiting on not_full_?
    // A:
    // R:

    EXPECT_EQ(items_produced.load(), 10);
    EXPECT_EQ(queue.size(), 0);
}

// ============================================================================
// TEST 4: TODO - Implement Priority Queue with Condition Variables - Hard
// ============================================================================

// TODO: Implement a thread-safe priority queue where:
// TODO: 1. Higher priority items are consumed first
// TODO: 2. Producers can add items with different priorities
// TODO: 3. Consumers block when queue is empty
// TODO: Use std::priority_queue and condition_variable

template<typename T>
class PriorityQueue
{
public:
    void push(T value, int priority)
    {
        // TODO: Implement thread-safe push with priority
    }

    T pop()
    {
        // TODO: Implement thread-safe pop that blocks when empty
        return T{};
    }

private:
    // TODO: Add std::priority_queue, mutex, and condition_variable
};

TEST_F(ProducerConsumerTest, DISABLED_PriorityQueueOrdering)
{
    PriorityQueue<int> queue;

    std::thread producer([&queue]()
    {
        queue.push(1, 1);
        queue.push(3, 3);
        queue.push(2, 2);
    });

    producer.join();

    std::vector<int> consumed;
    std::thread consumer([&queue, &consumed]()
    {
        for (int i = 0; i < 3; ++i)
        {
            consumed.push_back(queue.pop());
        }
    });

    consumer.join();

    // Q: With priorities 1, 3, 2 inserted in that order, what order are items consumed?
    // Q: Does std::priority_queue provide max-heap or min-heap by default?
    // A:
    // R:

    EXPECT_EQ(consumed[0], 3);
    EXPECT_EQ(consumed[1], 2);
    EXPECT_EQ(consumed[2], 1);
}

// ============================================================================
// TEST 5: Spurious Wakeups and Predicate Importance - Hard
// ============================================================================

template<typename T>
class WakeupDemoQueue
{
public:
    void push(T value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }

    T pop_with_predicate()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty(); });
        EventLog::instance().record("pop_with_predicate: woke up with non-empty queue");
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    T pop_without_predicate()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock);
        if (queue_.empty())
        {
            EventLog::instance().record("pop_without_predicate: spurious wakeup detected");
            return T{};
        }
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

TEST_F(ProducerConsumerTest, SpuriousWakeupsWithPredicate)
{
    WakeupDemoQueue<int> queue;

    std::thread consumer([&queue]()
    {
        int val = queue.pop_with_predicate();
        EXPECT_EQ(val, 42);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    queue.push(42);

    consumer.join();

    // Q: The wait() call with a predicate is equivalent to a loop: while (!pred()) wait().
    // Q: Why is the loop necessary? What can cause wait() to return even when the queue is empty?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("woke up with non-empty queue"), 1);
    EXPECT_EQ(EventLog::instance().count_events("spurious wakeup"), 0);
}

// ============================================================================
// TEST 6: notify_one vs notify_all - Moderate
// ============================================================================

class NotificationDemo
{
public:
    void signal_one()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ready_ = true;
        }
        cv_.notify_one();
        EventLog::instance().record("NotificationDemo::signal_one() called");
    }

    void signal_all()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ready_ = true;
        }
        cv_.notify_all();
        EventLog::instance().record("NotificationDemo::signal_all() called");
    }

    void wait_for_signal(int thread_id)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return ready_; });
        EventLog::instance().record("Thread " + std::to_string(thread_id) + " woke up");
    }

    void reset()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ready_ = false;
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool ready_ = false;
};

TEST_F(ProducerConsumerTest, NotifyOneVsNotifyAll)
{
    NotificationDemo demo;
    constexpr int num_waiters = 5;

    std::vector<std::thread> waiters;
    for (int i = 0; i < num_waiters; ++i)
    {
        waiters.emplace_back([&demo, i]()
        {
            demo.wait_for_signal(i);
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    demo.signal_all();

    for (auto& w : waiters)
    {
        w.join();
    }

    // Q: If signal_one() were called instead of signal_all(), how many threads would
    // Q: wake up? What would happen to the remaining threads?
    // A:
    // R:

    // Q: After one thread wakes from notify_one(), could it call notify_one() again
    // Q: to wake another thread? What pattern would enable this?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("woke up"), num_waiters);
}
