// Test Suite: Reader-Writer Locks and shared_mutex
// Estimated Time: 4 hours
// Difficulty: Moderate
// C++ Standard: C++17

#include <gtest/gtest.h>
#include "instrumentation.h"
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>

class ReaderWriterLocksTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic shared_mutex with Multiple Readers - Easy
// ============================================================================

class SharedCounter
{
public:
    SharedCounter() : value_(0) {}

    int read() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        EventLog::instance().record("SharedCounter::read() acquired shared_lock");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return value_;
    }

    void write(int val)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        EventLog::instance().record("SharedCounter::write() acquired unique_lock");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        value_ = val;
    }

    int value() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return value_;
    }

private:
    mutable std::shared_mutex mutex_;
    int value_;
};

TEST_F(ReaderWriterLocksTest, MultipleReadersNoContention)
{
    SharedCounter counter;
    counter.write(42);
    EventLog::instance().clear();

    constexpr int num_readers = 5;
    std::vector<std::thread> threads;
    std::atomic<int> ready_count{0};
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < num_readers; ++i)
    {
        threads.emplace_back([&counter, &ready_count]()
        {
            ready_count.fetch_add(1);
            while (ready_count.load() < num_readers) {}
            int val = counter.read();
            EXPECT_EQ(val, 42);
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    // Q: If readers held exclusive locks, 5 readers × 10ms sleep = 50ms minimum.
    // Q: With shared_lock, what is the expected minimum elapsed time? Why?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("acquired shared_lock"), num_readers);
    EXPECT_LT(elapsed, 30);
}

// ============================================================================
// TEST 2: Writer Blocks Readers - Moderate
// ============================================================================

TEST_F(ReaderWriterLocksTest, WriterBlocksReaders)
{
    SharedCounter counter;
    std::atomic<bool> writer_started{false};
    std::atomic<bool> writer_finished{false};
    std::atomic<int> readers_blocked{0};

    std::thread writer([&counter, &writer_started, &writer_finished]()
    {
        writer_started.store(true);
        counter.write(100);
        writer_finished.store(true);
    });

    while (!writer_started.load()) {}
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    constexpr int num_readers = 3;
    std::vector<std::thread> readers;

    for (int i = 0; i < num_readers; ++i)
    {
        readers.emplace_back([&counter, &writer_finished, &readers_blocked]()
        {
            readers_blocked.fetch_add(1);
            int val = counter.read();
            EXPECT_TRUE(writer_finished.load());
            EXPECT_EQ(val, 100);
        });
    }

    writer.join();
    for (auto& r : readers)
    {
        r.join();
    }

    // Q: Why do readers see writer_finished == true? What synchronization does
    // Q: unique_lock provide when it releases the mutex?
    // A:
    // R:

    EXPECT_EQ(readers_blocked.load(), num_readers);
    EXPECT_EQ(counter.value(), 100);
}

// ============================================================================
// TEST 3: Reader Blocks Writer - Moderate
// ============================================================================

TEST_F(ReaderWriterLocksTest, ReaderBlocksWriter)
{
    SharedCounter counter;
    counter.write(50);
    EventLog::instance().clear();

    std::atomic<bool> reader_started{false};
    std::atomic<bool> reader_finished{false};
    std::atomic<bool> writer_blocked{false};

    std::thread reader([&counter, &reader_started, &reader_finished]()
    {
        reader_started.store(true);
        int val = counter.read();
        EXPECT_EQ(val, 50);
        reader_finished.store(true);
    });

    while (!reader_started.load()) {}
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    std::thread writer([&counter, &writer_blocked, &reader_finished]()
    {
        writer_blocked.store(true);
        counter.write(200);
        EXPECT_TRUE(reader_finished.load());
    });

    reader.join();
    writer.join();

    // Q: The writer sets writer_blocked = true before calling write(). Why does the
    // Q: writer see reader_finished == true inside write()? What lock ordering occurred?
    // A:
    // R:

    EXPECT_TRUE(writer_blocked.load());
    EXPECT_EQ(counter.value(), 200);
}

// ============================================================================
// TEST 4: TODO - Implement Upgradable Lock Pattern - Hard
// ============================================================================

// TODO: Implement a cache that supports:
// TODO: 1. Multiple concurrent readers (shared_lock)
// TODO: 2. Exclusive writers (unique_lock)
// TODO: 3. Upgrade from reader to writer when cache miss occurs
// TODO: Note: C++17 shared_mutex does NOT support lock upgrading directly
// TODO: You must release the shared_lock and acquire unique_lock (potential race)

class UpgradableCache
{
public:
    UpgradableCache() : value_(0) {}

    int get_or_compute(int key)
    {
        // TODO: First acquire shared_lock and check if value exists
        // TODO: If miss, release shared_lock and acquire unique_lock
        // TODO: Check again (another thread may have computed it)
        // TODO: If still missing, compute and store
        return 0;
    }

private:
    mutable std::shared_mutex mutex_;
    int value_;
};

TEST_F(ReaderWriterLocksTest, DISABLED_UpgradableLockPattern)
{
    UpgradableCache cache;

    constexpr int num_threads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> compute_count{0};

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&cache, &compute_count]()
        {
            int val = cache.get_or_compute(1);
            EXPECT_EQ(val, 1);
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    // Q: After releasing shared_lock and before acquiring unique_lock, another thread
    // Q: may compute the value. Why must you check again after acquiring unique_lock?
    // A:
    // R:
}

// ============================================================================
// TEST 5: Reader-Writer Starvation - Hard
// ============================================================================

class StarvationDemo
{
public:
    StarvationDemo() : value_(0) {}

    int read() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        EventLog::instance().record("StarvationDemo::read()");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return value_;
    }

    void write(int val)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        EventLog::instance().record("StarvationDemo::write()");
        value_ = val;
    }

private:
    mutable std::shared_mutex mutex_;
    int value_;
};

