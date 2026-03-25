// Test Suite: Thread Pools and Work Stealing
// Estimated Time: 5 hours
// Difficulty: Hard

#include "instrumentation.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <gtest/gtest.h>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPoolsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Shared Thread Pool Base Implementation
// ============================================================================

class ThreadPool
{
public:
    explicit ThreadPool(size_t num_threads) : stop_(false)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers_.emplace_back([this, i]() {
                EventLog::instance().record("Worker " + std::to_string(i) + " started");
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

                        if (stop_ && tasks_.empty())
                        {
                            EventLog::instance().record("Worker " + std::to_string(i) + " exiting");
                            return;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }

                    try
                    {
                        task();
                    }
                    catch (const std::exception& e)
                    {
                        EventLog::instance().record("Worker " + std::to_string(i) + " caught exception: " + e.what());
                    }
                }
            });
        }
    }

    template <typename F> void enqueue(F&& task)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_)
            {
                EventLog::instance().record("Task rejected: pool is shutting down");
                return;
            }
            tasks_.emplace(std::forward<F>(task));
        }
        cv_.notify_one();
    }

    template <typename F, typename... Args>
    auto enqueue_future(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();

        return result;
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
    }

    void wait()
    {
        for (auto& worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    size_t pending_tasks() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return tasks_.size();
    }

    ~ThreadPool()
    {
        shutdown();
        wait();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
};

// ============================================================================
// TEST 1: Basic Thread Pool Execution - Moderate
// ============================================================================

TEST_F(ThreadPoolsTest, BasicThreadPoolExecution)
{
    constexpr size_t num_workers = 4;
    constexpr int num_tasks = 20;
    std::atomic<int> completed{0};

    ThreadPool pool(num_workers);

    for (int i = 0; i < num_tasks; ++i)
    {
        pool.enqueue([&completed, i]() {
            EventLog::instance().record("Task " + std::to_string(i) + " executing");
            completed.fetch_add(1);
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Q: With 4 workers and 20 tasks, what is the minimum number of tasks each worker
    // Q: could execute? What determines the actual distribution?
    // A:
    // R:

    // Q: In the destructor, why must stop_ be set to true before calling notify_all()?
    // Q: What happens if notify_all() is called first?
    // A:
    // R:

    EXPECT_EQ(completed.load(), num_tasks);
    EXPECT_EQ(EventLog::instance().count_events("started"), num_workers);
}

// ============================================================================
// TEST 2: Thread Pool with std::future Return Values - Moderate
// ============================================================================

TEST_F(ThreadPoolsTest, ThreadPoolWithFutures)
{
    ThreadPool pool(4);

    auto future1 = pool.enqueue_future([](int x) { return x * 2; }, 21);
    auto future2 = pool.enqueue_future([](int x, int y) { return x + y; }, 10, 20);
    auto future3 = pool.enqueue_future([]() { return std::string("hello"); });

    // Q: The enqueue_future() call returns immediately. When does the lambda actually execute?
    // Q: What thread executes it?
    // A:
    // R:

    // Q: If you call future.get() before the task executes, what happens to the calling
    // Q: thread? What synchronization does std::future provide?
    // A:
    // R:

    EXPECT_EQ(future1.get(), 42);
    EXPECT_EQ(future2.get(), 30);
    EXPECT_EQ(future3.get(), "hello");
}

// ============================================================================
// TEST 3: Thread Pool Shutdown and Task Cancellation - Hard
// ============================================================================

TEST_F(ThreadPoolsTest, ThreadPoolShutdown)
{
    ThreadPool pool(2);
    std::atomic<int> completed{0};

    for (int i = 0; i < 5; ++i)
    {
        pool.enqueue([&completed, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            completed.fetch_add(1);
            EventLog::instance().record("Task " + std::to_string(i) + " completed");
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    pool.shutdown();

    pool.enqueue([&completed]() { completed.fetch_add(1); });

    pool.wait();

    // Q: After shutdown() is called, the worker threads check `stop_ && tasks_.empty()`.
    // Q: Why check both conditions? What happens to tasks already in the queue?
    // A:
    // R:

    // Q: The enqueue() after shutdown() is rejected. What race condition exists between
    // Q: checking stop_ and adding to tasks_? How does the lock prevent it?
    // A:
    // R:

    EXPECT_GE(completed.load(), 1);
    EXPECT_LE(completed.load(), 5);
    EXPECT_EQ(EventLog::instance().count_events("rejected"), 1);
}

// ============================================================================
// TEST 4: Thread Pool Exception Handling - Hard
// ============================================================================

TEST_F(ThreadPoolsTest, ThreadPoolExceptionHandling)
{
    ThreadPool pool(2);
    std::atomic<int> completed{0};

    pool.enqueue([&completed]() {
        completed.fetch_add(1);
        throw std::runtime_error("Task 1 failed");
    });

    pool.enqueue([&completed]() { completed.fetch_add(1); });

    pool.enqueue([&completed]() { completed.fetch_add(1); });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Q: After Task 1 throws an exception, the worker thread catches it and continues.
    // Q: What would happen if the exception were NOT caught inside the worker loop?
    // A:
    // R:

    // Q: If a task throws, does it affect other tasks in the queue? What isolation
    // Q: does the try-catch block provide?
    // A:
    // R:

    EXPECT_EQ(completed.load(), 3);
    EXPECT_EQ(EventLog::instance().count_events("caught exception"), 1);
}

// ============================================================================
// TEST 5: Thread Pool with Task Dependencies - Hard
// ============================================================================

TEST_F(ThreadPoolsTest, ThreadPoolTaskDependencies)
{
    ThreadPool pool(2);
    std::atomic<int> execution_order{0};

    auto task1 = pool.enqueue_future([&execution_order]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        execution_order.fetch_add(1);
        EventLog::instance().record("Task 1 completed");
    });

    auto task2 = pool.enqueue_future([&execution_order, &task1]() {
        task1.wait();
        execution_order.fetch_add(1);
        EventLog::instance().record("Task 2 completed (after Task 1)");
    });

    auto task3 = pool.enqueue_future([&execution_order, &task1]() {
        task1.wait();
        execution_order.fetch_add(1);
        EventLog::instance().record("Task 3 completed (after Task 1)");
    });

    task2.wait();
    task3.wait();

    // Q: Task 2 and Task 3 both call task1.wait() inside the worker thread. What happens
    // Q: if Task 1 hasn't completed yet? Does the worker block or continue?
    // A:
    // R:

    // Q: With 2 workers and 3 tasks where 2 depend on 1, what is the minimum execution
    // Q: time if Task 1 takes 20ms? Can Task 2 and Task 3 run in parallel?
    // A:
    // R:

    EXPECT_EQ(execution_order.load(), 3);
    EXPECT_EQ(EventLog::instance().count_events("completed"), 3);
}

// ============================================================================
// TEST 6: TODO - Implement Work-Stealing Thread Pool - Hard
// ============================================================================

// TODO: Implement a work-stealing thread pool where:
// TODO: 1. Each worker has its own task queue (deque)
// TODO: 2. Workers push/pop from their own queue (LIFO - cache-friendly)
// TODO: 3. When idle, workers steal from other queues (FIFO - from back)
// TODO: 4. Use std::deque with mutex per queue

class WorkStealingPool
{
public:
    explicit WorkStealingPool(size_t num_threads)
    {
        // TODO: Initialize per-worker queues and threads
    }

    template <typename F> void enqueue(F&& task)
    {
        // TODO: Add task to a worker's queue (round-robin or random)
    }

    ~WorkStealingPool()
    {
        // TODO: Shutdown and join workers
    }

private:
    // TODO: Add worker queues, threads, and synchronization
};

TEST_F(ThreadPoolsTest, DISABLED_WorkStealingThreadPool)
{
    WorkStealingPool pool(4);
    std::atomic<int> completed{0};

    for (int i = 0; i < 100; ++i)
    {
        pool.enqueue([&completed, i]() {
            if (i % 10 == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            completed.fetch_add(1);
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Q: When a worker's queue is empty, it attempts to steal from another worker.
    // Q: Why steal from the back (FIFO) instead of the front (LIFO)?
    // A:
    // R:

    EXPECT_EQ(completed.load(), 100);
}
