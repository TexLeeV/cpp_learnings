// Test Suite: Lifetime in Callbacks
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <functional>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string>

class LifetimeCallbacksTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Capturing shared_ptr Extends Lifetime (Easy)
// ============================================================================

TEST_F(LifetimeCallbacksTest, LambdaCaptureSharedPtrExtendsLifetime)
{
    std::function<void()> callback;

    {
        auto shared = std::make_shared<Tracked>("CapturedShared");
        callback = [shared]() { EventLog::instance().record("callback_ran"); };

        // Q: Right after creating the lambda, why is `shared.use_count()` 2?
        // A:
        // R:

        EXPECT_EQ(shared.use_count(), 2);
    }

    // Q: The local `shared` is gone. Why is there still no `::dtor`?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);

    callback();
    EXPECT_EQ(EventLog::instance().count_events("callback_ran"), 1u);

    callback = nullptr;

    // Q: After clearing `callback`, what EventLog signal confirms destruction?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}

// ============================================================================
// Scenario 2: Capturing weak_ptr Does Not Extend Lifetime (Moderate)
// ============================================================================

TEST_F(LifetimeCallbacksTest, LambdaCaptureWeakPtrDoesNotExtendLifetime)
{
    std::function<void()> callback;
    bool lock_ok = false;

    {
        auto shared = std::make_shared<Tracked>("CapturedWeak");
        std::weak_ptr<Tracked> weak = shared;

        callback = [weak, &lock_ok]() {
            lock_ok = static_cast<bool>(weak.lock());
            EventLog::instance().record(lock_ok ? "weak_lock_ok" : "weak_lock_failed");
        };

        // Q: After capturing `weak`, why is `shared.use_count()` still 1?
        // A:
        // R:

        EXPECT_EQ(shared.use_count(), 1);
    }

    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);

    callback();

    // Q: Why does `lock()` fail here, and which EventLog record confirms it?
    // A:
    // R:

    EXPECT_FALSE(lock_ok);
    EXPECT_EQ(EventLog::instance().count_events("weak_lock_failed"), 1u);
}

// ============================================================================
// Scenario 3: Weak Cache Lets Entries Expire (Moderate)
// ============================================================================

TEST_F(LifetimeCallbacksTest, WeakPtrCacheExpiresWithLastOwner)
{
    std::map<std::string, std::weak_ptr<Tracked>> cache;
    std::shared_ptr<Tracked> live;

    {
        auto item = std::make_shared<Tracked>("Cached");
        cache["Cached"] = item;
        live = item;

        EXPECT_EQ(item.use_count(), 2);
    }

    // Q: After the inner owner dies, why can `cache["Cached"].lock()` still succeed?
    // A:
    // R:

    EXPECT_FALSE(cache["Cached"].expired());
    EXPECT_NE(cache["Cached"].lock().get(), nullptr);

    live.reset();

    // Q: After the last strong owner resets, what do `expired()` and EventLog show?
    // A:
    // R:

    EXPECT_TRUE(cache["Cached"].expired());
    EXPECT_EQ(cache["Cached"].lock().get(), nullptr);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 1);
}
