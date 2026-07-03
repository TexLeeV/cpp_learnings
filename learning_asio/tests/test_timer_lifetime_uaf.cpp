// Test Suite: Timer Handler Lifetime -- Use-After-Free and How to Fix It
// Estimated Time: 3-4 hours
// Difficulty: Hard
// C++ Standard: C++11 (Boost.Asio, classic callback style)
//
// THE HAZARD
// ----------
// An asynchronous operation outlives the statement that started it. A timer's
// completion handler runs *later*, when some thread is inside io_context::run().
// If that handler captured a raw `this` (or a reference/pointer into an object)
// and the object is destroyed before the handler runs, the handler dereferences
// freed memory: a use-after-free.
//
// The trap is sharpened by a fact from test_timers.cpp: destroying a timer (for
// example because its owner is being destroyed) does NOT silently drop the
// pending wait. It CANCELS it, which *queues the handler to run with
// boost::asio::error::operation_aborted*. So the handler still runs -- now with
// a dangling `this`. A handler that touches members before (or instead of)
// checking the error_code walks straight off the cliff.
//
// HOW TO REPRODUCE THE CRASH (the disabled test below)
// ----------------------------------------------------
// The reproduction is marked DISABLED_ so the normal suite stays green. To see
// the failure the way you would when triaging a legacy crash, run it under the
// AddressSanitizer preset:
//
//   cmake --preset gcc-asan
//   cmake --build --preset gcc-asan --target test_timer_lifetime_uaf
//   ./build/gcc-asan/learning_asio/test_timer_lifetime_uaf \
//       --gtest_also_run_disabled_tests \
//       --gtest_filter='*RawThisHandlerUseAfterFree*'
//
// AddressSanitizer prints "heap-use-after-free", a READ of size 4, and -- most
// usefully -- names the lambda inside RawThisReporter::start() as the access
// site, plus the "freed by" stack (the owner's destruction). That report is the
// thread you pull on to fix the real bug.

#include "instrumentation.h"

#include <boost/asio.hpp>

#include <gtest/gtest.h>
#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class TimerLifetimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// BROKEN owner: captures a raw `this` into its timer handler.
// ============================================================================

class RawThisReporter
{
public:
    explicit RawThisReporter(boost::asio::io_context& ctx)
    : timer_(ctx, std::chrono::milliseconds(1))
    , tick_count_(nullptr)
    {
        tick_count_ = std::make_shared<int>(5);
    }

    ~RawThisReporter()
    {
        // Destroying the member timer cancels the pending async_wait, which
        // QUEUES on_tick() to run later with operation_aborted -- after `this`
        // is gone.
        EventLog::instance().record("RawThisReporter::dtor");
    }

    void start()
    {
        // The defect: the handler captures the bare `this` pointer. Its
        // lifetime is now silently coupled to an object it does not own.
        timer_.async_wait([this](const boost::system::error_code& ec) { on_tick(ec); });
    }

private:
    void on_tick(const boost::system::error_code& /*ec*/)
    {
        // Legacy-style handler: it touches members BEFORE consulting the
        // error_code. By the time this runs, `this` may already be freed, so
        // every access below is undefined behavior.
        ++(*tick_count_);
        EventLog::instance().record("RawThisReporter::on_tick magic=" + std::to_string(magic_));
    }

    boost::asio::steady_timer timer_;
    int magic_ = 0x1234;
    std::shared_ptr<int> tick_count_;
};

// ============================================================================
// Scenario 1: Reproduce the Use-After-Free (Hard) -- DISABLED by default
// ============================================================================

TEST_F(TimerLifetimeTest, DISABLED_RawThisHandlerUseAfterFreeWhenOwnerDiesFirst)
{
    boost::asio::io_context ctx;

    RawThisReporter* reporter = new RawThisReporter(ctx);
    reporter->start();

    // Destroy the owner while the wait is still pending. This cancels the timer,
    // which queues on_tick() to run with operation_aborted -- now pointing at
    // freed memory.
    delete reporter;

    // Q: This test has no EXPECT assertions -- its only "result" is the runtime
    //    report. Under the AddressSanitizer preset, what access triggers the
    //    heap-use-after-free, and which source line does the "freed by" stack
    //    name as the cause?
    // A:
    // R:
    ctx.run();
}

// ============================================================================
// FIX 1: enable_shared_from_this -- the handler co-owns the object.
// ============================================================================

class SharedReporter : public std::enable_shared_from_this<SharedReporter>
{
public:
    explicit SharedReporter(boost::asio::io_context& ctx)
        : timer_(ctx, std::chrono::milliseconds(1))
    {
    }

    ~SharedReporter()
    {
        EventLog::instance().record("SharedReporter::dtor");
    }

    void start()
    {
        // Capture a shared_ptr to self. While the handler is queued, this
        // shared_ptr keeps the object alive -- the object cannot be destroyed
        // until the handler (and thus the captured shared_ptr) is gone.
        std::shared_ptr<SharedReporter> self = shared_from_this();
        timer_.async_wait([self](const boost::system::error_code& /*ec*/) {
            EventLog::instance().record("SharedReporter::on_tick");
        });
    }

private:
    boost::asio::steady_timer timer_;
};

