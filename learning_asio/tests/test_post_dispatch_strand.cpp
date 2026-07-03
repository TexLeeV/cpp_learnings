// Test Suite: Strands, post(), and dispatch()
// Estimated Time: 3-4 hours
// Difficulty: Moderate
// C++ Standard: C++11 (Boost.Asio, classic callback style)
//
// When several threads call io_context::run() on the SAME context, completion
// handlers can execute concurrently. That is great for throughput and terrible
// for any shared mutable state. A strand is Asio's answer: handlers submitted
// through one strand are guaranteed never to run concurrently with each other,
// and the strand also establishes the memory ordering needed for the data they
// touch. A strand is "a mutex you never lock" -- serialization without a lock
// held across the handler.
//
// Recommended: run this suite under the gcc-tsan preset
//     cmake --preset gcc-tsan
//     cmake --build --preset gcc-tsan --target test_post_dispatch_strand
//     ctest --preset gcc-tsan -R test_post_dispatch_strand --output-on-failure
// The plain (non-atomic) counter in Scenario 1 is touched from a 4-thread pool;
// it is race-free ONLY because the strand serializes the handlers. Remove the
// strand and ThreadSanitizer will report the race that proves why strands exist.

#include "instrumentation.h"

#include <boost/asio.hpp>

#include <gtest/gtest.h>
#include <atomic>
#include <cstddef>
#include <string>
#include <thread>
#include <vector>

class StrandTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: A Strand Serializes Handlers So They Never Overlap (Moderate)
// ============================================================================

TEST_F(StrandTest, StrandSerializesHandlersOnAThreadPool)
{
    const int kHandlers = 2000;
    const int kThreads = 4;

    boost::asio::io_context ctx;
    auto strand = boost::asio::make_strand(ctx);

    // Deliberately a PLAIN int, not a std::atomic. Its correctness depends
    // entirely on the strand preventing concurrent handler execution.
    int unsynchronized_counter = 0;

    // These atomics only *observe* concurrency; they are not what we are
    // testing. active = handlers currently executing; max_active = the peak.
    std::atomic<int> active(0);
    std::atomic<int> max_active(0);

    for (int i = 0; i < kHandlers; ++i)
    {
        boost::asio::post(strand, [&] {
            const int now = active.fetch_add(1) + 1;
            int observed = max_active.load();
            while (now > observed && !max_active.compare_exchange_weak(observed, now))
            {
            }

            ++unsynchronized_counter;

            // Record each worker thread once so EventLog shows the pool really
            // was multi-threaded (count is 1..kThreads, scheduler-dependent).
            thread_local bool noted = false;
            if (!noted)
            {
                noted = true;
                EventLog::instance().record("worker-thread");
            }

            active.fetch_sub(1);
        });
    }

    std::vector<std::thread> pool;
    for (int t = 0; t < kThreads; ++t)
    {
        pool.emplace_back([&ctx] { ctx.run(); });
    }
    for (std::size_t t = 0; t < pool.size(); ++t)
    {
        pool[t].join();
    }

    // Q: Four threads ran handlers that each did a non-atomic ++ on the same
    //    int. Why does the final value equal kHandlers exactly, with no lost
    //    updates, despite there being no mutex and no atomic on that counter?
    // A:
    // R:
    EXPECT_EQ(unsynchronized_counter, kHandlers);

    // Q: max_active is the largest number of handlers seen running at the same
    //    instant. What value must it be if the strand did its job, and what
    //    would a value greater than that have proven?
    // A:
    // R:
    EXPECT_EQ(max_active.load(), 1);

    // The pool was genuinely multi-threaded (at least one worker ran handlers).
    const std::size_t workers = EventLog::instance().count_events("worker-thread");
    EXPECT_GE(workers, 1u);
    EXPECT_LE(workers, static_cast<std::size_t>(kThreads));

    // TODO (learner): Build this file under the gcc-tsan preset, then remove the
    // strand (post directly to `ctx` instead of `strand`) and rebuild. Record
    // in // A: above the exact ThreadSanitizer report you get, and identify
    // which two operations it names as racing.
}

