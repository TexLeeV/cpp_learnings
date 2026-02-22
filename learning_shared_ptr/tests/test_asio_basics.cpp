#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

// ============================================================================
// ASIO CORE CONCEPTS
// ============================================================================
//
// asio::post(io_context, handler)
//   - Submits handler to io_context's task queue
//   - Non-blocking: returns immediately
//   - Thread-safe: can call from any thread
//   - FIFO order: handlers execute in submission order
//
// io_context::run()
//   - Blocks calling thread and executes queued handlers
//   - Returns when queue is empty (no more work)
//   - Can be called from multiple threads (thread pool pattern)
//   - Returns: number of handlers executed
//
// Pattern:
//   asio::io_context io;
//   asio::post(io, []() { /* work */ });  // Queue work
//   io.run();                              // Execute work (blocks)
//
// ============================================================================

class AsioBasicsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(AsioBasicsTest, Lesson1_BasicPostAndRun)
{
    asio::io_context io;
    bool callback_executed = false;
    
    // Q: What does asio::post() do?
    // A:
    // R:
    
    asio::post(io, [&callback_executed]()
    {
        callback_executed = true;
    });
    
    // Q: At this point (before io.run()), has the callback executed?
    // A:
    // R:
    
    EXPECT_FALSE(callback_executed);
    
    // Q: What does io.run() do?
    // A:
    // R:
    
    io.run();
    
    // Q: After io.run(), has the callback executed?
    // A:
    // R:
    
    EXPECT_TRUE(callback_executed);
}

TEST_F(AsioBasicsTest, Lesson2_MultipleCallbacksOrder)
{
    asio::io_context io;
    std::vector<int> execution_order;
    
    // Post 3 callbacks
    asio::post(io, [&execution_order]() { execution_order.push_back(1); });
    asio::post(io, [&execution_order]() { execution_order.push_back(2); });
    asio::post(io, [&execution_order]() { execution_order.push_back(3); });
    
    // Q: In what order will the callbacks execute?
    // A:
    // R:
    
    io.run();
    
    // Q: Why does asio::post() guarantee FIFO (first-in-first-out) order?
    // A:
    // R:
    
    EXPECT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
    EXPECT_EQ(execution_order[2], 3);
}

TEST_F(AsioBasicsTest, Lesson3_RunInSeparateThread)
{
    asio::io_context io;
    std::atomic<bool> callback_executed{false};
    
    asio::post(io, [&callback_executed]()
    {
        callback_executed = true;
    });
    
    // Q: What happens when we run io.run() in a separate thread?
    // A:
    // R:
    
    std::thread io_thread([&io]()
    {
        io.run();  // Runs in separate thread
    });
    
    // Q: Why do we need to join the thread?
    // A:
    // R:
    
    io_thread.join();
    
    EXPECT_TRUE(callback_executed);
}

// ============================================================================
// LESSON 4: Capturing Variables by Value vs Reference
// ============================================================================

TEST_F(AsioBasicsTest, Lesson4_CaptureByValueVsReference)
{
    asio::io_context io;
    int value = 10;
    int result_by_value = 0;
    int result_by_reference = 0;
    
    // Capture by value
    asio::post(io, [value, &result_by_value]()
    {
        result_by_value = value;
    });
    
    // Capture by reference
    asio::post(io, [&value, &result_by_reference]()
    {
        result_by_reference = value;
    });
    
    // Change value before running
    value = 20;
    
    // Q: What will result_by_value be? Why?
    // A:
    // R:
    
    // Q: What will result_by_reference be? Why?
    // A:
    // R:
    
    io.run();
    
    EXPECT_EQ(result_by_value, 10);   // Captured value at time of post
    EXPECT_EQ(result_by_reference, 20); // Captured reference, sees changed value
}

// ============================================================================
// LESSON 5: Basic shared_ptr with asio::post()
// ============================================================================

TEST_F(AsioBasicsTest, Lesson5_SharedPtrWithPost)
{
    asio::io_context io;
    long use_count_in_callback = 0;
    
    {
        std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Obj1");
        
        // Q: What is the use_count before posting?
        // A:
        // R:
        
        long before_post = ptr.use_count();
        EXPECT_EQ(before_post, 1);
        
        // Capture shared_ptr by value
        asio::post(io, [ptr, &use_count_in_callback]()
        {
            use_count_in_callback = ptr.use_count();
        });
        
        // Q: What is the use_count after posting but before io.run()?
        // A:
        // R:
        
        long after_post = ptr.use_count();
        EXPECT_EQ(after_post, 2);  // ptr + lambda's copy
        
        // Q: When this scope ends, what happens to ptr?
        // A:
        // R:
    }
    
    // Q: Is the Tracked object destroyed at this point?
    // A:
    // R:
    
    io.run();
    
    // Q: What was the use_count inside the callback?
    // A:
    // R:
    
    EXPECT_EQ(use_count_in_callback, 1);  // Only lambda's copy remains
}

// ============================================================================
// LESSON 6: Danger of Raw Pointers with asio::post()
// ============================================================================