// ============================================================================
// Scenario 2: shared_from_this Extends Lifetime Until the Handler Runs (Hard)
// ============================================================================

TEST_F(TimerLifetimeTest, SharedFromThisExtendsLifetimeUntilHandlerCompletes)
{
    boost::asio::io_context ctx;

    {
        std::shared_ptr<SharedReporter> reporter = std::make_shared<SharedReporter>(ctx);
        reporter->start();
        // Our local reference goes out of scope HERE -- but the object does not
        // die, because the queued handler holds its own shared_ptr copy.
    }

    EventLog::instance().record("before-run");

    // Q: At this point our only named reference to the reporter is gone. Why is
    //    the object still alive, and what exactly is keeping it alive?
    // A:
    // R:
    ctx.run();

    // The decisive observable: the handler ran, and the destructor ran AFTER
    // it -- never before.
    const std::vector<std::string> ev = EventLog::instance().events();
    ASSERT_EQ(ev.size(), 3u);
    EXPECT_EQ(ev[0], "before-run");
    EXPECT_EQ(ev[1], "SharedReporter::on_tick");
    EXPECT_EQ(ev[2], "SharedReporter::dtor");

    // Q: The destructor entry appears immediately after on_tick, not at end of
    //    scope above. What event drops the final reference count to zero and
    //    triggers destruction right there?
    // A:
    // R:

    // TODO (learner): Change SharedReporter::start() to capture a raw `this`
    // instead of `self` (as RawThisReporter does), then re-run under the gcc-asan
    // preset. Record what changes in the // A: lines above and explain why the
    // "dtor runs after on_tick" ordering can no longer be guaranteed.
}

// ============================================================================
// FIX 2: weak_ptr guard -- let the owner die, but detect it safely.
// ============================================================================

class GuardedReporter : public std::enable_shared_from_this<GuardedReporter>
{
public:
    explicit GuardedReporter(boost::asio::io_context& ctx)
        : timer_(ctx, std::chrono::seconds(60))
    {
    }

    ~GuardedReporter()
    {
        EventLog::instance().record("GuardedReporter::dtor");
    }

    void start()
    {
        // Capture a weak_ptr, not a shared_ptr. This deliberately does NOT keep
        // the object alive -- it lets the owner be destroyed on schedule, but
        // gives the handler a safe way to ask "are you still there?".
        std::weak_ptr<GuardedReporter> weak_self = shared_from_this();
        timer_.async_wait([weak_self](const boost::system::error_code& /*ec*/) {
            std::shared_ptr<GuardedReporter> self = weak_self.lock();
            if (!self)
            {
                EventLog::instance().record("GuardedReporter::owner-gone-bail");
                return;
            }
            EventLog::instance().record("GuardedReporter::touched-owner");
        });
    }

private:
    boost::asio::steady_timer timer_;
};

// ============================================================================
// Scenario 3: weak_ptr Lets the Owner Die and the Handler Bails Safely (Hard)
// ============================================================================

TEST_F(TimerLifetimeTest, WeakPtrGuardLetsOwnerDieAndHandlerBailsSafely)
{
    boost::asio::io_context ctx;

    std::shared_ptr<GuardedReporter> reporter = std::make_shared<GuardedReporter>(ctx);
    reporter->start();

    // Destroy the owner while the wait is pending. Unlike FIX 1, nothing keeps
    // it alive, so the destructor runs now; the timer is cancelled and the
    // handler is queued with operation_aborted.
    reporter.reset();

    // Q: This is the exact setup that crashed RawThisReporter: owner dead,
    //    handler still queued. Why is it safe here? What does weak_self.lock()
    //    return now, and what does the handler do as a result?
    // A:
    // R:
    ctx.run();

    const std::vector<std::string> ev = EventLog::instance().events();
    ASSERT_EQ(ev.size(), 2u);
    EXPECT_EQ(ev[0], "GuardedReporter::dtor");
    EXPECT_EQ(ev[1], "GuardedReporter::owner-gone-bail");
    EXPECT_EQ(EventLog::instance().count_events("GuardedReporter::touched-owner"), 0u);

    // Q: Compare the two fixes. shared_from_this guarantees the work completes
    //    by keeping the object alive; weak_ptr lets the object die and abandons
    //    the work. For a periodic health-reporter being shut down, which
    //    semantics do you want, and what observable signal would you assert to
    //    prove you got it?
    // A:
    // R:

    // TODO (learner): Change GuardedReporter::start() to capture a shared_ptr
    // (as SharedReporter does) instead of a weak_ptr, then re-run. Record which
    // EventLog entries change -- especially the dtor's position relative to the
    // handler -- in the // A: lines above, and explain why the
    // "owner-gone-bail" path can no longer be reached.
}
