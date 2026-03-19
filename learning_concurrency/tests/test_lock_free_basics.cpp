// Test Suite: Lock-Free Data Structures Basics
// Estimated Time: 6 hours
// Difficulty: Hard
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

class LockFreeBasicsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Atomic Operations and Memory Ordering - Easy
// ============================================================================

TEST_F(LockFreeBasicsTest, AtomicOperationsBasics)
{
    std::atomic<int> counter{0};
    constexpr int num_threads = 10;
    constexpr int increments_per_thread = 1000;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&counter, increments_per_thread]()
        {
            for (int j = 0; j < increments_per_thread; ++j)
            {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: Why does fetch_add with memory_order_relaxed still produce the correct count?
    // Q: What guarantee does the atomic operation itself provide?
    // A:
    // R:

    // Q: If counter were a regular int instead of std::atomic<int>, what race condition
    // Q: would occur during the read-modify-write of counter++?
    // A:
    // R:

    EXPECT_EQ(counter.load(), num_threads * increments_per_thread);
}

// ============================================================================
// TEST 2: Compare-Exchange and ABA Problem Introduction - Moderate
// ============================================================================

struct Node
{
    int value;
    Node* next;

    explicit Node(int v) : value(v), next(nullptr)
    {
        EventLog::instance().record("Node::ctor value=" + std::to_string(v));
    }

    ~Node()
    {
        EventLog::instance().record("Node::dtor value=" + std::to_string(value));
    }
};

class SimpleLockFreeStack
{
public:
    SimpleLockFreeStack() : head_(nullptr) {}

    void push(int value)
    {
        Node* new_node = new Node(value);
        new_node->next = head_.load(std::memory_order_relaxed);

        while (!head_.compare_exchange_weak(new_node->next, new_node,
                                            std::memory_order_release,
                                            std::memory_order_relaxed))
        {
            EventLog::instance().record("push: CAS retry for value=" + std::to_string(value));
        }

        EventLog::instance().record("SimpleLockFreeStack::push(" + std::to_string(value) + ")");
    }

    bool pop(int& value)
    {
        Node* old_head = head_.load(std::memory_order_acquire);

        while (old_head != nullptr)
        {
            if (head_.compare_exchange_weak(old_head, old_head->next,
                                            std::memory_order_release,
                                            std::memory_order_acquire))
            {
                value = old_head->value;
                EventLog::instance().record("SimpleLockFreeStack::pop(" + std::to_string(value) + ")");
                delete old_head;
                return true;
            }
            EventLog::instance().record("pop: CAS retry");
        }

        return false;
    }

    ~SimpleLockFreeStack()
    {
        int value;
        while (pop(value)) {}
    }

private:
    std::atomic<Node*> head_;
};

TEST_F(LockFreeBasicsTest, CompareExchangeBasics)
{
    SimpleLockFreeStack stack;

    constexpr int num_threads = 5;
    constexpr int items_per_thread = 10;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&stack, i, items_per_thread]()
        {
            for (int j = 0; j < items_per_thread; ++j)
            {
                stack.push(i * 100 + j);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: When compare_exchange_weak fails, new_node->next is updated with the current
    // Q: head value. Why is this update necessary before retrying?
    // A:
    // R:

    // Q: What memory ordering does compare_exchange use for the load when it fails?
    // Q: Why is memory_order_relaxed sufficient for the failure case?
    // A:
    // R:

    int popped_count = 0;
    int value;
    while (stack.pop(value))
    {
        popped_count++;
    }

    EXPECT_EQ(popped_count, num_threads * items_per_thread);
    EXPECT_GE(EventLog::instance().count_events("CAS retry"), 0);
}

// ============================================================================
// TEST 3: Memory Ordering: Acquire-Release Semantics - Hard
// ============================================================================

std::atomic<int> data{0};
std::atomic<bool> ready{false};

void producer_thread()
{
    data.store(42, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release);
    EventLog::instance().record("Producer: stored data and set ready");
}

void consumer_thread()
{
    while (!ready.load(std::memory_order_acquire)) {}
    int value = data.load(std::memory_order_relaxed);
    EventLog::instance().record("Consumer: read data=" + std::to_string(value));
    EXPECT_EQ(value, 42);
}

TEST_F(LockFreeBasicsTest, AcquireReleaseSemantics)
{
    data.store(0, std::memory_order_relaxed);
    ready.store(false, std::memory_order_relaxed);

    std::thread producer(producer_thread);
    std::thread consumer(consumer_thread);

    producer.join();
    consumer.join();

    // Q: The producer writes data with memory_order_relaxed, then sets ready with
    // Q: memory_order_release. What happens-before relationship does this establish?
    // A:
    // R:

    // Q: If both stores used memory_order_relaxed, could the consumer see ready==true
    // Q: but data==0? Why or why not?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Producer:"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Consumer:"), 1);
}

// ============================================================================
// TEST 4: TODO - Implement Lock-Free Stack with Proper Memory Reclamation - Hard
// ============================================================================

// TODO: The SimpleLockFreeStack has a critical bug: pop() deletes nodes immediately,
// TODO: but other threads may still be accessing them. Implement a safe version using:
// TODO: 1. Reference counting (std::atomic<int> ref_count in Node)
// TODO: 2. Hazard pointers (simplified version)
// TODO: 3. Or defer deletion until all threads finish (collect in a list)

class SafeLockFreeStack
{
public:
    SafeLockFreeStack() : head_(nullptr) {}

    void push(int value)
    {
        // TODO: Implement thread-safe push
    }

    bool pop(int& value)
    {
        // TODO: Implement thread-safe pop with safe memory reclamation
        return false;
    }

    ~SafeLockFreeStack()
    {
        // TODO: Clean up remaining nodes
    }

private:
    std::atomic<Node*> head_;
};

TEST_F(LockFreeBasicsTest, DISABLED_SafeLockFreeStackMemoryReclamation)
{
    SafeLockFreeStack stack;

    constexpr int num_threads = 10;
    constexpr int items_per_thread = 100;

    std::vector<std::thread> pushers;
    for (int i = 0; i < num_threads; ++i)
    {
        pushers.emplace_back([&stack, i, items_per_thread]()
        {
            for (int j = 0; j < items_per_thread; ++j)
            {
                stack.push(i * 1000 + j);
            }
        });
    }

    std::vector<std::thread> poppers;
    for (int i = 0; i < num_threads; ++i)
    {
        poppers.emplace_back([&stack, items_per_thread]()
        {
            int value;
            for (int j = 0; j < items_per_thread; ++j)
            {
                while (!stack.pop(value)) {}
            }
        });
    }

    for (auto& t : pushers)
    {
        t.join();
    }

    for (auto& t : poppers)
    {
        t.join();
    }

    // Q: Why can't you simply delete the node immediately after CAS succeeds in pop()?
    // Q: What race condition occurs with concurrent pop() operations?
    // A:
    // R:
}

// ============================================================================
// TEST 5: Atomic Flag and Spinlock - Moderate
// ============================================================================

class Spinlock
{
public:
    void lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire))
        {
        }
        EventLog::instance().record("Spinlock::lock() acquired");
    }

    void unlock()
    {
        EventLog::instance().record("Spinlock::unlock() released");
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

TEST_F(LockFreeBasicsTest, SpinlockBasics)
{
    Spinlock spinlock;
    int shared_value = 0;
    constexpr int num_threads = 5;
    constexpr int increments_per_thread = 100;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&spinlock, &shared_value, increments_per_thread]()
        {
            for (int j = 0; j < increments_per_thread; ++j)
            {
                spinlock.lock();
                ++shared_value;
                spinlock.unlock();
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: How does test_and_set() achieve mutual exclusion? What value does it return
    // Q: when the lock is already held vs when it's free?
    // A:
    // R:

    // Q: Why use memory_order_acquire in lock() and memory_order_release in unlock()?
    // Q: What synchronization do they provide?
    // A:
    // R:

    EXPECT_EQ(shared_value, num_threads * increments_per_thread);
    EXPECT_EQ(EventLog::instance().count_events("acquired"), num_threads * increments_per_thread);
}

// ============================================================================
// TEST 6: Sequential Consistency vs Relaxed Ordering - Hard
// ============================================================================

std::atomic<int> x{0};
std::atomic<int> y{0};
std::atomic<int> r1{0};
std::atomic<int> r2{0};

TEST_F(LockFreeBasicsTest, SequentialConsistencyVsRelaxed)
{
    constexpr int iterations = 100000;
    int anomalies = 0;

    for (int iter = 0; iter < iterations; ++iter)
    {
        x.store(0, std::memory_order_relaxed);
        y.store(0, std::memory_order_relaxed);
        r1.store(0, std::memory_order_relaxed);
        r2.store(0, std::memory_order_relaxed);

        std::thread t1([&]()
        {
            x.store(1, std::memory_order_relaxed);
            r1.store(y.load(std::memory_order_relaxed), std::memory_order_relaxed);
        });

        std::thread t2([&]()
        {
            y.store(1, std::memory_order_relaxed);
            r2.store(x.load(std::memory_order_relaxed), std::memory_order_relaxed);
        });

        t1.join();
        t2.join();

        if (r1.load() == 0 && r2.load() == 0)
        {
            anomalies++;
        }
    }

    // Q: With memory_order_relaxed, is it possible for both r1 and r2 to be 0?
    // Q: What reordering would cause this outcome?
    // A:
    // R:

    // Q: If all operations used memory_order_seq_cst instead, would this anomaly
    // Q: still be possible? What guarantee does seq_cst provide?
    // A:
    // R:

    EventLog::instance().record("Anomalies detected: " + std::to_string(anomalies));
    if (anomalies > 0)
    {
        EventLog::instance().record("Memory reordering observed with relaxed ordering");
    }
    else
    {
        EventLog::instance().record("No anomalies detected (hardware may have strong ordering)");
    }
}