TEST_F(AsioBasicsTest, Lesson6_RawPointerDanger)
{
    asio::io_context io;
    bool callback_executed = false;
    
    Tracked* raw_ptr = nullptr;
    
    {
        std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Obj2");
        raw_ptr = ptr.get();
        
        // DANGER: Capturing raw pointer
        asio::post(io, [raw_ptr, &callback_executed]()
        {
            // Q: Is it safe to use raw_ptr here?
            // A:
            // R:
            
            // We won't actually dereference it to avoid crash
            callback_executed = true;
        });
        
        // Q: When ptr goes out of scope, what happens to the object?
        // A:
        // R:
    }
    
    // Q: At this point, what does raw_ptr point to?
    // A:
    // R:
    
    io.run();
    
    EXPECT_TRUE(callback_executed);
    // NOTE: Dereferencing raw_ptr here would be undefined behavior!
}

// ============================================================================
// LESSON 7: Safe Pattern - Capturing shared_ptr
// ============================================================================

TEST_F(AsioBasicsTest, Lesson7_SafeSharedPtrCapture)
{
    asio::io_context io;
    std::string result;
    
    {
        std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Obj3");
        
        // SAFE: Capture shared_ptr by value
        asio::post(io, [ptr, &result]()
        {
            // Q: Why is it safe to use ptr here?
            // A:
            // R:
            
            result = ptr->name();
        });
        
        // Q: Even though ptr goes out of scope, why isn't the object destroyed?
        // A:
        // R:
    }
    
    io.run();
    
    // Q: When is the Tracked object finally destroyed?
    // A:
    // R:
    
    EXPECT_EQ(result, "Obj3");
}

// ============================================================================
// LESSON 8: Multiple Threads Running io_context
// ============================================================================

