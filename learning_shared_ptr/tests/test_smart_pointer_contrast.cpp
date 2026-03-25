#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>

class SmartPointerContrastTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(SmartPointerContrastTest, UniquePtrVsSharedPtrOwnership)
{
    long shared_count = 0;

    {
        std::unique_ptr<Tracked> unique = std::make_unique<Tracked>("Unique");
        std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("Shared");

        shared_count = shared.use_count();
        std::shared_ptr<Tracked> shared_copy = shared;

        // Q: Can you copy unique_ptr? Why or why not?
        // A:
        // R:

        // Q: What is the fundamental ownership difference between unique_ptr and shared_ptr?
        // A:
        // R:
    }

    EXPECT_EQ(shared_count, 1);
}

TEST_F(SmartPointerContrastTest, UniquePtrMoveSemantics)
{
    std::unique_ptr<Tracked> u1 = std::make_unique<Tracked>("U1");

    std::unique_ptr<Tracked> u2 = std::move(u1);

    bool u1_is_null = (u1 == nullptr);
    bool u2_is_valid = (u2 != nullptr);

    EXPECT_TRUE(u1_is_null);
    EXPECT_TRUE(u2_is_valid);
}

TEST_F(SmartPointerContrastTest, SharedPtrCopyVsUniquePtrMove)
{
    EventLog::instance().clear();

    {
        auto s1 = std::make_shared<Tracked>("S1");
        auto s2 = s1;
    }

    size_t shared_events = EventLog::instance().events().size();

    EventLog::instance().clear();

    {
        auto u1 = std::make_unique<Tracked>("U1");
        auto u2 = std::move(u1);
    }

    size_t unique_events = EventLog::instance().events().size();

    // Question: Which has more events? Why?
    EXPECT_GT(shared_events, 0);
    EXPECT_GT(unique_events, 0);
}

TEST_F(SmartPointerContrastTest, RawPointerDangerVsSmartPointer)
{
    Tracked* raw = new Tracked("Raw");

    {
        auto shared = std::make_shared<Tracked>("Shared");
    }

    // shared_ptr automatically cleaned up
    // But raw pointer is still allocated!

    bool raw_still_allocated = true;

    delete raw; // Manual cleanup required

    EXPECT_TRUE(raw_still_allocated);
}

TEST_F(SmartPointerContrastTest, CustomDeleterComparison)
{
    {
        std::unique_ptr<Tracked, LoggingDeleter<Tracked>> u(new Tracked("UniqueWithDeleter"),
                                                            LoggingDeleter<Tracked>("UniqueDeleter"));
    }

    auto unique_events = EventLog::instance().events();

    EventLog::instance().clear();

    {
        std::shared_ptr<Tracked> s(new Tracked("SharedWithDeleter"), LoggingDeleter<Tracked>("SharedDeleter"));
    }

    auto shared_events = EventLog::instance().events();

    // Question: What's the difference in deleter storage?
    EXPECT_GT(unique_events.size(), 0);
    EXPECT_GT(shared_events.size(), 0);
}

TEST_F(SmartPointerContrastTest, SharedPtrOverheadVsUnique)
{
    long shared_count = 0;

    auto shared = std::make_shared<Tracked>("Shared");

    shared_count = shared.use_count();

    auto unique = std::make_unique<Tracked>("Unique");

    // Question: What's the size difference?
    // shared_ptr: pointer + control block pointer
    // unique_ptr: just pointer (usually)

    EXPECT_EQ(shared_count, 1);
}

TEST_F(SmartPointerContrastTest, AliasingUniqueVsShared)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name) : member(name)
        {
        }
    };

    auto unique_container = std::make_unique<Container>("UniqueContainer");

    Tracked* raw_member = &unique_container->member;

    auto shared_container = std::make_shared<Container>("SharedContainer");

    std::shared_ptr<Tracked> aliased_member(shared_container, &shared_container->member);

    long alias_count = aliased_member.use_count();

    // Question: Can unique_ptr do aliasing? Why or why not?
    EXPECT_EQ(alias_count, 2);
}
