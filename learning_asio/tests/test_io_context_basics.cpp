// Test Suite: io_context Basics
// Estimated Time: 2 hours
// Difficulty: Easy
// C++ Standard: C++11 (Boost.Asio, classic callback style)
//
// The io_context is Asio's execution engine: it owns a queue of completion
// handlers and runs them on whatever thread calls run(). Nothing in Asio
// happens "in the background" by magic -- a handler only executes while some
// thread is inside run()/poll()/run_one(). These scenarios make that
// scheduling visible through EventLog.
//
// Library-evolution note: the free functions boost::asio::post/dispatch/defer
// and the io_context type itself arrived in Boost 1.66 (2017). Before that the
// type was boost::asio::io_service and you called ctx.post(handler) as a member
// function. The scheduling semantics below are the same in both eras.

#include "instrumentation.h"

#include <boost/asio.hpp>

#include <gtest/gtest.h>
#include <cstddef>
#include <string>
#include <vector>

class IoContextBasicsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: post() Queues Work That Only Runs Inside run() (Easy)
// ============================================================================

TEST_F(IoContextBasicsTest, PostQueuesHandlersUntilRunDrainsThemFifo)
{
    boost::asio::io_context ctx;

    boost::asio::post(ctx, [] { EventLog::instance().record("h1"); });
    boost::asio::post(ctx, [] { EventLog::instance().record("h2"); });
    boost::asio::post(ctx, [] { EventLog::instance().record("h3"); });

    // Q: At this point three handlers have been posted but run() has not been
    //    called. How many have executed, and what EventLog evidence confirms it?
    // A:
    // R:
    EXPECT_EQ(EventLog::instance().events().size(), 0u);

    const std::size_t executed = ctx.run();

    // Q: post() defers work to the thread that calls run(). In what order are
    //    the three handlers executed, and which observable signal proves that
    //    ordering rather than your assumption about it?
    // A:
    // R:
    const std::vector<std::string> ev = EventLog::instance().events();
    ASSERT_EQ(ev.size(), 3u);
    EXPECT_EQ(ev[0], "h1");
    EXPECT_EQ(ev[1], "h2");
    EXPECT_EQ(ev[2], "h3");

    // Q: run() returns a std::size_t. What does that number count here?
    // A:
    // R:
    EXPECT_EQ(executed, 3u);
}

// ============================================================================
// Scenario 2: run() Returns the Number of Handlers It Executed (Easy)
// ============================================================================

TEST_F(IoContextBasicsTest, RunReturnsHandlerCountAndZeroWhenIdle)
{
    boost::asio::io_context ctx;

    // Q: No work has been posted. Why does run() return immediately instead of
    //    blocking forever waiting for something to do?
    // A:
    // R:
    const std::size_t none = ctx.run();
    EXPECT_EQ(none, 0u);

    // Nothing executed, so EventLog is still empty.
    EXPECT_EQ(EventLog::instance().events().size(), 0u);
}

// ============================================================================
// Scenario 3: A Drained Context Is "stopped" Until restart() (Moderate)
// ============================================================================

TEST_F(IoContextBasicsTest, DrainedContextRequiresRestartBeforeReuse)
{
    boost::asio::io_context ctx;

    boost::asio::post(ctx, [] { EventLog::instance().record("first"); });
    const std::size_t first_run = ctx.run();
    EXPECT_EQ(first_run, 1u);
    EXPECT_EQ(EventLog::instance().count_events("first"), 1u);

    // Q: After run() drains all work, what does ctx.stopped() report, and what
    //    does that imply for the next call to run()?
    // A:
    // R:
    EXPECT_TRUE(ctx.stopped());

    // Post more work, then try to run WITHOUT restarting first.
    boost::asio::post(ctx, [] { EventLog::instance().record("second"); });
    const std::size_t reused_without_restart = ctx.run();

    // Q: "second" was posted before this run(). Why does run() return 0 and why
    //    does EventLog show "second" did NOT execute yet?
    // A:
    // R:
    EXPECT_EQ(reused_without_restart, 0u);
    EXPECT_EQ(EventLog::instance().count_events("second"), 0u);

    // restart() clears the stopped state so the context can run again. The work
    // posted earlier was never discarded -- it was only waiting for a live run.
    ctx.restart();
    const std::size_t after_restart = ctx.run();

    // Q: run() now returns 1 and "second" finally executes. What single call
    //    made the difference, and what state did it reset?
    // A:
    // R:
    EXPECT_EQ(after_restart, 1u);
    EXPECT_EQ(EventLog::instance().count_events("second"), 1u);
}

