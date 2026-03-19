// Test Suite: Thread-Safe Singleton Patterns
// Estimated Time: 3 hours
// Difficulty: Moderate
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>

class ThreadSafeSingletonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Meyers Singleton (Magic Statics) - Easy
// ============================================================================

class MeyersSingleton
{
public:
    static MeyersSingleton& instance()
    {
        static MeyersSingleton inst;
        return inst;
    }

    void do_work()
    {
        EventLog::instance().record("MeyersSingleton::do_work() called");
    }

    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;

private:
    MeyersSingleton()
    {
        EventLog::instance().record("MeyersSingleton::ctor");
    }

    ~MeyersSingleton()
    {
        EventLog::instance().record("MeyersSingleton::dtor");
    }
};

TEST_F(ThreadSafeSingletonTest, MeyersSingletonThreadSafe)
{
    constexpr int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> ready_count{0};

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&ready_count]()
        {
            ready_count.fetch_add(1);
            while (ready_count.load() < num_threads) {}
            MeyersSingleton::instance().do_work();
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: How many times is the MeyersSingleton constructor called when 10 threads
    // Q: simultaneously access instance()? What C++11 guarantee ensures thread safety?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("MeyersSingleton::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("MeyersSingleton::do_work()"), num_threads);
}

// ============================================================================
// TEST 2: Double-Checked Locking Anti-Pattern - Moderate
// ============================================================================

class BrokenDoubleCheckedSingleton
{
public:
    static BrokenDoubleCheckedSingleton* instance()
    {
        if (inst_ == nullptr)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (inst_ == nullptr)
            {
                inst_ = new BrokenDoubleCheckedSingleton();
            }
        }
        return inst_;
    }

    void do_work()
    {
        EventLog::instance().record("BrokenDCL::do_work()");
    }

    static void reset()
    {
        delete inst_;
        inst_ = nullptr;
    }

private:
    BrokenDoubleCheckedSingleton()
    {
        EventLog::instance().record("BrokenDCL::ctor");
    }

    static BrokenDoubleCheckedSingleton* inst_;
    static std::mutex mutex_;
};

BrokenDoubleCheckedSingleton* BrokenDoubleCheckedSingleton::inst_ = nullptr;
std::mutex BrokenDoubleCheckedSingleton::mutex_;

