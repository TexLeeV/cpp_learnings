#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class TryItOutTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(TryItOutTest, CopyingSharedPtr)
{
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Original");
    
    // Q: What is the use_count of p1 at this point?
    // A:
    // R:
    
    std::shared_ptr<Tracked> p2 = p1;
    
    // Q: After copying p1 to p2, what is the use_count of both p1 and p2?
    // A:
    // R:
    
    // Q: How many Tracked objects exist in memory right now?
    // A:
    // R:
    
    auto events = EventLog::instance().events();
    EXPECT_EQ(p1.use_count(), 2);
    EXPECT_EQ(p2.use_count(), 2);
    EXPECT_EQ(events.size(), 1);
}

TEST_F(TryItOutTest, MovingSharedPtr)
{
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Original");
    
    std::shared_ptr<Tracked> p2 = std::move(p1);
    
    // Q: After the move, what is the use_count of p2?
    // A:
    // R:
    
    // Q: After the move, what is the use_count of p1?
    // A:
    // R:
    
    // Q: Why does the moved-from p1 have use_count 0 but still exist as a valid variable?
    // A:
    // R:
    
    auto events = EventLog::instance().events();
    EXPECT_EQ(p2.use_count(), 1);
    EXPECT_EQ(p1.use_count(), 0);
    EXPECT_EQ(events.size(), 1);
}
