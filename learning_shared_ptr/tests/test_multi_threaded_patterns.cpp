#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <asio.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <vector>
#include <sstream>

// C++11 compatible latch implementation
class CountDownLatch
{
public:
    explicit CountDownLatch(int count)
    : count_(count)
    {
    }
    
    void arrive_and_wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (--count_ == 0)
        {
            cv_.notify_all();
        }
        else
        {
            cv_.wait(lock, [this]() { return count_ == 0; });
        }
    }
    
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int count_;
};

// C++11 compatible barrier implementation
class Barrier
{
public:
    explicit Barrier(int count)
    : threshold_(count)
    , count_(count)
    , generation_(0)
    {
    }
    
    void arrive_and_wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        int gen = generation_;
        
        if (--count_ == 0)
        {
            ++generation_;
            count_ = threshold_;
            cv_.notify_all();
        }
        else
        {
            cv_.wait(lock, [this, gen]() { return gen != generation_; });
        }
    }
    
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int threshold_;
    int count_;
    int generation_;
};

// Thread-safe EventLog wrapper for multi-threaded tests
class ThreadSafeEventLog
{
public:
    static ThreadSafeEventLog& instance()
    {
        static ThreadSafeEventLog instance;
        return instance;
    }
    
    void record(const std::string& event)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.push_back(event);
    }
    
    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.clear();
    }
    
    std::vector<std::string> events() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_;
    }
    
    size_t count(const std::string& pattern) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t count = 0;
        for (const auto& event : events_)
        {
            if (event.find(pattern) != std::string::npos)
            {
                ++count;
            }
        }
        return count;
    }
    
private:
    ThreadSafeEventLog() = default;
    mutable std::mutex mutex_;
    std::vector<std::string> events_;
};

class MultiThreadedPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
        ThreadSafeEventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Ownership Transfer Across Threads (io_context)
// ============================================================================

class AsyncWorker : public std::enable_shared_from_this<AsyncWorker>
{
public:
    explicit AsyncWorker(const std::string& name)
    : tracked_(name)
    {
    }
    
    void do_work_safe(asio::io_context& io)
    {
        // Safe: shared_from_this() keeps object alive during async operation
        asio::post(io, std::bind(&AsyncWorker::work_impl, shared_from_this()));
    }
    
    void do_work_unsafe(asio::io_context& io)
    {
        // UNSAFE: Using 'this' - object might be destroyed before callback runs
        asio::post(io, std::bind(&AsyncWorker::work_impl, this));
    }
    
    std::string name() const
    {
        return tracked_.name();
    }
    
private:
    void work_impl()
    {
        ThreadSafeEventLog::instance().record("AsyncWorker::work_impl executed");
    }
    
    Tracked tracked_;
};

TEST_F(MultiThreadedPatternsTest, OwnershipTransferSafeWithSharedFromThis)
{
    asio::io_context io;
    std::atomic<long> use_count_in_callback{0};
    
    {
        std::shared_ptr<AsyncWorker> worker = std::make_shared<AsyncWorker>("Worker1");
        
        // Q: What is the use_count before posting work?
        // A:
        // R:
        
        // Post work with shared_from_this()
        asio::post(io, [worker, &use_count_in_callback]()
        {
            use_count_in_callback = worker.use_count();
            ThreadSafeEventLog::instance().record("Lambda executed with worker alive");
        });
        
        // Q: What happens to the worker object when this scope ends?
        // A:
        // R:
        
        // Q: Why is the worker object still alive when the callback executes?
        // A:
        // R:
    }
    
    // Q: At this point (before io.run()), is the worker object destroyed?
    // A:
    // R:
    
    io.run();
    
    // Q: What is the use_count observed inside the callback, and why?
    // A:
    // R:
    
    auto events = ThreadSafeEventLog::instance().events();
    EXPECT_EQ(events.size(), 1);
    EXPECT_EQ(use_count_in_callback, 1);
}

TEST_F(MultiThreadedPatternsTest, OwnershipTransferUnsafeWithRawThis)
{
    asio::io_context io;
    bool callback_executed = false;
    
    {
        std::shared_ptr<AsyncWorker> worker = std::make_shared<AsyncWorker>("Worker2");
        
        // ANTI-PATTERN: Using raw 'this' pointer
        asio::post(io, [worker_ptr = worker.get(), &callback_executed]()
        {
            // Q: What is the danger of using worker_ptr here?
            // A:
            // R:
            
            callback_executed = true;
            // Accessing worker_ptr->name() here would be undefined behavior if object destroyed
        });
        
        // Q: What happens when worker goes out of scope?
        // A:
        // R:
    }
    
    // Q: Is it safe to run io.run() now? Why or why not?
    // A:
    // R:
    
    io.run();
    
    EXPECT_TRUE(callback_executed);
}