TEST_F(ReaderWriterLocksTest, ReaderWriterStarvation)
{
    StarvationDemo demo;
    std::atomic<bool> stop{false};
    std::atomic<int> write_count{0};

    constexpr int num_readers = 10;
    std::vector<std::thread> readers;

    for (int i = 0; i < num_readers; ++i)
    {
        readers.emplace_back([&demo, &stop]()
        {
            while (!stop.load())
            {
                demo.read();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::thread writer([&demo, &write_count]()
    {
        for (int i = 0; i < 5; ++i)
        {
            demo.write(i);
            write_count.fetch_add(1);
        }
    });

    writer.join();
    stop.store(true);

    for (auto& r : readers)
    {
        r.join();
    }

    // Q: In this scenario, continuous readers may delay the writer. Does std::shared_mutex
    // Q: guarantee writer priority, or can readers starve writers? What does the standard say?
    // A:
    // R:

    EXPECT_EQ(write_count.load(), 5);
    EXPECT_GT(EventLog::instance().count_events("StarvationDemo::read()"), 0);
}

// ============================================================================
// TEST 6: Shared Lock RAII and Exception Safety - Moderate
// ============================================================================

class ExceptionSafeReader
{
public:
    ExceptionSafeReader() : value_(0) {}

    int read_with_exception() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        EventLog::instance().record("ExceptionSafeReader::read() acquired lock");
        throw std::runtime_error("Simulated exception");
        return value_;
    }

    void write(int val)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        EventLog::instance().record("ExceptionSafeReader::write() acquired lock");
        value_ = val;
    }

private:
    mutable std::shared_mutex mutex_;
    int value_;
};

TEST_F(ReaderWriterLocksTest, SharedLockExceptionSafety)
{
    ExceptionSafeReader reader;

    std::thread t1([&reader]()
    {
        try
        {
            reader.read_with_exception();
        }
        catch (const std::runtime_error&)
        {
            EventLog::instance().record("Exception caught in thread");
        }
    });

    t1.join();

    std::thread t2([&reader]()
    {
        reader.write(100);
    });

    t2.join();

    // Q: After the exception is thrown in read_with_exception(), the shared_lock goes
    // Q: out of scope. What RAII guarantee ensures the mutex is released?
    // A:
    // R:

    // Q: If the lock were NOT released, what would happen when the writer tries to
    // Q: acquire unique_lock? What observable signal would you see?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("acquired lock"), 2);
    EXPECT_EQ(EventLog::instance().count_events("Exception caught"), 1);
}