TEST_F(AsioBasicsTest, Lesson8_MultipleThreadsRunningIoContext)
{
    asio::io_context io;
    std::atomic<int> tasks_executed{0};
    
    // Post 10 tasks
    for (int i = 0; i < 10; ++i)
    {
        asio::post(io, [&tasks_executed]()
        {
            ++tasks_executed;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    
    // Q: What happens when multiple threads call io.run()?
    // A:
    // R:
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i)
    {
        threads.emplace_back([&io]()
        {
            io.run();  // Multiple threads processing the same queue
        });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: How many tasks were executed?
    // A:
    // R:
    
    EXPECT_EQ(tasks_executed, 10);
}

// ============================================================================
// LESSON 9: Basic Timer (asio::steady_timer)
// ============================================================================

TEST_F(AsioBasicsTest, Lesson9_BasicTimer)
{
    asio::io_context io;
    bool timer_fired = false;
    
    asio::steady_timer timer(io);
    
    // Q: What does expires_after() do?
    // A:
    // R:
    
    timer.expires_after(std::chrono::milliseconds(50));
    
    // Q: What does async_wait() do?
    // A:
    // R:
    
    timer.async_wait([&timer_fired](const asio::error_code& ec)
    {
        if (!ec)
        {
            timer_fired = true;
        }
    });
    
    // Q: At this point, has the timer fired?
    // A:
    // R:
    
    EXPECT_FALSE(timer_fired);
    
    // Q: What happens when io.run() is called?
    // A:
    // R:
    
    io.run();  // Blocks until timer fires
    
    EXPECT_TRUE(timer_fired);
}

// ============================================================================
// LESSON 10: Timer with shared_ptr Lifetime
// ============================================================================

TEST_F(AsioBasicsTest, Lesson10_TimerWithSharedPtrLifetime)
{
    asio::io_context io;
    std::string result;
    
    {
        std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("TimerObj");
        asio::steady_timer timer(io);
        
        timer.expires_after(std::chrono::milliseconds(50));
        
        // Q: What happens if we capture ptr by value in the timer callback?
        // A:
        // R:
        
        timer.async_wait([ptr, &result](const asio::error_code& ec)
        {
            if (!ec)
            {
                result = ptr->name();
            }
        });
        
        // Q: When this scope ends, is the Tracked object destroyed?
        // A:
        // R:
    }
    
    io.run();
    
    // Q: Why was the object still alive when the timer fired?
    // A:
    // R:
    
    EXPECT_EQ(result, "TimerObj");
}

// ============================================================================
// LESSON 11: std::bind Basics
// ============================================================================

class SimpleClass
{
public:
    explicit SimpleClass(const std::string& name)
    : tracked_(name)
    {
    }
    
    void method(int value)
    {
        last_value_ = value;
    }
    
    int last_value() const { return last_value_; }
    
private:
    Tracked tracked_;
    int last_value_ = 0;
};

TEST_F(AsioBasicsTest, Lesson11_StdBindBasics)
{
    asio::io_context io;
    
    std::shared_ptr<SimpleClass> obj = std::make_shared<SimpleClass>("BindObj");
    
    // Q: What does std::bind do?
    // A:
    // R:
    
    // Using std::bind to bind member function
    asio::post(io, std::bind(&SimpleClass::method, obj, 42));
    
    // Q: What arguments does std::bind capture?
    // A:
    // R:
    
    io.run();
    
    EXPECT_EQ(obj->last_value(), 42);
}

// ============================================================================
// LESSON 12: std::bind vs Lambda
// ============================================================================

TEST_F(AsioBasicsTest, Lesson12_BindVsLambda)
{
    asio::io_context io;
    
    std::shared_ptr<SimpleClass> obj = std::make_shared<SimpleClass>("CompareObj");
    
    // Using std::bind
    asio::post(io, std::bind(&SimpleClass::method, obj, 10));
    
    // Using lambda (equivalent)
    asio::post(io, [obj]()
    {
        obj->method(20);
    });
    
    // Q: Which is more readable: std::bind or lambda?
    // A:
    // R:
    
    // Q: When would you prefer std::bind over lambda?
    // A:
    // R:
    
    io.run();
    
    EXPECT_EQ(obj->last_value(), 20);  // Lambda executed second
}

// ============================================================================
// LESSON 13: Synchronization with std::mutex
// ============================================================================

TEST_F(AsioBasicsTest, Lesson13_BasicMutex)
{
    asio::io_context io;
    int counter = 0;
    std::mutex mutex;
    
    // Post 100 tasks that increment counter
    for (int i = 0; i < 100; ++i)
    {
        asio::post(io, [&counter, &mutex]()
        {
            // Q: Why do we need std::lock_guard here?
            // A:
            // R:
            
            std::lock_guard<std::mutex> lock(mutex);
            ++counter;
        });
    }
    
    // Run with multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&io]() { io.run(); });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: Without the mutex, what could happen to counter?
    // A:
    // R:
    
    EXPECT_EQ(counter, 100);
}

// ============================================================================
// LESSON 14: Atomic Variables
// ============================================================================

TEST_F(AsioBasicsTest, Lesson14_AtomicVariables)
{
    asio::io_context io;
    std::atomic<int> atomic_counter{0};
    int regular_counter = 0;
    std::mutex mutex;
    
    // Post 100 tasks
    for (int i = 0; i < 100; ++i)
    {
        asio::post(io, [&atomic_counter, &regular_counter, &mutex]()
        {
            // Atomic increment (thread-safe without mutex)
            ++atomic_counter;
            
            // Regular increment (needs mutex)
            std::lock_guard<std::mutex> lock(mutex);
            ++regular_counter;
        });
    }
    
    // Q: What is the difference between std::atomic and regular int?
    // A:
    // R:
    
    // Q: When should you use std::atomic vs std::mutex?
    // A:
    // R:
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&io]() { io.run(); });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    EXPECT_EQ(atomic_counter, 100);
    EXPECT_EQ(regular_counter, 100);
}

// ============================================================================
// LESSON 15: Putting It All Together - Safe Async Pattern
// ============================================================================

class AsyncWorker : public std::enable_shared_from_this<AsyncWorker>
{
public:
    explicit AsyncWorker(const std::string& name)
    : tracked_(name)
    , completed_(false)
    {
    }
    
    void start_work(asio::io_context& io)
    {
        // Q: Why do we use shared_from_this() here?
        // A:
        // R:
        
        asio::post(io, std::bind(&AsyncWorker::do_work, shared_from_this()));
    }
    
    bool is_completed() const { return completed_; }
    
private:
    void do_work()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        completed_ = true;
    }
    
    Tracked tracked_;
    bool completed_;
};

TEST_F(AsioBasicsTest, Lesson15_SafeAsyncPattern)
{
    asio::io_context io;
    
    {
        std::shared_ptr<AsyncWorker> worker = std::make_shared<AsyncWorker>("Worker1");
        
        // Q: What keeps the worker alive after this scope ends?
        // A:
        // R:
        
        worker->start_work(io);
        
        // Q: Is it safe for worker to go out of scope here?
        // A:
        // R:
    }
    
    // Q: When will the AsyncWorker object be destroyed?
    // A:
    // R:
    
    io.run();
    
    // At this point, the worker has been destroyed
    // (after do_work() completed and the callback finished)
}

// ============================================================================
// SUMMARY TEST: All Concepts Together
// ============================================================================

TEST_F(AsioBasicsTest, Summary_AllConceptsTogether)
{
    asio::io_context io;
    std::atomic<int> tasks_completed{0};
    
    // Create multiple workers
    for (int i = 0; i < 5; ++i)
    {
        std::shared_ptr<AsyncWorker> worker = std::make_shared<AsyncWorker>("Worker" + std::to_string(i));
        
        // Post work using lambda with shared_ptr capture
        asio::post(io, [worker, &tasks_completed]()
        {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ++tasks_completed;
        });
    }
    
    // Run on multiple threads
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i)
    {
        threads.emplace_back([&io]() { io.run(); });
    }
    
    for (auto& t : threads)
    {
        t.join();
    }
    
    // Q: Review - What are the key concepts demonstrated in this test?
    // A:
    // A:
    // A:
    // A:
    // A:
    // R:
    
    EXPECT_EQ(tasks_completed, 5);
}