// ============================================================================
// TEST 2: Reference Counting Across Threads (Atomic Operations)
// ============================================================================

TEST_F(MultiThreadedPatternsTest, AtomicUseCountIncrementAcrossThreads)
{
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("Shared");
    std::vector<long> use_counts;
    std::mutex counts_mutex;
    
    // Q: Why is the use_count increment/decrement atomic in shared_ptr?
    // A:
    // R:
    
    CountDownLatch start_latch(4);  // Synchronize thread start
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&shared, &use_counts, &counts_mutex, &start_latch]()
        {
            start_latch.arrive_and_wait();  // All threads start simultaneously
            
            std::shared_ptr<Tracked> local_copy = shared;
            
            // Q: What happens to the control block's use_count when local_copy is created?
            // A:
            // R:
            
            {
                std::lock_guard<std::mutex> lock(counts_mutex);
                use_counts.push_back(local_copy.use_count());
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: What is the maximum use_count observed across all threads?
    // A:
    // R:
    
    // Q: Why don't we see race conditions in use_count despite concurrent modifications?
    // A:
    // R:
    
    EXPECT_GE(*std::max_element(use_counts.begin(), use_counts.end()), 4);
}

TEST_F(MultiThreadedPatternsTest, ParallelCopyAndMoveOperations)
{
    std::shared_ptr<Tracked> source = std::make_shared<Tracked>("Source");
    std::atomic<int> copy_count{0};
    std::atomic<int> move_count{0};
    
    Barrier sync_point(4);
    std::vector<std::thread> threads;
    
    // Two threads copy, two threads move
    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&, i]()
        {
            sync_point.arrive_and_wait();
            
            if (i < 2)
            {
                // Copy operation
                std::shared_ptr<Tracked> copy = source;
                ++copy_count;
                
                // Q: What atomic operation occurs in the control block during copy?
                // A:
                // R:
            }
            else
            {
                // Move operation
                std::shared_ptr<Tracked> temp = source;
                std::shared_ptr<Tracked> moved = std::move(temp);
                ++move_count;
                
                // Q: What happens to the control block's use_count during move?
                // A:
                // R:
            }
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: Why is move faster than copy in multi-threaded context?
    // A:
    // R:
    
    EXPECT_EQ(copy_count, 2);
    EXPECT_EQ(move_count, 2);
    EXPECT_EQ(source.use_count(), 1);
}

// ============================================================================
// TEST 3: Weak Callbacks and Expired Object Handling
// ============================================================================

class TimerCallback : public std::enable_shared_from_this<TimerCallback>
{
public:
    explicit TimerCallback(const std::string& name)
    : tracked_(name)
    {
    }
    
    void schedule_with_weak(asio::io_context& io, std::chrono::milliseconds delay)
    {
        std::weak_ptr<TimerCallback> weak_self = shared_from_this();
        
        asio::post(io, [weak_self, delay]()
        {
            std::this_thread::sleep_for(delay);
            
            if (std::shared_ptr<TimerCallback> self = weak_self.lock())
            {
                ThreadSafeEventLog::instance().record("Timer callback executed - object alive");
            }
            else
            {
                ThreadSafeEventLog::instance().record("Timer callback skipped - object expired");
            }
        });
    }
    
    void schedule_with_shared(asio::io_context& io, std::chrono::milliseconds delay)
    {
        std::shared_ptr<TimerCallback> shared_self = shared_from_this();
        
        asio::post(io, [shared_self, delay]()
        {
            std::this_thread::sleep_for(delay);
            ThreadSafeEventLog::instance().record("Timer callback executed - object kept alive");
        });
    }
    
private:
    Tracked tracked_;
};

TEST_F(MultiThreadedPatternsTest, WeakCallbackExpiresBeforeExecution)
{
    asio::io_context io;
    std::thread io_thread;
    
    {
        std::shared_ptr<TimerCallback> timer = std::make_shared<TimerCallback>("Timer1");
        
        // Q: Why use weak_ptr in the callback instead of shared_ptr?
        // A:
        // R:
        
        timer->schedule_with_weak(io, std::chrono::milliseconds(50));
        
        // Start io_context in separate thread
        io_thread = std::thread([&io]() { io.run(); });
        
        // Q: What happens when timer goes out of scope immediately?
        // A:
        // R:
    }
    
    // Q: When the callback executes, what does weak_ptr::lock() return?
    // A:
    // R:
    
    io_thread.join();
    
    auto events = ThreadSafeEventLog::instance().events();
    EXPECT_EQ(ThreadSafeEventLog::instance().count("skipped - object expired"), 1);
}

TEST_F(MultiThreadedPatternsTest, SharedCallbackKeepsObjectAlive)
{
    asio::io_context io;
    std::thread io_thread;
    
    {
        std::shared_ptr<TimerCallback> timer = std::make_shared<TimerCallback>("Timer2");
        timer->schedule_with_shared(io, std::chrono::milliseconds(50));
        
        io_thread = std::thread([&io]() { io.run(); });
        
        // Q: Even though timer goes out of scope, why isn't the object destroyed?
        // A:
        // R:
    }
    
    // Q: What is the lifetime of the TimerCallback object in this scenario?
    // A:
    // R:
    
    io_thread.join();
    
    auto events = ThreadSafeEventLog::instance().events();
    EXPECT_EQ(ThreadSafeEventLog::instance().count("kept alive"), 1);
}

// ============================================================================
// TEST 4: Thread-Safe Cache with weak_ptr
// ============================================================================

class ThreadSafeCache
{
public:
    std::shared_ptr<Tracked> get_or_create(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        if (it != cache_.end())
        {
            if (std::shared_ptr<Tracked> cached = it->second.lock())
            {
                ThreadSafeEventLog::instance().record("Cache hit: " + key);
                return cached;
            }
        }
        
        // Q: Why is lock() called inside the mutex-protected section?
        // A:
        // R:
        
        std::shared_ptr<Tracked> new_resource = std::make_shared<Tracked>(key);
        cache_[key] = new_resource;
        ThreadSafeEventLog::instance().record("Cache miss: " + key);
        return new_resource;
    }
    
    void cleanup()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto it = cache_.begin(); it != cache_.end();)
        {
            if (it->second.expired())
            {
                it = cache_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_.size();
    }
    
private:
    mutable std::mutex mutex_;
    std::map<std::string, std::weak_ptr<Tracked>> cache_;
};

TEST_F(MultiThreadedPatternsTest, ThreadSafeCacheWithWeakPtr)
{
    ThreadSafeCache cache;
    Barrier sync_point(4);
    std::vector<std::thread> threads;
    
    // Q: Why does the cache store weak_ptr instead of shared_ptr?
    // A:
    // R:
    
    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&cache, &sync_point, i]()
        {
            sync_point.arrive_and_wait();
            
            std::shared_ptr<Tracked> r1 = cache.get_or_create("resource1");
            std::shared_ptr<Tracked> r2 = cache.get_or_create("resource2");
            
            // Q: What prevents race conditions when multiple threads access the cache?
            // A:
            // R:
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: How many "Cache miss" events should be logged?
    // A:
    // R:
    
    EXPECT_EQ(ThreadSafeEventLog::instance().count("Cache hit"), 6);  // 4 threads * 2 resources - 2 initial misses
    EXPECT_EQ(ThreadSafeEventLog::instance().count("Cache miss"), 2);
}

// ============================================================================
// TEST 5: Producer-Consumer with shared_ptr and std::queue
// ============================================================================

template<typename T>
class ThreadSafeQueue
{
public:
    void push(std::shared_ptr<T> item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
        cv_.notify_one();
        
        // Q: What happens to the use_count when item is pushed onto the queue?
        // A:
        // R:
    }
    
    std::shared_ptr<T> pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty() || done_; });
        
        if (queue_.empty())
        {
            return nullptr;
        }
        
        std::shared_ptr<T> item = queue_.front();
        queue_.pop();
        
        // Q: What happens to the use_count when item is popped from the queue?
        // A:
        // R:
        
        return item;
    }
    
    void set_done()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        done_ = true;
        cv_.notify_all();
    }
    
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::shared_ptr<T>> queue_;
    bool done_ = false;
};

