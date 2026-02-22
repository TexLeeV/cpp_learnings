#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>

class DeallocationLifetimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

TEST_F(DeallocationLifetimeTest, BasicDestructionOrder)
{
    {
        std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Obj1");
        std::shared_ptr<Tracked> p2 = std::make_shared<Tracked>("Obj2");
        
        // Q: At what point in the code will Obj1's destructor be called?
        // A:
        // R:
        
        // Q: At what point in the code will Obj2's destructor be called?
        // A:
        // R:
        
        // Q: Which object will be destroyed first, Obj1 or Obj2, and why?
        // A:
        // R:
        
        // Q: The test expects 4 events. What are these 4 events?
        // A:
        // R:
    }
    
    auto events = EventLog::instance().events();
    size_t event_count = events.size();
    
    EXPECT_EQ(event_count, 4);
}

TEST_F(DeallocationLifetimeTest, CustomDeleter)
{
    {
        std::shared_ptr<Tracked> p1(new Tracked("Obj1"), LoggingDeleter<Tracked>("CustomDeleter"));
        
        // Q: When will the custom deleter be invoked?
        // A:
        // R:
        
        // Q: What is the difference between the deleter stored in the control block and the default deleter used by make_shared?
        // A:
        // R:
        
        // Q: Can you change the deleter after the shared_ptr is constructed?
        // A:
        // R:
    }
    
    auto events = EventLog::instance().events();
    bool deleter_called = false;
    
    for (const auto& event : events)
    {
        if (event.find("CustomDeleter::operator()") != std::string::npos)
        {
            deleter_called = true;
            break;
        }
    }
    
    EXPECT_TRUE(deleter_called);
}

TEST_F(DeallocationLifetimeTest, ArrayDeleter)
{
    {
        std::shared_ptr<Tracked> p1(new Tracked[3]{Tracked("D1"), Tracked("D2"), Tracked("D3")}, LoggingArrayDeleter<Tracked>("ArrayDeleter"));
        
        // Q: Why is a custom deleter necessary when managing arrays with shared_ptr?
        // A:
        // R:
        
        // Q: What would happen if you used the default deleter (delete) instead of delete[] for an array?
        // A:
        // R:
        
        // Q: In what order will the three array elements be destroyed?
        // A:
        // R:
    }
    
    auto events = EventLog::instance().events();
    bool array_deleter_called = false;
    
    for (const auto& event : events)
    {
        if (event.find("ArrayDeleter::operator()") != std::string::npos)
        {
            array_deleter_called = true;
            break;
        }
    }
    
    EXPECT_TRUE(array_deleter_called);
}

TEST_F(DeallocationLifetimeTest, ResetWithCustomDeleter)
{
    std::shared_ptr<Tracked> p1(new Tracked("Obj1"), LoggingDeleter<Tracked>("FirstDeleter"));
    
    EventLog::instance().clear();
    
    p1.reset(new Tracked("Obj2"), LoggingDeleter<Tracked>("SecondDeleter"));
    
    // Q: Which deleter was invoked when reset() was called, FirstDeleter or SecondDeleter?
    // A:
    // R:
    
    // Q: What happened to the control block associated with the first object?
    // A:
    // R:
    
    // Q: After reset(), which deleter is stored in p1's control block?
    // A:
    // R:
    
    // Q: When will SecondDeleter be invoked?
    // A:
    // R:
    
    auto events = EventLog::instance().events();
    bool first_deleter_called = false;
    
    for (const auto& event : events)
    {
        if (event.find("FirstDeleter::operator()") != std::string::npos)
        {
            first_deleter_called = true;
            break;
        }
    }
    
    EXPECT_TRUE(first_deleter_called);
}

