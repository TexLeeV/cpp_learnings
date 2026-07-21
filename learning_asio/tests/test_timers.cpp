// Test Suite: Asio Timers
// Estimated Time: 2 hours
// Difficulty: Easy / Moderate
// Library: standalone Asio (asio::) — not std::asio
// C++ Standard: C++20

#include "instrumentation.h"

#include <asio.hpp>
#include <chrono>
#include <gtest/gtest.h>

class TimersTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Synchronous Wait Blocks Until Expiry (Easy)
// ============================================================================

TEST_F(TimersTest, SyncWaitBlocksUntilExpiry)
{
    asio::io_context io;
    asio::steady_timer timer(io);

    const auto start = std::chrono::steady_clock::now();
    timer.expires_after(std::chrono::milliseconds(20));
    timer.wait();
    const auto elapsed = std::chrono::steady_clock::now() - start;

    // Q: Which clock does `steady_timer` use, and why is that preferable to
    //    wall-clock time for measuring intervals?
    // A:
    // R:

    EXPECT_GE(elapsed, std::chrono::milliseconds(15));
    EventLog::instance().record("sync_wait_done");
    EXPECT_EQ(EventLog::instance().count_events("sync_wait_done"), 1);
}

// ============================================================================
// Scenario 2: async_wait Completes Through io_context::run (Easy)
// ============================================================================

TEST_F(TimersTest, AsyncWaitCompletesViaRun)
{
    asio::io_context io;
    asio::steady_timer timer(io, std::chrono::milliseconds(10));

    asio::error_code done_ec = asio::error::would_block;
    timer.async_wait([&](const asio::error_code& ec) {
        done_ec = ec;
        EventLog::instance().record("timer_fired");
    });

    // Q: Has the completion handler run before `io.run()`? What owns the
    //    "wait for readiness" work?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("timer_fired"), 0);

    io.run();

    // Q: What does a default-constructed / success `error_code` mean for an
    //    expired timer completion?
    // A:
    // R:

    EXPECT_FALSE(done_ec);
    EXPECT_EQ(EventLog::instance().count_events("timer_fired"), 1);
}

// ============================================================================
// Scenario 3: cancel() Completes Handler with operation_aborted (Moderate)
// ============================================================================

TEST_F(TimersTest, CancelCompletesWithOperationAborted)
{
    asio::io_context io;
    asio::steady_timer timer(io, std::chrono::seconds(30));

    asio::error_code done_ec;
    timer.async_wait([&](const asio::error_code& ec) {
        done_ec = ec;
        EventLog::instance().record("timer_completed");
    });

    const std::size_t cancelled = timer.cancel();

    // Q: What does `cancel()` return, and why does the handler still run after
    //    cancel rather than vanishing silently?
    // A:
    // R:

    EXPECT_EQ(cancelled, 1u);
    io.run();

    EXPECT_EQ(done_ec, asio::error::operation_aborted);
    EXPECT_EQ(EventLog::instance().count_events("timer_completed"), 1);
}

// ============================================================================
// Scenario 4: expires_after Resets a Pending Wait (Moderate)
// ============================================================================

TEST_F(TimersTest, ExpiresAfterCancelsPriorAsyncWait)
{
    asio::io_context io;
    asio::steady_timer timer(io);

    int completions = 0;
    asio::error_code first_ec;
    asio::error_code second_ec;

    timer.expires_after(std::chrono::seconds(30));
    timer.async_wait([&](const asio::error_code& ec) {
        first_ec = ec;
        ++completions;
        EventLog::instance().record("first_handler");
    });

    // Setting a new expiry cancels the outstanding wait.
    timer.expires_after(std::chrono::milliseconds(10));
    timer.async_wait([&](const asio::error_code& ec) {
        second_ec = ec;
        ++completions;
        EventLog::instance().record("second_handler");
    });

    io.run();

    // Q: After `expires_after` while a wait is outstanding, which handler sees
    //    `operation_aborted`, and which sees success?
    // A:
    // R:

    EXPECT_EQ(completions, 2);
    EXPECT_EQ(first_ec, asio::error::operation_aborted);
    EXPECT_FALSE(second_ec);
    EXPECT_EQ(EventLog::instance().count_events("first_handler"), 1);
    EXPECT_EQ(EventLog::instance().count_events("second_handler"), 1);
}