TEST_F(MultiThreadedPatternsTest, ProducerConsumerWithSharedPtr)
{
    ThreadSafeQueue<Tracked> queue;
    std::atomic<int> items_consumed{0};
    const int num_items = 10;
    
    // Producer thread
    std::thread producer([&queue, num_items]()
    {
        for (int i = 0; i < num_items; ++i)
        {
            std::shared_ptr<Tracked> item = std::make_shared<Tracked>("Item" + std::to_string(i));
            
            // Q: When does the Tracked object get destroyed?
            // A:
            // R:
            
            queue.push(item);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        queue.set_done();
    });
    
    // Consumer thread
    std::thread consumer([&queue, &items_consumed]()
    {
        while (true)
        {
            std::shared_ptr<Tracked> item = queue.pop();
            if (!item)
            {
                break;
            }
            
            ++items_consumed;
            
            // Q: What is the use_count of item at this point?
            // A:
            // R:
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    producer.join();
    consumer.join();
    
    // Q: Why is shared_ptr ownership transfer ideal for producer-consumer patterns?
    // A:
    // R:
    
    EXPECT_EQ(items_consumed, num_items);
}

// ============================================================================
// TEST 6: Destruction Order Across Threads
// ============================================================================

class DestructionTracker : public std::enable_shared_from_this<DestructionTracker>
{
public:
    explicit DestructionTracker(const std::string& name)
    : tracked_(name)
    , thread_id_(std::this_thread::get_id())
    {
        ThreadSafeEventLog::instance().record("Constructed in thread " + thread_id_string());
    }
    
    ~DestructionTracker()
    {
        ThreadSafeEventLog::instance().record("Destroyed in thread " + current_thread_id_string());
    }
    
    void do_async_work(asio::io_context& io)
    {
        asio::post(io, [self = shared_from_this()]()
        {
            ThreadSafeEventLog::instance().record("Work executed in thread " + self->current_thread_id_string());
        });
    }
    
private:
    std::string thread_id_string() const
    {
        std::ostringstream oss;
        oss << thread_id_;
        return oss.str();
    }
    
    std::string current_thread_id_string() const
    {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        return oss.str();
    }
    
    Tracked tracked_;
    std::thread::id thread_id_;
};

TEST_F(MultiThreadedPatternsTest, DestructionInDifferentThread)
{
    asio::io_context io;
    std::thread io_thread;
    
    {
        std::shared_ptr<DestructionTracker> tracker = std::make_shared<DestructionTracker>("Tracker1");
        
        // Q: In which thread is the DestructionTracker constructed?
        // A:
        // R:
        
        tracker->do_async_work(io);
        
        io_thread = std::thread([&io]() { io.run(); });
        
        // Q: When tracker goes out of scope here, is the object destroyed immediately?
        // A:
        // R:
    }
    
    io_thread.join();
    
    // Q: In which thread is the DestructionTracker destroyed, and why?
    // A:
    // R:
    
    auto events = ThreadSafeEventLog::instance().events();
    EXPECT_EQ(ThreadSafeEventLog::instance().count("Constructed"), 1);
    EXPECT_EQ(ThreadSafeEventLog::instance().count("Destroyed"), 1);
}

// ============================================================================
// TEST 7: Aliasing Constructor with Concurrent Access
// ============================================================================

struct Container
{
    Tracked member;
    std::mutex mutex;
    
    explicit Container(const std::string& name)
    : member(name)
    {
    }
};

TEST_F(MultiThreadedPatternsTest, AliasingWithConcurrentAccess)
{
    std::shared_ptr<Container> owner = std::make_shared<Container>("Container1");
    std::shared_ptr<Tracked> alias(owner, &owner->member);
    
    // Q: What is the use_count of both owner and alias?
    // A:
    // R:
    
    Barrier sync_point(3);
    std::vector<std::thread> threads;
    
    // Thread 1: Access through owner
    threads.emplace_back([&owner, &sync_point]()
    {
        sync_point.arrive_and_wait();
        std::lock_guard<std::mutex> lock(owner->mutex);
        ThreadSafeEventLog::instance().record("Accessed via owner: " + owner->member.name());
    });
    
    // Thread 2: Access through alias
    threads.emplace_back([&alias, &owner, &sync_point]()
    {
        sync_point.arrive_and_wait();
        std::lock_guard<std::mutex> lock(owner->mutex);
        ThreadSafeEventLog::instance().record("Accessed via alias: " + alias->name());
    });
    
    // Thread 3: Reset owner
    threads.emplace_back([&owner, &sync_point]()
    {
        sync_point.arrive_and_wait();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        owner.reset();
        
        // Q: After owner.reset(), is the Container object destroyed?
        // A:
        // R:
    });
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: Why is the Container still alive even after owner.reset()?
    // A:
    // R:
    
    // Q: What object gets deleted when alias goes out of scope?
    // A:
    // R:
    
    EXPECT_EQ(alias.use_count(), 1);
}

// ============================================================================
// TEST 8: Weak Pointer with Aliasing in Multi-threaded Context
// ============================================================================

TEST_F(MultiThreadedPatternsTest, WeakAliasingAcrossThreads)
{
    std::weak_ptr<Tracked> weak_alias;
    std::atomic<int> successful_locks{0};
    std::atomic<int> failed_locks{0};
    
    {
        std::shared_ptr<Container> owner = std::make_shared<Container>("Container2");
        std::shared_ptr<Tracked> alias(owner, &owner->member);
        weak_alias = alias;
        
        // Q: What does weak_alias observe - the Container or the Tracked member?
        // A:
        // R:
        
        Barrier sync_point(4);
        std::vector<std::thread> threads;
        
        for (int i = 0; i < 4; ++i)
        {
            threads.emplace_back([&weak_alias, &successful_locks, &failed_locks, &sync_point, i]()
            {
                sync_point.arrive_and_wait();
                
                std::this_thread::sleep_for(std::chrono::milliseconds(i * 10));
                
                if (std::shared_ptr<Tracked> locked = weak_alias.lock())
                {
                    ++successful_locks;
                    ThreadSafeEventLog::instance().record("Lock succeeded: " + locked->name());
                }
                else
                {
                    ++failed_locks;
                    ThreadSafeEventLog::instance().record("Lock failed - expired");
                }
            });
        }
        
        // Let first thread lock successfully, then destroy owner and alias
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        
        // Q: What happens to weak_alias when owner and alias go out of scope?
        // A:
        // R:
    }
    
    // Wait for all threads
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Q: Why do some threads successfully lock while others fail?
    // A:
    // R:
    
    EXPECT_GT(successful_locks, 0);
    EXPECT_GT(failed_locks, 0);
}

// ============================================================================
// TEST 9: Thread Pool with Ownership Transfer
// ============================================================================

class WorkItem : public std::enable_shared_from_this<WorkItem>
{
public:
    explicit WorkItem(int id)
    : tracked_("Work" + std::to_string(id))
    , id_(id)
    {
    }
    
    void execute()
    {
        ThreadSafeEventLog::instance().record("WorkItem " + std::to_string(id_) + " executed");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    int id() const { return id_; }
    
private:
    Tracked tracked_;
    int id_;
};

TEST_F(MultiThreadedPatternsTest, ThreadPoolOwnershipTransfer)
{
    asio::thread_pool pool(4);
    std::atomic<int> completed{0};
    
    // Q: What is the advantage of using asio::thread_pool over asio::io_context?
    // A:
    // R:
    
    for (int i = 0; i < 20; ++i)
    {
        std::shared_ptr<WorkItem> work = std::make_shared<WorkItem>(i);
        
        // Q: What happens to work's use_count when posted to the thread pool?
        // A:
        // R:
        
        asio::post(pool, [work, &completed]()
        {
            work->execute();
            ++completed;
        });
    }
    
    pool.join();
    
    // Q: When are the WorkItem objects destroyed?
    // A:
    // R:
    
    EXPECT_EQ(completed, 20);
}

// ============================================================================
// TEST 10: Race Condition Detection with shared_ptr
// ============================================================================

class RaceConditionDemo
{
public:
    explicit RaceConditionDemo(const std::string& name)
    : tracked_(name)
    , counter_(0)
    {
    }
    
    void increment_unsafe()
    {
        // ANTI-PATTERN: No synchronization
        int temp = counter_;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        counter_ = temp + 1;
    }
    
    void increment_safe()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++counter_;
    }
    
    int counter() const
    {
        return counter_;
    }
    
private:
    Tracked tracked_;
    int counter_;
    std::mutex mutex_;
};

TEST_F(MultiThreadedPatternsTest, SharedPtrDoesNotPreventDataRaces)
{
    std::shared_ptr<RaceConditionDemo> shared = std::make_shared<RaceConditionDemo>("RaceDemo");
    
    // Q: Does shared_ptr protect against data races on the contained object?
    // A:
    // R:
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([shared]()
        {
            for (int j = 0; j < 100; ++j)
            {
                shared->increment_unsafe();
            }
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: Why is the final counter value likely less than 1000?
    // A:
    // R:
    
    // Q: What does shared_ptr's atomicity guarantee, and what doesn't it guarantee?
    // A:
    // R:
    
    EXPECT_LT(shared->counter(), 1000);  // Race condition causes lost updates
}

// ============================================================================
// TEST 11: Singleton with Thread-Safe Initialization
// ============================================================================

class ThreadSafeSingleton : public std::enable_shared_from_this<ThreadSafeSingleton>
{
public:
    static std::shared_ptr<ThreadSafeSingleton> instance()
    {
        std::call_once(init_flag_, []()
        {
            instance_ = std::shared_ptr<ThreadSafeSingleton>(new ThreadSafeSingleton());
            ThreadSafeEventLog::instance().record("Singleton initialized");
        });
        
        return instance_;
    }
    
    void do_work()
    {
        ThreadSafeEventLog::instance().record("Singleton work executed");
    }
    
private:
    ThreadSafeSingleton()
    : tracked_("Singleton")
    {
    }
    
    Tracked tracked_;
    static std::shared_ptr<ThreadSafeSingleton> instance_;
    static std::once_flag init_flag_;
};

std::shared_ptr<ThreadSafeSingleton> ThreadSafeSingleton::instance_;
std::once_flag ThreadSafeSingleton::init_flag_;

TEST_F(MultiThreadedPatternsTest, ThreadSafeSingletonAccess)
{
    Barrier sync_point(10);
    std::vector<std::thread> threads;
    std::vector<void*> instance_addresses;
    std::mutex addresses_mutex;
    
    // Q: What does std::call_once guarantee?
    // A:
    // R:
    
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([&sync_point, &instance_addresses, &addresses_mutex]()
        {
            sync_point.arrive_and_wait();  // All threads try to get instance simultaneously
            
            std::shared_ptr<ThreadSafeSingleton> inst = ThreadSafeSingleton::instance();
            inst->do_work();
            
            {
                std::lock_guard<std::mutex> lock(addresses_mutex);
                instance_addresses.push_back(inst.get());
            }
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: How many times is "Singleton initialized" logged?
    // A:
    // R:
    
    // Q: Why do all threads get the same instance address?
    // A:
    // R:
    
    EXPECT_EQ(ThreadSafeEventLog::instance().count("Singleton initialized"), 1);
    
    // Verify all addresses are the same
    void* first_address = instance_addresses[0];
    for (void* addr : instance_addresses)
    {
        EXPECT_EQ(addr, first_address);
    }
}

// ============================================================================
// TEST 12: Circular Reference with Threads (Anti-pattern)
// ============================================================================

class ThreadedNode : public std::enable_shared_from_this<ThreadedNode>
{
public:
    explicit ThreadedNode(const std::string& name)
    : tracked_(name)
    {
    }
    
    void set_next_unsafe(std::shared_ptr<ThreadedNode> next)
    {
        // ANTI-PATTERN: Creates circular reference
        next_ = next;
    }
    
    void set_next_safe(std::shared_ptr<ThreadedNode> next)
    {
        // CORRECT: Use weak_ptr to break cycle
        weak_next_ = next;
    }
    
    std::shared_ptr<ThreadedNode> next() const
    {
        return weak_next_.lock();
    }
    
private:
    Tracked tracked_;
    std::shared_ptr<ThreadedNode> next_;  // Causes leak
    std::weak_ptr<ThreadedNode> weak_next_;  // Prevents leak
};

TEST_F(MultiThreadedPatternsTest, CircularReferenceAcrossThreads)
{
    std::atomic<int> dtors_called{0};
    
    {
        std::shared_ptr<ThreadedNode> node1 = std::make_shared<ThreadedNode>("Node1");
        std::shared_ptr<ThreadedNode> node2 = std::make_shared<ThreadedNode>("Node2");
        
        std::thread t1([node1, node2]()
        {
            node1->set_next_unsafe(node2);
        });
        
        std::thread t2([node1, node2]()
        {
            node2->set_next_unsafe(node1);
        });
        
        t1.join();
        t2.join();
        
        // Q: What is the use_count of node1 and node2 after the circular reference is created?
        // A:
        // R:
        
        // Q: Why won't the nodes be destroyed when this scope ends?
        // A:
        // R:
    }
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    // Q: How many destructors are called, and why?
    // A:
    // R:
    
    EXPECT_EQ(dtor_count, 0);  // Memory leak!
}

TEST_F(MultiThreadedPatternsTest, BreakCircularReferenceWithWeakPtr)
{
    {
        std::shared_ptr<ThreadedNode> node1 = std::make_shared<ThreadedNode>("Node3");
        std::shared_ptr<ThreadedNode> node2 = std::make_shared<ThreadedNode>("Node4");
        
        std::thread t1([node1, node2]()
        {
            node1->set_next_safe(node2);
        });
        
        std::thread t2([node1, node2]()
        {
            node2->set_next_safe(node1);
        });
        
        t1.join();
        t2.join();
        
        // Q: What is the use_count of node1 and node2 with weak_ptr?
        // A:
        // R:
        
        // Q: Why are the nodes properly destroyed when this scope ends?
        // A:
        // R:
    }
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    // Q: How does weak_ptr prevent the memory leak?
    // A:
    // R:
    
    EXPECT_EQ(dtor_count, 2);  // Both nodes destroyed
}

// ============================================================================
// TEST 13: Timer-Based Callbacks with Cancellation
// ============================================================================

class CancellableTimer : public std::enable_shared_from_this<CancellableTimer>
{
public:
    explicit CancellableTimer(asio::io_context& io, const std::string& name)
    : io_(io)
    , timer_(io)
    , tracked_(name)
    , cancelled_(false)
    {
    }
    
    void start(std::chrono::milliseconds delay)
    {
        timer_.expires_after(delay);
        
        std::weak_ptr<CancellableTimer> weak_self = shared_from_this();
        
        timer_.async_wait([weak_self](const asio::error_code& ec)
        {
            if (ec)
            {
                ThreadSafeEventLog::instance().record("Timer cancelled or error");
                return;
            }
            
            if (std::shared_ptr<CancellableTimer> self = weak_self.lock())
            {
                if (!self->cancelled_)
                {
                    ThreadSafeEventLog::instance().record("Timer fired: " + self->tracked_.name());
                }
                else
                {
                    ThreadSafeEventLog::instance().record("Timer skipped - cancelled");
                }
            }
            else
            {
                ThreadSafeEventLog::instance().record("Timer skipped - object expired");
            }
        });
        
        // Q: Why capture weak_ptr instead of shared_ptr in the timer callback?
        // A:
        // R:
    }
    
    void cancel()
    {
        cancelled_ = true;
        timer_.cancel();
    }
    
private:
    asio::io_context& io_;
    asio::steady_timer timer_;
    Tracked tracked_;
    bool cancelled_;
};

TEST_F(MultiThreadedPatternsTest, TimerCallbackWithWeakPtr)
{
    asio::io_context io;
    
    {
        std::shared_ptr<CancellableTimer> timer = std::make_shared<CancellableTimer>(io, "Timer1");
        timer->start(std::chrono::milliseconds(50));
        
        // Q: What happens if timer goes out of scope before the timer fires?
        // A:
        // R:
    }
    
    std::thread io_thread([&io]() { io.run(); });
    io_thread.join();
    
    // Q: What does the timer callback observe when it executes?
    // A:
    // R:
    
    EXPECT_EQ(ThreadSafeEventLog::instance().count("object expired"), 1);
}

TEST_F(MultiThreadedPatternsTest, TimerCancellation)
{
    asio::io_context io;
    std::shared_ptr<CancellableTimer> timer = std::make_shared<CancellableTimer>(io, "Timer2");
    
    timer->start(std::chrono::milliseconds(100));
    
    std::thread io_thread([&io]() { io.run(); });
    
    // Cancel after 20ms
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    timer->cancel();
    
    io_thread.join();
    
    // Q: What prevents the timer callback from executing after cancellation?
    // A:
    // R:
    
    EXPECT_GT(ThreadSafeEventLog::instance().count("cancelled"), 0);
}

// ============================================================================
// TEST 14: Connection Management Pattern (Real-world async I/O)
// ============================================================================

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    explicit Connection(asio::io_context& io, int id)
    : io_(io)
    , tracked_("Connection" + std::to_string(id))
    , id_(id)
    , active_(true)
    {
        ThreadSafeEventLog::instance().record("Connection " + std::to_string(id_) + " created");
    }
    
    ~Connection()
    {
        ThreadSafeEventLog::instance().record("Connection " + std::to_string(id_) + " destroyed");
    }
    
    void start_async_read()
    {
        // Q: Why is it critical to use shared_from_this() here?
        // A:
        // R:
        
        asio::post(io_, std::bind(&Connection::handle_read, shared_from_this()));
    }
    
    void close()
    {
        active_ = false;
    }
    
    bool is_active() const { return active_; }
    
private:
    void handle_read()
    {
        if (!active_)
        {
            ThreadSafeEventLog::instance().record("Connection " + std::to_string(id_) + " read cancelled");
            return;
        }
        
        ThreadSafeEventLog::instance().record("Connection " + std::to_string(id_) + " read completed");
        
        // Simulate another async read
        if (active_)
        {
            asio::post(io_, std::bind(&Connection::handle_read, shared_from_this()));
        }
    }
    
    asio::io_context& io_;
    Tracked tracked_;
    int id_;
    bool active_;
};

class ConnectionManager
{
public:
    explicit ConnectionManager(asio::io_context& io)
    : io_(io)
    {
    }
    
    void add_connection(int id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::shared_ptr<Connection> conn = std::make_shared<Connection>(io_, id);
        connections_[id] = conn;
        conn->start_async_read();
        
        // Q: Why store weak_ptr in the manager instead of shared_ptr?
        // A:
        // R:
    }
    
    void close_connection(int id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = connections_.find(id);
        if (it != connections_.end())
        {
            if (std::shared_ptr<Connection> conn = it->second.lock())
            {
                conn->close();
            }
            connections_.erase(it);
        }
    }
    
    size_t active_count() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        size_t count = 0;
        for (const auto& pair : connections_)
        {
            if (std::shared_ptr<Connection> conn = pair.second.lock())
            {
                if (conn->is_active())
                {
                    ++count;
                }
            }
        }
        return count;
    }
    
private:
    asio::io_context& io_;
    mutable std::mutex mutex_;
    std::map<int, std::weak_ptr<Connection>> connections_;
};

TEST_F(MultiThreadedPatternsTest, ConnectionManagementPattern)
{
    asio::io_context io;
    ConnectionManager manager(io);
    
    // Add connections
    manager.add_connection(1);
    manager.add_connection(2);
    manager.add_connection(3);
    
    std::thread io_thread([&io]() { io.run_for(std::chrono::milliseconds(100)); });
    
    // Q: What keeps the Connection objects alive while async operations are pending?
    // A:
    // R:
    
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    manager.close_connection(2);
    
    // Q: After close_connection(2), when is Connection 2 destroyed?
    // A:
    // R:
    
    io_thread.join();
    
    // Q: Why is weak_ptr ideal for connection managers?
    // A:
    // R:
    
    EXPECT_EQ(ThreadSafeEventLog::instance().count("created"), 3);
    EXPECT_EQ(ThreadSafeEventLog::instance().count("destroyed"), 3);
}

// ============================================================================
// TEST 15: std::bind vs Lambda Capture Comparison
// ============================================================================

class BindVsLambda : public std::enable_shared_from_this<BindVsLambda>
{
public:
    explicit BindVsLambda(const std::string& name)
    : tracked_(name)
    {
    }
    
    void method_with_args(int value, const std::string& text)
    {
        ThreadSafeEventLog::instance().record("method_with_args: " + std::to_string(value) + ", " + text);
    }
    
    void schedule_with_bind(asio::io_context& io)
    {
        // Using std::bind
        asio::post(io, std::bind(&BindVsLambda::method_with_args, shared_from_this(), 42, "bind"));
        
        // Q: What does std::bind capture, and how does it keep the object alive?
        // A:
        // R:
    }
    
    void schedule_with_lambda(asio::io_context& io)
    {
        // Using lambda
        asio::post(io, [self = shared_from_this()]()
        {
            self->method_with_args(42, "lambda");
        });
        
        // Q: What is the advantage of lambda capture over std::bind?
        // A:
        // R:
    }
    
private:
    Tracked tracked_;
};

TEST_F(MultiThreadedPatternsTest, BindVsLambdaComparison)
{
    asio::io_context io;
    
    {
        std::shared_ptr<BindVsLambda> obj = std::make_shared<BindVsLambda>("BindLambda");
        
        obj->schedule_with_bind(io);
        obj->schedule_with_lambda(io);
        
        // Q: Do both std::bind and lambda capture keep the object alive?
        // A:
        // R:
    }
    
    io.run();
    
    // Q: When should you prefer std::bind over lambda, and vice versa?
    // A:
    // R:
    
    EXPECT_EQ(ThreadSafeEventLog::instance().count("method_with_args"), 2);
}
