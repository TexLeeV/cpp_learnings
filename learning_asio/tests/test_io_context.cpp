// Test Suite: Asio io_context
// Estimated Time: 1-2 hours
// Difficulty: Easy / Moderate
// Library: standalone Asio (asio::) — not std::asio
// C++ Standard: C++20

#include "instrumentation.h"

#include <asio.hpp>
#include <gtest/gtest.h>

class IoContextTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: run() Drains Posted Handlers (Easy)
// ============================================================================

TEST_F(IoContextTest, RunExecutesPostedHandlers)
{
    asio::io_context io;

    asio::post(io, []() { EventLog::instance().record("handler_a"); });
    asio::post(io, []() { EventLog::instance().record("handler_b"); });

    // Q: Before `io.run()`, have either handler executed? Why or why not?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("handler_"), 0);

    const std::size_t ran = io.run();

    // Q: What does `run()` return here, and what condition makes `run()` return?
    // A:
    // R:

    EXPECT_EQ(ran, 2u);
    EXPECT_EQ(EventLog::instance().count_events("handler_a"), 1);
    EXPECT_EQ(EventLog::instance().count_events("handler_b"), 1);
}

// ============================================================================
// Scenario 2: poll() Does Not Block Waiting for Work (Easy)
// ============================================================================

TEST_F(IoContextTest, PollProcessesReadyWorkOnly)
{
    asio::io_context io;
    asio::steady_timer timer(io, std::chrono::seconds(60));
    timer.async_wait([](const asio::error_code&) { EventLog::instance().record("late"); });

    asio::post(io, []() { EventLog::instance().record("ready"); });

    const std::size_t n = io.poll();

    // Q: Why can `poll()` run the posted handler but leave the timer handler
    //    unexecuted?
    // A:
    // R:

    EXPECT_GE(n, 1u);
    EXPECT_EQ(EventLog::instance().count_events("ready"), 1);
    EXPECT_EQ(EventLog::instance().count_events("late"), 0);

    timer.cancel();
    io.run();
}

// ============================================================================
// Scenario 3: Restart After run() Returns (Moderate)
// ============================================================================

TEST_F(IoContextTest, RestartAllowsAnotherRun)
{
    asio::io_context io;

    asio::post(io, []() { EventLog::instance().record("first_batch"); });
    EXPECT_EQ(io.run(), 1u);

    asio::post(io, []() { EventLog::instance().record("second_batch"); });

    // Q: After `run()` returns, the context is stopped. Why does this `run()`
    //    return 0 without executing `second_batch`?
    // A:
    // R:

    EXPECT_EQ(io.run(), 0u);
    EXPECT_EQ(EventLog::instance().count_events("second_batch"), 0);

    io.restart();

    // Q: After `restart()`, what happens to work that was already queued?
    // A:
    // R:

    EXPECT_EQ(io.run(), 1u);
    EXPECT_EQ(EventLog::instance().count_events("second_batch"), 1);
}
