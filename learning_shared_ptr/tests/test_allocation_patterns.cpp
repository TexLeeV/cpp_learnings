#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <cstdio>
class AllocationPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};
TEST_F(AllocationPatternsTest, MakeSharedVsNew)
{
    long new_count = 0;
    long make_shared_count = 0;
    // Q: When you create a shared_ptr using `new`, how many heap allocations occur?
    // A:
    // R:
    std::shared_ptr<Tracked> p1(new Tracked("New"));
    new_count = p1.use_count();
    // Q: When you create a shared_ptr using `make_shared`, how many heap allocations occur?
    // A:
    // R:
    std::shared_ptr<Tracked> p2 = std::make_shared<Tracked>("MakeShared");
    make_shared_count = p2.use_count();
    // Q: Both use_count values are 1. What does this tell you about the relationship between allocation strategy and reference counting?
    // A:
    // A:
    // R:
    // R:
    // R:
    EXPECT_EQ(new_count, 1);
    EXPECT_EQ(make_shared_count, 1);
    // Q: Given that `new` requires 2 allocations (object + control block) and `make_shared` requires 1 (combined), what are the performance implications?
    // A:
    // R:
    // Q: What happens to cache locality when the object and control block are allocated separately vs together?
    // A:
    // R:
}
class ThrowingResource
{
public:
    explicit ThrowingResource(bool should_throw)
    : tracked_("Throwing")
    {
        if (should_throw)
        {
            throw std::runtime_error("Construction failed");
        }
    }
private:
    Tracked tracked_;
};
void process_with_new(bool throw_first, bool throw_second)
{
    try
    {
        std::shared_ptr<ThrowingResource> p1(new ThrowingResource(throw_first));
        std::shared_ptr<ThrowingResource> p2(new ThrowingResource(throw_second));
    }
    catch (...)
    {
    }
}
void process_with_make_shared(bool throw_first, bool throw_second)
{
    try
    {
        std::shared_ptr<ThrowingResource> p1 = std::make_shared<ThrowingResource>(throw_first);
        std::shared_ptr<ThrowingResource> p2 = std::make_shared<ThrowingResource>(throw_second);
    }
    catch (...)
    {
    }
}
TEST_F(AllocationPatternsTest, ExceptionSafetyMakeShared)
{
    EventLog::instance().clear();
    // Q: When p2's construction throws, what happens to p1? Walk through the stack unwinding.
    // A:
    // R:
    // Q: At what point in the statement `std::shared_ptr<T> p1(new T(true))` could an exception leave a memory leak?
    // A:
    // R:
    // R:
    process_with_make_shared(false, true);
    auto events = EventLog::instance().events();
    size_t ctor_count = 0;
    size_t dtor_count = 0;
    for (const auto& event : events)
    {
        if (event.find("::ctor") != std::string::npos)
        {
            ++ctor_count;
        }
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    // Q: The test expects ctor_count == dtor_count. What does this verify about exception safety?
    // A:
    // R:
    EXPECT_EQ(ctor_count, dtor_count);
}
struct FileDeleter
{
    void operator()(FILE* file) const
    {
        if (file)
        {
            EventLog::instance().record("FileDeleter::operator() closing file");
            std::fclose(file);
        }
    }
};
TEST_F(AllocationPatternsTest, CustomDeleterFileHandle)
{
    {
        FILE* file = std::tmpfile();
        // Q: Why can't you use make_shared with a custom deleter?
        // A:
        // R:
        // R:
        // Q: Where is the FileDeleter stored—in the control block or with the shared_ptr object itself?
        // A:
        // R:
        // R:
        std::shared_ptr<FILE> file_ptr(file, FileDeleter());
        long use_count = file_ptr.use_count();
        EXPECT_EQ(use_count, 1);
    }
    auto events = EventLog::instance().events();
    bool deleter_called = false;
    for (const auto& event : events)
    {
        if (event.find("FileDeleter::operator()") != std::string::npos)
        {
            deleter_called = true;
        }
    }
    // Q: When exactly is the custom deleter invoked relative to the control block's destruction?
    // A:
    // R:
    // R:
    EXPECT_TRUE(deleter_called);
}
class Resource
{
public:
    explicit Resource(const std::string& name)
    : tracked_(name)
    {
    }
private:
    Tracked tracked_;
};
struct ResourceDeleter
{
    void operator()(Resource* res) const
    {
        EventLog::instance().record("ResourceDeleter::operator() called");
        delete res;
    }
};
TEST_F(AllocationPatternsTest, CustomDeleterWithTrackedObject)
{
    {
        // Q: What is the sequence of operations when the shared_ptr is destroyed? (deleter call vs object destructor)
        // A:
        // R:
        std::shared_ptr<Resource> res_ptr(new Resource("Res1"), ResourceDeleter());
        long use_count = res_ptr.use_count();
        EXPECT_EQ(use_count, 1);
    }
    auto events = EventLog::instance().events();
    bool deleter_called = false;
    bool dtor_called = false;
    for (const auto& event : events)
    {
        if (event.find("ResourceDeleter::operator()") != std::string::npos)
        {
            deleter_called = true;
        }
        if (event.find("::dtor") != std::string::npos)
        {
            dtor_called = true;
        }
    }
    // Q: The deleter calls `delete res`. What happens inside `delete` that triggers the Tracked destructor?
    // A:
    // R:
    EXPECT_TRUE(deleter_called);
    EXPECT_TRUE(dtor_called);
}
TEST_F(AllocationPatternsTest, ArrayAllocationWithCustomDeleter)
{
    {
        // Q: Why must you use a custom deleter for arrays? What happens if you use the default deleter with `new T[]`?
        // A:
        // R:
        // R:
        // Q: What is the difference between `delete ptr` and `delete[] ptr`?
        // A:
        // A:
        // A:
        // R:
        // R:
        // R:
        // R:
        // R:
        // R:
        // R:
        // R:
        // R:
        std::shared_ptr<Tracked> arr_ptr(
            new Tracked[3]{Tracked("Arr1"), Tracked("Arr2"), Tracked("Arr3")},
            LoggingArrayDeleter<Tracked>("ArrayDeleter")
        );
        long use_count = arr_ptr.use_count();
        EXPECT_EQ(use_count, 1);
    }
    auto events = EventLog::instance().events();
    bool array_deleter_called = false;
    for (const auto& event : events)
    {
        if (event.find("ArrayDeleter::operator()") != std::string::npos)
        {
            array_deleter_called = true;
        }
    }
    EXPECT_TRUE(array_deleter_called);
}
TEST_F(AllocationPatternsTest, DeleterTypeErasure)
{
    struct Deleter1
    {
        void operator()(Tracked* ptr) const
        {
            EventLog::instance().record("Deleter1::operator()");
            delete ptr;
        }
    };
    struct Deleter2
    {
        void operator()(Tracked* ptr) const
        {
            EventLog::instance().record("Deleter2::operator()");
            delete ptr;
        }
    };
    // Q: Both Deleter1 and Deleter2 have different types. How can p1 and p2 have the same type `shared_ptr<Tracked>`?
    // A:
    // R:
    // R:
    // R:
    std::shared_ptr<Tracked> p1(new Tracked("T1"), Deleter1());
    std::shared_ptr<Tracked> p2(new Tracked("T2"), Deleter2());
    // Q: When you assign p2 to p1, what happens to p1's original object and which deleter is invoked?
    // A:
    // A:
    // R:
    // Q: After assignment, p1 now points to p2's object. Which deleter does p1's control block store—Deleter1 or Deleter2?
    // A:
    // R:
    p1 = p2;
    auto events = EventLog::instance().events();
    bool deleter1_called = false;
    for (const auto& event : events)
    {
        if (event.find("Deleter1::operator()") != std::string::npos)
        {
            deleter1_called = true;
        }
    }
    EXPECT_TRUE(deleter1_called);
}
TEST_F(AllocationPatternsTest, AliasingWithCustomDeleter)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };
    struct ContainerDeleter
    {
        void operator()(Container* ptr) const
        {
            EventLog::instance().record("ContainerDeleter::operator()");
            delete ptr;
        }
    };
    long alias_count = 0;
    {
        std::shared_ptr<Container> owner(new Container("Owner"), ContainerDeleter());
        // Q: The alias points to `&owner->member` (a Tracked*), but shares the control block with owner. Which deleter is stored in the control block?
        // A:
        // R:
        std::shared_ptr<Tracked> alias(owner, &owner->member);
        alias_count = alias.use_count();
        // Q: After owner.reset(), only alias remains. When alias is destroyed, what gets deleted—the Tracked member or the Container?
        // A:
        // R:
        // R:
        owner.reset();
    }
    auto events = EventLog::instance().events();
    bool container_deleter_called = false;
    for (const auto& event : events)
    {
        if (event.find("ContainerDeleter::operator()") != std::string::npos)
        {
            container_deleter_called = true;
        }
    }
    EXPECT_EQ(alias_count, 2);
    EXPECT_TRUE(container_deleter_called);
}
TEST_F(AllocationPatternsTest, SharedDeleterBetweenPointers)
{
    struct SharedDeleter
    {
        void operator()(Tracked* ptr) const
        {
            EventLog::instance().record("SharedDeleter::operator()");
            delete ptr;
        }
    };
    std::shared_ptr<Tracked> p1(new Tracked("Shared"), SharedDeleter());
    // Q: When you copy p1 to p2, do they share the same control block? What about the deleter stored in it?
    // A:
    // R:
    std::shared_ptr<Tracked> p2 = p1;
    long use_count = p1.use_count();
    // Q: After p1.reset(), the use_count drops to 1. Why isn't the deleter called yet?
    // A:
    // R:
    p1.reset();
    auto events_after_reset = EventLog::instance().events();
    bool deleter_called_early = false;
    for (const auto& event : events_after_reset)
    {
        if (event.find("SharedDeleter::operator()") != std::string::npos)
        {
            deleter_called_early = true;
        }
    }
    EXPECT_EQ(use_count, 2);
    EXPECT_FALSE(deleter_called_early);
}
