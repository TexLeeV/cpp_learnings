// Test Suite: Asio Timers
// Estimated Time: 2-3 hours
// Difficulty: Easy -> Moderate
// C++ Standard: C++11 (Boost.Asio, classic callback style)
//
// A steady_timer is the simplest asynchronous operation in Asio: it completes
// after a duration elapses, or earlier if it is cancelled. Its completion
// handler has the signature void(const boost::system::error_code&). The
// error_code is the channel through which Asio reports *why* the handler ran --
// natural expiry vs. cancellation -- so reading it is the whole point.
//
// Key idea reinforced throughout: a cancelled asynchronous operation does NOT
// silently vanish. Its handler is still invoked, but with
// boost::asio::error::operation_aborted. Handlers are a promise the library
// always keeps.

#include "instrumentation.h"

#include <boost/asio.hpp>

#include <gtest/gtest.h>
#include <chrono>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

class TimerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: A Timer That Expires Reports Success (Easy)
// ============================================================================

TEST_F(TimerTest, ExpiredTimerInvokesHandlerWithSuccessCode)
{
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx, std::chrono::milliseconds(1));

    boost::system::error_code observed;
    timer.async_wait([&observed](const boost::system::error_code& ec) {
        observed = ec;
        EventLog::instance().record("timer:fired");
    });

    // Q: At this line the handler has been *registered* but not run. What must
    //    happen before "timer:fired" can appear in EventLog?
    // A:
    // R:
    EXPECT_EQ(EventLog::instance().count_events("timer:fired"), 0u);

    ctx.run();

    // Q: The timer expired normally. What does the error_code passed to the
    //    handler evaluate to in a boolean context, and what is its numeric
    //    value() for a successful wait?
    // A:
    // R:
    EXPECT_EQ(EventLog::instance().count_events("timer:fired"), 1u);
    EXPECT_FALSE(observed);
    EXPECT_EQ(observed.value(), 0);
}

// ============================================================================
// Scenario 2: Cancelling a Pending Timer Still Runs Its Handler (Easy->Moderate)
// ============================================================================

TEST_F(TimerTest, CancelDeliversOperationAbortedNotSilence)
{
    boost::asio::io_context ctx;

    // Arm the timer far in the future so it cannot expire on its own during the
    // test. We will cancel it before it ever has the chance.
    boost::asio::steady_timer timer(ctx, std::chrono::seconds(60));

    boost::system::error_code observed;
    timer.async_wait([&observed](const boost::system::error_code& ec) {
        observed = ec;
        EventLog::instance().record("timer:handler-ran");
    });

    const std::size_t cancelled = timer.cancel();

    // Q: cancel() returns how many pending operations it stopped here, and at
    //    this exact point (before run()) has the handler executed yet?
    // A:
    // R:
    EXPECT_EQ(cancelled, 1u);
    EXPECT_EQ(EventLog::instance().count_events("timer:handler-ran"), 0u);

    ctx.run();

    // Q: run() returns almost immediately rather than waiting 60 seconds, and
    //    the handler DID run. What error_code did it receive, and what does
    //    that tell you about how Asio treats cancellation versus dropping work?
    // A:
    // R:
    EXPECT_EQ(EventLog::instance().count_events("timer:handler-ran"), 1u);
    EXPECT_TRUE(observed);
    EXPECT_EQ(observed, boost::asio::error::operation_aborted);

    // TODO (learner): Many real handlers begin with
    //     if (ec == boost::asio::error::operation_aborted) return;
    // Explain in // A: above why omitting that early-return is a common source
    // of use-after-free bugs when the object owning the timer is being torn
    // down. What observable signal would betray such a bug?
}

// ============================================================================
// Scenario 3: Multiple Timers Complete in Expiry Order, Not Arming Order (Moderate)
// ============================================================================

TEST_F(TimerTest, MultipleTimersFireInExpiryOrder)
{
    boost::asio::io_context ctx;

    // Note the arming order: "slow" is created and waited on FIRST, but is set
    // to expire LATER than "fast".
    boost::asio::steady_timer slow(ctx, std::chrono::milliseconds(40));
    boost::asio::steady_timer fast(ctx, std::chrono::milliseconds(10));

    slow.async_wait([](const boost::system::error_code&) {
        EventLog::instance().record("slow");
    });
    fast.async_wait([](const boost::system::error_code&) {
        EventLog::instance().record("fast");
    });

    ctx.run();

    // Q: Both handlers run, but in which order? Is the ordering decided by the
    //    sequence in which async_wait was called, or by expiry time? Which
    //    EventLog entries are your evidence?
    // A:
    // R:
    const std::vector<std::string> ev = EventLog::instance().events();
    ASSERT_EQ(ev.size(), 2u);
    EXPECT_EQ(ev[0], "fast");
    EXPECT_EQ(ev[1], "slow");
}

// ============================================================================
// Scenario 4: A Timer That Re-Arms Itself From Its Own Handler (Hard)
// ============================================================================

TEST_F(TimerTest, RecurringTimerReArmsFromWithinItsHandler)
{
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer(ctx);

    int ticks = 0;

    // C++11 self-referential handler: store the std::function first, then let
    // the lambda capture a reference to it so it can re-register itself. (A
    // lambda cannot name itself directly until C++23's "deducing this".)
    std::function<void(const boost::system::error_code&)> on_tick;
    on_tick = [&](const boost::system::error_code& ec) {
        if (ec)
        {
            return;
        }
        EventLog::instance().record("tick");
        ++ticks;
        if (ticks < 3)
        {
            timer.expires_after(std::chrono::milliseconds(1));
            timer.async_wait(on_tick);
        }
    };

    timer.expires_after(std::chrono::milliseconds(1));
    timer.async_wait(on_tick);

    // Q: A single io_context with a single timer produces a *repeating* effect.
    //    There is no loop in this test driving it. What keeps run() from
    //    returning after the first tick, and what makes it finally return?
    // A:
    // R:
    ctx.run();

    EXPECT_EQ(ticks, 3);
    EXPECT_EQ(EventLog::instance().count_events("tick"), 3u);

    // Q: The handler re-arms the timer only while ticks < 3. If that guard were
    //    removed, what would happen to ctx.run(), and what EventLog growth
    //    pattern would you observe before you killed the process?
    // A:
    // R:

    // TODO (learner): Change the re-arm guard to ticks < 5 and predict the new
    // count BEFORE rebuilding. Record your prediction in // A: above, then
    // verify it. Then explain why this "re-arm from the handler" shape is the
    // standard way to build a periodic timer in callback-style Asio.
}
