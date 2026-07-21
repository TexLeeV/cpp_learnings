// Test Suite: Asio Async Composition
// Estimated Time: 2 hours
// Difficulty: Moderate
// Library: standalone Asio (asio::) — not std::asio
// C++ Standard: C++20

#include "instrumentation.h"

#include <asio.hpp>
#include <gtest/gtest.h>
#include <memory>

class AsyncCompositionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Chained Timers (Easy)
// ============================================================================

TEST_F(AsyncCompositionTest, ChainTwoTimersSequentially)
{
    asio::io_context io;
    auto t1 = std::make_shared<asio::steady_timer>(io);
    auto t2 = std::make_shared<asio::steady_timer>(io);

    t1->expires_after(std::chrono::milliseconds(5));
    t1->async_wait([t1, t2](const asio::error_code& ec) {
        EXPECT_FALSE(ec);
        EventLog::instance().record("first");
        t2->expires_after(std::chrono::milliseconds(5));
        t2->async_wait([](const asio::error_code& ec2) {
            EXPECT_FALSE(ec2);
            EventLog::instance().record("second");
        });
    });

    io.run();

    // Q: Why must `t1`/`t2` outlive the first completion handler, and what
    //    keeps them alive across the async boundary here?
    // A:
    // R:

    // Q: What EventLog order proves the second timer was armed only after the
    //    first completed?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("first"), 1);
    EXPECT_EQ(EventLog::instance().count_events("second"), 1);
    const auto events = EventLog::instance().events();
    ASSERT_EQ(events.size(), 2u);
    EXPECT_NE(events[0].find("first"), std::string::npos);
    EXPECT_NE(events[1].find("second"), std::string::npos);
}

// ============================================================================
// Scenario 2: post() Defers Work Onto the io_context (Easy)
// ============================================================================

TEST_F(AsyncCompositionTest, PostDefersWorkOntoContext)
{
    asio::io_context io;
    bool ran_inline = false;

    asio::post(io, [&]() {
        ran_inline = true;
        EventLog::instance().record("posted");
    });

    // Q: Why is `ran_inline` still false immediately after `post`, and which
    //    call actually executes the function object?
    // A:
    // R:

    EXPECT_FALSE(ran_inline);
    io.run();
    EXPECT_TRUE(ran_inline);
    EXPECT_EQ(EventLog::instance().count_events("posted"), 1);
}

// ============================================================================
// Scenario 3: strand Serializes Handlers (Moderate)
// ============================================================================

TEST_F(AsyncCompositionTest, StrandSerializesHandlers)
{
    asio::io_context io;
    auto strand = asio::make_strand(io);

    int counter = 0;

    auto bump = [&]() {
        const int seen = counter;
        EventLog::instance().record("bump_" + std::to_string(seen));
        // Cooperative "yield" point: without a strand, concurrent run()
        // threads could interleave here. With a strand, handlers are
        // non-concurrent.
        ++counter;
    };

    for (int i = 0; i < 4; ++i)
    {
        asio::post(strand, bump);
    }

    io.run();

    // Q: What guarantee does a strand provide about concurrent execution of
    //    handlers posted through it?
    // A:
    // R:

    EXPECT_EQ(counter, 4);
    EXPECT_EQ(EventLog::instance().count_events("bump_"), 4);
}

// ============================================================================
// Scenario 4: bind_executor Pins Handler to a Strand (Moderate)
// ============================================================================

TEST_F(AsyncCompositionTest, BindExecutorPinsTimerHandler)
{
    asio::io_context io;
    auto strand = asio::make_strand(io);
    asio::steady_timer timer(io, std::chrono::milliseconds(5));

    int hits = 0;
    timer.async_wait(asio::bind_executor(strand, [&](const asio::error_code& ec) {
        EXPECT_FALSE(ec);
        ++hits;
        EventLog::instance().record("bound_handler");
    }));

    // Q: How does `bind_executor` change where/how the timer completion is
    //    invoked relative to posting the handler unbound?
    // A:
    // R:

    io.run();
    EXPECT_EQ(hits, 1);
    EXPECT_EQ(EventLog::instance().count_events("bound_handler"), 1);
}
