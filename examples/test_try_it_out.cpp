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
    
    // TODO: Create p2 by moving from p1 using std::move
    std::shared_ptr<Tracked> p2;
    // p2 = ???
    
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

/*
 * CUSTOMIZING YOUR LEARNING EXPERIENCE
 * =====================================
 * 
 * Now that you've tried the Socratic method, you can customize how it works:
 * 
 * - Pacing: Tell the AI "one test at a time" or "bulk mode" or "self-directed"
 * - Hints: Tell the AI "offer hints when I'm stuck" or "no hints unless I ask"
 * - Feedback: Tell the AI "chat-only feedback" or keep the inline Q/A/R pattern
 * - Response depth: Tell the AI "beginner-friendly" or "precise technical" (default)
 * - Verification: Tell the AI "relax verification" if you find file-reading slow
 * 
 * See .cursor/rules/socratic-software-engineering.mdc for full preference details.
 */
