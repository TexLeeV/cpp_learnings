// Test Suite: Resource Handles
// Estimated Time: 1-2 hours
// Difficulty: Easy
// C++ Standard: C++20

#include "instrumentation.h"

#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

class ResourceHandlesTest : public ::testing::Test
{
protected:
    static constexpr const char* kTempFile = "raii_temp_file.txt";

    void SetUp() override { EventLog::instance().clear(); }

    void TearDown() override { std::remove(kTempFile); }
};

// Move-only handle owning a Tracked* via LoggingDeleter.
class Handle
{
public:
    explicit Handle(Tracked* ptr) : ptr_(ptr, LoggingDeleter<Tracked>("HandleDeleter"))
    {
        EventLog::instance().record("Handle::acquire");
    }

    Handle(Handle&& other) noexcept : ptr_(std::move(other.ptr_))
    {
        EventLog::instance().record("Handle::move");
    }

    Handle& operator=(Handle&& other) noexcept
    {
        if (this != &other)
        {
            ptr_ = std::move(other.ptr_);
            EventLog::instance().record("Handle::move_assign");
        }
        return *this;
    }

    Tracked* get() const { return ptr_.get(); }
    explicit operator bool() const { return static_cast<bool>(ptr_); }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

private:
    std::unique_ptr<Tracked, LoggingDeleter<Tracked>> ptr_;
};

// ============================================================================
// Scenario 1: fstream Closes on Scope Exit (Easy)
// ============================================================================

TEST_F(ResourceHandlesTest, FstreamClosesOnScopeExit)
{
    {
        std::ofstream file(kTempFile);
        EXPECT_TRUE(file.is_open());
        file << "RAII content\n";

        // Q: Which RAII object owns the OS file descriptor here, and when does it close?
        // A:
        // R:
    }

    std::ifstream read_file(kTempFile);
    std::string content;
    std::getline(read_file, content);

    // Q: Why can this read succeed after `file` left scope?
    // A:
    // R:

    EXPECT_EQ(content, "RAII content");
}

// ============================================================================
// Scenario 2: Exception Still Closes File (Moderate)
// ============================================================================

TEST_F(ResourceHandlesTest, ExceptionStillClosesFile)
{
    try
    {
        std::ofstream file(kTempFile);
        EXPECT_TRUE(file.is_open());
        file << "Before exception\n";

        // Q: If the throw happens before an explicit close, what still closes the file?
        // A:
        // R:

        throw std::runtime_error("Test exception");
    }
    catch (const std::runtime_error&)
    {
    }

    std::ifstream read_file(kTempFile);
    std::string content;
    std::getline(read_file, content);
    EXPECT_EQ(content, "Before exception");
}

// ============================================================================
// Scenario 3: Move-Only Handle with LoggingDeleter (Moderate)
// ============================================================================

TEST_F(ResourceHandlesTest, HandleOwnsTrackedWithLoggingDeleter)
{
    {
        Handle handle(new Tracked("Owned"));

        // Q: Which EventLog signals show acquire vs eventual LoggingDeleter cleanup?
        // A:
        // R:

        EXPECT_TRUE(handle);
        EXPECT_EQ(handle.get()->name(), "Owned");
        EXPECT_EQ(EventLog::instance().count_events("Handle::acquire"), 1);
    }

    EXPECT_EQ(EventLog::instance().count_events("HandleDeleter::operator()"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(Owned)::dtor"), 1);
}

// ============================================================================
// Scenario 4: Move Transfers Ownership (Moderate)
// ============================================================================

TEST_F(ResourceHandlesTest, MoveTransfersOwnershipSourceEmpty)
{
    Handle source(new Tracked("Moved"));
    EventLog::instance().clear();
    Handle dest(std::move(source));

    // Q: After the move, what are `source` and `dest`, and why is Tracked still alive?
    // A:
    // R:

    EXPECT_FALSE(source);
    EXPECT_TRUE(dest);
    EXPECT_EQ(dest.get()->name(), "Moved");
    EXPECT_EQ(EventLog::instance().count_events("Handle::move"), 1);
    EXPECT_EQ(EventLog::instance().count_events("::dtor"), 0);

    // Q: How many HandleDeleter calls should fire when `dest` is destroyed?
    // A:
    // R:
}