TEST_F(ThreadSafeSingletonTest, BrokenDoubleCheckedLocking)
{
    BrokenDoubleCheckedSingleton::reset();
    EventLog::instance().clear();

    // Q: The outer `if (inst_ == nullptr)` check is unprotected. What memory ordering
    // Q: issue can occur between the write `inst_ = new ...` in one thread and the
    // Q: read `inst_ == nullptr` in another thread?
    // A:
    // R:

    // Q: Even if the pointer becomes visible, what can go wrong with the object's
    // Q: member variables when another thread dereferences the pointer?
    // A:
    // R:

    constexpr int num_threads = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([]()
        {
            BrokenDoubleCheckedSingleton::instance()->do_work();
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(EventLog::instance().count_events("BrokenDCL::ctor"), 1);
}

// ============================================================================
// TEST 3: Correct Double-Checked Locking with std::atomic - Moderate
// ============================================================================

class CorrectDoubleCheckedSingleton
{
public:
    static CorrectDoubleCheckedSingleton* instance()
    {
        CorrectDoubleCheckedSingleton* tmp = inst_.load(std::memory_order_acquire);
        if (tmp == nullptr)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tmp = inst_.load(std::memory_order_relaxed);
            if (tmp == nullptr)
            {
                tmp = new CorrectDoubleCheckedSingleton();
                inst_.store(tmp, std::memory_order_release);
            }
        }
        return tmp;
    }

    void do_work()
    {
        EventLog::instance().record("CorrectDCL::do_work()");
    }

    static void reset()
    {
        delete inst_.load();
        inst_.store(nullptr);
    }

private:
    CorrectDoubleCheckedSingleton()
    {
        EventLog::instance().record("CorrectDCL::ctor");
    }

    static std::atomic<CorrectDoubleCheckedSingleton*> inst_;
    static std::mutex mutex_;
};

std::atomic<CorrectDoubleCheckedSingleton*> CorrectDoubleCheckedSingleton::inst_{nullptr};
std::mutex CorrectDoubleCheckedSingleton::mutex_;

TEST_F(ThreadSafeSingletonTest, CorrectDoubleCheckedLocking)
{
    CorrectDoubleCheckedSingleton::reset();
    EventLog::instance().clear();

    constexpr int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> ready_count{0};

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&ready_count]()
        {
            ready_count.fetch_add(1);
            while (ready_count.load() < num_threads) {}
            CorrectDoubleCheckedSingleton::instance()->do_work();
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: Why is memory_order_acquire used for the first load and memory_order_release
    // Q: for the store? What happens-before relationship do they establish?
    // A:
    // R:

    // Q: Why can the second load (inside the lock) use memory_order_relaxed?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("CorrectDCL::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("CorrectDCL::do_work()"), num_threads);
}

// ============================================================================
// TEST 4: std::call_once Singleton - Easy
// ============================================================================

class CallOnceSingleton
{
public:
    static CallOnceSingleton& instance()
    {
        static CallOnceSingleton inst;
        std::call_once(init_flag_, []()
        {
            EventLog::instance().record("CallOnceSingleton::init via call_once");
        });
        return inst;
    }

    void do_work()
    {
        EventLog::instance().record("CallOnceSingleton::do_work()");
    }

    CallOnceSingleton(const CallOnceSingleton&) = delete;
    CallOnceSingleton& operator=(const CallOnceSingleton&) = delete;

    static void reset()
    {
    }

private:
    CallOnceSingleton()
    {
        EventLog::instance().record("CallOnceSingleton::ctor");
    }

    ~CallOnceSingleton()
    {
        EventLog::instance().record("CallOnceSingleton::dtor");
    }

    static std::once_flag init_flag_;
};

std::once_flag CallOnceSingleton::init_flag_;

TEST_F(ThreadSafeSingletonTest, CallOnceSingleton)
{
    CallOnceSingleton::reset();
    EventLog::instance().clear();

    constexpr int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> ready_count{0};

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&ready_count]()
        {
            ready_count.fetch_add(1);
            while (ready_count.load() < num_threads) {}
            CallOnceSingleton::instance().do_work();
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: What advantage does std::call_once provide over Meyers singleton?
    // Q: When would you prefer call_once over magic statics?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("CallOnceSingleton::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("CallOnceSingleton::do_work()"), num_threads);
}

// ============================================================================
// TEST 5: Lazy Initialization Race - Moderate
// ============================================================================

class LazyInitRace
{
public:
    static LazyInitRace* instance()
    {
        if (inst_ == nullptr)
        {
            inst_ = new LazyInitRace();
        }
        return inst_;
    }

    void do_work()
    {
        EventLog::instance().record("LazyInitRace::do_work()");
    }

    static void reset()
    {
        delete inst_;
        inst_ = nullptr;
    }

private:
    LazyInitRace()
    {
        EventLog::instance().record("LazyInitRace::ctor");
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    static LazyInitRace* inst_;
};

LazyInitRace* LazyInitRace::inst_ = nullptr;

TEST_F(ThreadSafeSingletonTest, LazyInitializationRace)
{
    LazyInitRace::reset();
    EventLog::instance().clear();

    constexpr int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> ready_count{0};

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&ready_count]()
        {
            ready_count.fetch_add(1);
            while (ready_count.load() < num_threads) {}
            LazyInitRace::instance()->do_work();
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: The constructor includes a deliberate sleep. What race condition does this
    // Q: expose? What observable signal in EventLog would confirm multiple constructions?
    // A:
    // R:

    size_t ctor_count = EventLog::instance().count_events("LazyInitRace::ctor");
    EXPECT_GT(ctor_count, 1);
}

// ============================================================================
// TEST 6: TODO - Implement Thread-Safe Initialization with std::atomic - Hard
// ============================================================================

// TODO: Implement a singleton using std::atomic<T*> with proper memory ordering
// TODO: Use acquire-release semantics to ensure visibility
// TODO: Handle the race between multiple threads trying to construct
// TODO: Ensure exactly one construction occurs

class AtomicSingleton
{
public:
    static AtomicSingleton* instance()
    {
        // TODO: Implement thread-safe lazy initialization using std::atomic
        // TODO: Use memory_order_acquire for loads
        // TODO: Use memory_order_release for stores
        // TODO: Use compare_exchange_strong to handle races
        return nullptr;
    }

    void do_work()
    {
        EventLog::instance().record("AtomicSingleton::do_work()");
    }

    static void reset()
    {
        delete inst_.load();
        inst_.store(nullptr);
    }

private:
    AtomicSingleton()
    {
        EventLog::instance().record("AtomicSingleton::ctor");
    }

    static std::atomic<AtomicSingleton*> inst_;
};

std::atomic<AtomicSingleton*> AtomicSingleton::inst_{nullptr};

TEST_F(ThreadSafeSingletonTest, DISABLED_AtomicSingletonCorrectness)
{
    AtomicSingleton::reset();
    EventLog::instance().clear();

    constexpr int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> ready_count{0};

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&ready_count]()
        {
            ready_count.fetch_add(1);
            while (ready_count.load() < num_threads) {}
            AtomicSingleton::instance()->do_work();
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: After implementing compare_exchange_strong, what happens to the losing threads'
    // Q: allocated objects? How do you prevent memory leaks?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("AtomicSingleton::ctor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("AtomicSingleton::do_work()"), num_threads);
}