TEST_F(DeallocationLifetimeTest, SharedOwnershipDeleterInvocation)
{
    std::shared_ptr<Tracked> p1(new Tracked("SharedObj"), LoggingDeleter<Tracked>("SharedDeleter"));
    
    std::shared_ptr<Tracked> p2 = p1;
    std::shared_ptr<Tracked> p3 = p1;
    
    // Q: How many shared_ptr instances share ownership of the object at this point?
    // A:
    // R:
    
    // Q: How many copies of the deleter exist?
    // A:
    // R:
    
    EventLog::instance().clear();
    
    p1.reset();
    
    // Q: Why wasn't the deleter invoked when p1 was reset?
    // A:
    // R:
    
    // Q: What is the reference count after p1.reset()?
    // A:
    // R:
    
    auto events_after_first = EventLog::instance().events();
    bool deleter_called_early = false;
    
    for (const auto& event : events_after_first)
    {
        if (event.find("SharedDeleter::operator()") != std::string::npos)
        {
            deleter_called_early = true;
            break;
        }
    }
    
    p2.reset();
    p3.reset();
    
    // Q: After which reset() call was the deleter invoked?
    // A:
    // R:
    
    // Q: What determines when the deleter stored in the control block is invoked?
    // A:
    // R:
    
    auto events_after_all = EventLog::instance().events();
    bool deleter_called_final = false;
    
    for (const auto& event : events_after_all)
    {
        if (event.find("SharedDeleter::operator()") != std::string::npos)
        {
            deleter_called_final = true;
            break;
        }
    }
    
    EXPECT_FALSE(deleter_called_early);
    EXPECT_TRUE(deleter_called_final);
}

TEST_F(DeallocationLifetimeTest, AliasingConstructorLifetime)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };
    
    EventLog::instance().clear();
    
    {
        std::shared_ptr<Container> owner(new Container("ContainerObj"), LoggingDeleter<Container>("ContainerDeleter"));
        
        std::shared_ptr<Tracked> alias(owner, &owner->member);
        
        // Q: What does alias point to?
        // A:
        // R:
        
        // Q: Which control block does alias share with owner?
        // A:
        // R:
        
        // Q: Which deleter will be invoked when the last reference is released, the one for Container or the one for Tracked?
        // A:
        // R:
        
        owner.reset();
        
        // Q: Why wasn't the Container deleted when owner was reset?
        // A:
        // R:
        
        // Q: What is keeping the Container alive?
        // A:
        // R:
        
        // Q: What happens to the Tracked member when the Container is eventually deleted?
        // A:
        // R:
        
        auto events_after_owner_reset = EventLog::instance().events();
        bool container_deleted = false;
        
        for (const auto& event : events_after_owner_reset)
        {
            if (event.find("ContainerDeleter::operator()") != std::string::npos)
            {
                container_deleted = true;
                break;
            }
        }
        
        EXPECT_FALSE(container_deleted);
    }
    
    auto events_final = EventLog::instance().events();
    bool container_deleted_final = false;
    
    for (const auto& event : events_final)
    {
        if (event.find("ContainerDeleter::operator()") != std::string::npos)
        {
            container_deleted_final = true;
            break;
        }
    }
    
    EXPECT_TRUE(container_deleted_final);
}

TEST_F(DeallocationLifetimeTest, NullptrReset)
{
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Obj1");
    
    EventLog::instance().clear();
    
    p1.reset();
    
    // Q: What does p1 point to after reset() with no arguments?
    // A:
    // R:
    
    // Q: What happened to the object that p1 was managing?
    // A:
    // R:
    
    // Q: Is reset() with no arguments equivalent to p1 = nullptr?
    // A:
    // R:
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    EXPECT_EQ(dtor_count, 1);
}

TEST_F(DeallocationLifetimeTest, MultipleResetsInSequence)
{
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Initial");
    
    EventLog::instance().clear();
    
    p1.reset(new Tracked("First"));
    p1.reset(new Tracked("Second"));
    p1.reset(new Tracked("Third"));
    
    // Q: How many objects were destroyed during the three reset() calls?
    // A:
    // R:
    
    // Q: When was each object destroyed?
    // A:
    // R:
    
    // Q: After the three reset() calls, which object does p1 manage?
    // A:
    // R:
    
    // Q: What would happen if you added a fourth reset() with no arguments?
    // A:
    // R:
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    
    EXPECT_EQ(dtor_count, 3);
}