// ============================================================================
// Scenario 2: A Strand Preserves Submission Order Across Threads (Moderate)
// ============================================================================

TEST_F(StrandTest, StrandPreservesPostOrderEvenOnAPool)
{
    boost::asio::io_context ctx;
    auto strand = boost::asio::make_strand(ctx);

    // Posted in a fixed order from this single thread, then drained by a pool.
    boost::asio::post(strand, [] { EventLog::instance().record("s1"); });
    boost::asio::post(strand, [] { EventLog::instance().record("s2"); });
    boost::asio::post(strand, [] { EventLog::instance().record("s3"); });
    boost::asio::post(strand, [] { EventLog::instance().record("s4"); });
    boost::asio::post(strand, [] { EventLog::instance().record("s5"); });

    std::vector<std::thread> pool;
    for (int t = 0; t < 4; ++t)
    {
        pool.emplace_back([&ctx] { ctx.run(); });
    }
    for (std::size_t t = 0; t < pool.size(); ++t)
    {
        pool[t].join();
    }

    // Q: Four worker threads competed to run these handlers, yet the EventLog
    //    order is fully determined. What ordering guarantee does a strand make
    //    for handlers submitted via post() from the same thread, and why is
    //    that stronger than what a plain mutex would give you?
    // A:
    // R:
    std::cout << EventLog::instance().dump() << std::endl;
    const std::vector<std::string> ev = EventLog::instance().events();
    // ASSERT_EQ(ev.size(), 5u);
    // EXPECT_EQ(ev[0], "s1");
    // EXPECT_EQ(ev[1], "s2");
    // EXPECT_EQ(ev[2], "s3");
    // EXPECT_EQ(ev[3], "s4");
    // EXPECT_EQ(ev[4], "s5");
}

// ============================================================================
// Scenario 3: dispatch() on Your Own Strand Runs Inline; post() Defers (Moderate)
// ============================================================================

TEST_F(StrandTest, DispatchOnOwnStrandRunsInlineWhilePostDefers)
{
    boost::asio::io_context ctx;
    auto strand = boost::asio::make_strand(ctx);

    // Single-threaded run() keeps this fully deterministic. The outer handler
    // is already executing *inside* the strand when it calls post and dispatch.
    boost::asio::post(strand, [&strand] {
        EventLog::instance().record("outer-start");
        boost::asio::post(strand, [] { EventLog::instance().record("posted"); });
        boost::asio::dispatch(strand, [] { EventLog::instance().record("dispatched"); });
        EventLog::instance().record("outer-end");
    });

    ctx.run();

    // Q: dispatch() may run its handler immediately when the caller is already
    //    inside the target strand (running inline cannot violate the
    //    no-concurrent-execution guarantee). Given that, where does
    //    "dispatched" land relative to "outer-end", and where does "posted"
    //    land? Predict the order before reading the expectations.
    // A:
    // R:
    const std::vector<std::string> ev = EventLog::instance().events();
    ASSERT_EQ(ev.size(), 4u);
    EXPECT_EQ(ev[0], "outer-start");
    EXPECT_EQ(ev[1], "dispatched");
    EXPECT_EQ(ev[2], "outer-end");
    EXPECT_EQ(ev[3], "posted");

    // Q: Suppose a SECOND thread were also running this io_context and called
    //    dispatch() on this strand while the outer handler was mid-flight. Would
    //    dispatch still run inline on that other thread? What rule about strand
    //    membership decides inline-vs-deferred?
    // A:
    // R:

    // TODO (learner): Change the inner dispatch() to post() and predict which
    // two EventLog entries swap. Record the prediction in // A: above, then
    // verify by rebuilding.
}