// ============================================================================
// Scenario 4: dispatch() May Run Inline; post() Always Defers (Moderate)
// ============================================================================

TEST_F(IoContextBasicsTest, DispatchRunsInlineWhilePostDefers)
{
    boost::asio::io_context ctx;

    // The outer handler runs while we are inside ctx.run() below. From inside
    // it, we both post() and dispatch() additional handlers.
    boost::asio::post(ctx, [&ctx] {
        EventLog::instance().record("outer-start");
        boost::asio::post(ctx, [] { EventLog::instance().record("posted"); });
        boost::asio::dispatch(ctx, [] { EventLog::instance().record("dispatched"); });
        EventLog::instance().record("outer-end");
    });

    ctx.run();

    // Q: dispatch() is allowed to invoke its handler immediately when the caller
    //    is already running on the io_context. Given that, where does
    //    "dispatched" fall relative to "outer-end", and where does "posted"
    //    fall? Predict the full order before reading the expectations below.
    // A:
    // R:
    const std::vector<std::string> ev = EventLog::instance().events();
    ASSERT_EQ(ev.size(), 4u);
    EXPECT_EQ(ev[0], "outer-start");
    EXPECT_EQ(ev[1], "dispatched");
    EXPECT_EQ(ev[2], "outer-end");
    EXPECT_EQ(ev[3], "posted");

    // Q: What observable difference would you expect if "dispatched" were
    //    changed to another post()? Which two entries would swap?
    // A:
    // R:

    // TODO (learner): Change the inner boost::asio::dispatch(...) to
    // boost::asio::post(...) and re-run. Record the new order in the // A:
    // lines above and explain why dispatch's inline shortcut disappeared.
}

// ============================================================================
// Scenario 5: poll() Runs Only Ready Work and Returns Without Blocking (Hard)
// ============================================================================

TEST_F(IoContextBasicsTest, PollRunsReadyHandlersThenReturns)
{
    boost::asio::io_context ctx;

    // A handler that, when run, posts a *second* handler. The second one is not
    // "ready" until the first one has executed and queued it.
    boost::asio::post(ctx, [&ctx] {
        EventLog::instance().record("stage1");
        boost::asio::post(ctx, [] { EventLog::instance().record("stage2"); });
    });

    // Q: poll() executes all handlers that are ready *right now* and then
    //    returns. "stage2" is only queued once "stage1" runs. Does a single
    //    poll() reach "stage2"? Predict the count it returns.
    // A:
    // R:
    const std::size_t first_poll = ctx.poll();

    // Both stage1 and stage2 ran in this poll: once stage1 executed, stage2 was
    // queued and was still "ready" within the same poll() pass.
    EXPECT_EQ(first_poll, 2u);
    EXPECT_EQ(EventLog::instance().count_events("stage1"), 1u);
    EXPECT_EQ(EventLog::instance().count_events("stage2"), 1u);

    // Q: poll() differs from run() in what it does when *no* handler is ready.
    //    With the queue now empty, what does a second poll() return, and why is
    //    that the key behavioral contrast with run() in a blocking design?
    // A:
    // R:
    ctx.restart();
    const std::size_t second_poll = ctx.poll();
    EXPECT_EQ(second_poll, 0u);

    // TODO (learner): Replace the first poll() with run() and confirm the
    // EventLog counts are identical here. Then describe one situation (hint:
    // long-lived asynchronous work such as a timer or socket) where run() and
    // poll() would behave very differently. Record your answer in // A: above.
}
