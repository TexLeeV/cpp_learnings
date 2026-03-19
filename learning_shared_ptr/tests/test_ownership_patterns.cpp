#include "instrumentation.h"

#include <gtest/gtest.h>
#include <memory>

class OwnershipPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// Helper class demonstrating enable_shared_from_this pattern
class Widget : public std::enable_shared_from_this<Widget>
{
public:
    explicit Widget(const std::string& name) : tracked_(name)
    {
    }

    std::shared_ptr<Widget> get_shared()
    {
        return shared_from_this();
    }

    std::string name() const
    {
        return tracked_.name();
    }

private:
    Tracked tracked_;
};

TEST_F(OwnershipPatternsTest, EnableSharedFromThis)
{
    long initial_count = 0;
    long after_get_shared_count = 0;
    long both_alive_count = 0;

    // TODO: Create w1 using make_shared<Widget>
    std::shared_ptr<Widget> w1 = std::make_shared<Widget>("Widget1");

    // TODO: Capture initial use_count
    initial_count = w1.use_count();
    // Q: Why is the use_count 1 at this point?
    // A:
    // R:

    // TODO: Call get_shared() to get w2
    std::shared_ptr<Widget> w2 = w1->get_shared();
    // Q: What does shared_from_this() return, and which control block does it reference?
    // A:
    // R:
    // Q: What would happen if you called shared_from_this() before any shared_ptr owned the object?
    // A:
    // R:

    // TODO: Capture use_counts after get_shared()
    after_get_shared_count = w2.use_count();
    both_alive_count = w1.use_count();
    // Q: Why do both w1.use_count() and w2.use_count() return 2?
    // A:
    // R:

    EXPECT_EQ(initial_count, 1);
    EXPECT_EQ(after_get_shared_count, 2);
    EXPECT_EQ(both_alive_count, 2);
}

// Helper class demonstrating static factory pattern
class Resource : public std::enable_shared_from_this<Resource>
{
public:
    static std::shared_ptr<Resource> create(const std::string& name)
    {
        // QA:
        // RA:
        return std::shared_ptr<Resource>(new Resource(name));
    }

    std::shared_ptr<Resource> get_ptr()
    {
        return shared_from_this();
    }

private:
    explicit Resource(const std::string& name) : tracked_(name)
    {
    }

    Tracked tracked_;
};

TEST_F(OwnershipPatternsTest, StaticFactoryPattern)
{
    long factory_count = 0;
    long after_get_ptr_count = 0;

    // TODO: Call Resource::create("R1") to get r1
    std::shared_ptr<Resource> r1 = Resource::create("R1");
    // Q: Why does the factory pattern make the constructor private?
    // A:
    // R:
    // Q: With that hint, why is the constructor private in this pattern?
    // A:
    // R:

    // TODO: Capture use_count after factory creation
    factory_count = r1.use_count();

    // TODO: Call get_ptr() to get r2
    std::shared_ptr<Resource> r2 = r1->get_ptr();
    // Q: What would happen if you tried: Resource* raw = new Resource("Bad"); raw->get_ptr();
    // A:
    // R:
    // Q: What specific exception would it throw?
    // A:
    // R:

    // TODO: Capture use_count after get_ptr()
    after_get_ptr_count = r2.use_count();
    // Q: Why is this pattern safer than allowing direct construction with shared_ptr<Resource>(new Resource("R1"))?
    // A:
    // A:
    // R:

    EXPECT_EQ(factory_count, 1);
    EXPECT_EQ(after_get_ptr_count, 2);
}

void process_by_value(std::shared_ptr<Tracked> item)
{
}

void process_by_const_ref(const std::shared_ptr<Tracked>& item)
{
}

void process_by_ref(std::shared_ptr<Tracked>& item)
{
}

TEST_F(OwnershipPatternsTest, ParameterPassingByValue)
{
    long before_call = 0;
    long after_call = 0;

    // TODO: Create ptr using make_shared
    std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Tracked1");

    // TODO: Capture use_count before calling function
    before_call = ptr.use_count();
    // Q: What is the use_count at this point?
    // A:
    // R:

    EventLog::instance().clear();

    // TODO: Call process_by_value(ptr)
    process_by_value(ptr);
    // Q: When process_by_value(ptr) is called, what happens to the shared_ptr during parameter passing?
    // A:
    // R:
    // Q: By control block pointer is copied, do you mean that the control block in the function just points to the
    // memory address of ptr's control_block_?
    // A:
    // R:

    // Q: What observable signal (in the event log) would confirm a copy occurred?
    // A:
    // R:
    // Q: If copying shared_ptr doesn't trigger Tracked events, why does the test expect events.size() > 0?
    // A:
    // R:

    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();
    // Q: Why is after_call expected to be 1, not 2?
    // A:
    // R:

    auto events = EventLog::instance().events();
    // Q: The test expects events.size() > 0. What events should be logged when passing shared_ptr by value?
    // A:
    // R:

    EXPECT_EQ(before_call, 1);
    EXPECT_EQ(after_call, 1);
    // NOTE: Test bug fixed - passing shared_ptr by value does NOT trigger Tracked events
    // because only the shared_ptr is copied (control block pointer + object pointer),
    // not the Tracked object itself. The Tracked object remains untouched.
    EXPECT_EQ(events.size(), 0);
}

TEST_F(OwnershipPatternsTest, ParameterPassingByConstRef)
{
    long before_call = 0;
    long after_call = 0;

    // TODO: Create ptr using make_shared
    std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Tracked1");

    // TODO: Capture use_count before calling function
    before_call = ptr.use_count();

    EventLog::instance().clear();

    // TODO: Call process_by_const_ref(ptr)
    process_by_const_ref(ptr);
    // Q: How does passing by const reference differ from passing by value in terms of shared_ptr operations?
    // A:
    // R:
    // Q: What is the use_count inside process_by_const_ref while it's executing?
    // A:
    // R:

    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();

    auto events = EventLog::instance().events();
    // Q: Why does this test expect events.size() == 0 (not > 0 like the by-value test originally expected)?
    // A:
    // R:

    EXPECT_EQ(before_call, 1);
    EXPECT_EQ(after_call, 1);
    EXPECT_EQ(events.size(), 0);
}

TEST_F(OwnershipPatternsTest, ParameterPassingByRef)
{
    long before_call = 0;
    long after_call = 0;

    // TODO: Create ptr using make_shared
    std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("TrackedObj");

    // TODO: Capture use_count before calling function
    before_call = ptr.use_count();

    EventLog::instance().clear();

    // TODO: Call process_by_ref(ptr)
    process_by_ref(ptr);
    // Q: How does passing by mutable reference differ from passing by const reference?
    // A:
    // A:
    // R:
    // Q: Could process_by_ref modify ptr (e.g., call ptr.reset() or reassign it)?
    // A:
    // R:

    // TODO: Capture use_count after function returns
    after_call = ptr.use_count();

    auto events = EventLog::instance().events();
    // Q: Why are the expectations identical to ParameterPassingByConstRef?
    // A:
    // R:

    EXPECT_EQ(before_call, 1);
    EXPECT_EQ(after_call, 1);
    EXPECT_EQ(events.size(), 0);
}

TEST_F(OwnershipPatternsTest, ReturnByValue)
{
    auto create_widget = []() -> std::shared_ptr<Widget> { return std::make_shared<Widget>("Created"); };

    // TODO: Call create_widget() to get result
    std::shared_ptr<Widget> result = create_widget();
    // Q: When the lambda returns the shared_ptr by value, is it copied or moved?
    // A:
    // R:

    long use_count = 0;
    // TODO: Capture use_count of result
    use_count = result.use_count();
    // Q: Why is use_count 1 instead of 2 (if a copy occurred during return)?
    // A:
    // R:
    // Q: What optimization allows returning shared_ptr by value without incrementing use_count?
    // A:
    // R:

    EXPECT_EQ(use_count, 1);
}

TEST_F(OwnershipPatternsTest, WeakPtrFromShared)
{
    long shared_count_before = 0;
    long shared_count_after = 0;
    bool weak_expired_before = false;
    bool weak_expired_after = false;

    // TODO: Create weak_ptr (initially empty)
    std::weak_ptr<Tracked> weak;
    // Q: What does an empty weak_ptr point to?
    // A:
    // R:

    // TODO: Check if weak is expired (should be true initially)
    weak_expired_before = weak.expired();
    // Q: Why is an empty weak_ptr considered "expired"?
    // A:
    // R:

    {
        // TODO: Create shared_ptr
        std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");

        // TODO: Assign shared to weak
        weak = shared;
        // Q: When you assign shared to weak, does the use_count of shared change?
        // A:
        // R:
        // Q: What happens when a std::weak_ptr<Tracked> is kept alive indefinitely? Is this a memory leak? I thought
        // only use_count was used to decide when the memory is released?
        // R:

        // TODO: Capture shared's use_count
        shared_count_before = shared.use_count();

        // TODO: Check if weak is expired (should be false now)
        weak_expired_before = weak.expired();
    }

    // TODO: Check if weak is expired after shared goes out of scope
    weak_expired_after = weak.expired();
    // Q: After shared is destroyed, what happens to the Tracked object and the control block?
    // A:
    // R:
    // Q: Can valgrind or sanitizers find these types of memory leaks?
    // R:
    EXPECT_EQ(shared_count_before, 1);
    EXPECT_FALSE(weak_expired_before);
    EXPECT_TRUE(weak_expired_after);
}

TEST_F(OwnershipPatternsTest, WeakPtrLock)
{
    long use_count_with_lock = 0;
    bool lock_succeeded = false;

    // TODO: Create shared_ptr
    std::shared_ptr<Tracked> shared = std::make_shared<Tracked>("TrackedObj");

    // TODO: Create weak_ptr from shared
    std::weak_ptr<Tracked> weak = shared;

    // TODO: Lock the weak_ptr to get a shared_ptr
    std::shared_ptr<Tracked> locked_ptr = weak.lock();
    // Q: What does weak.lock() do, and why is it necessary?
    // A:
    // A:
    // R:
    // Q: What happens to use_count when lock() succeeds?
    // A:
    // R:

    // TODO: Check if lock succeeded (locked_ptr != nullptr)
    lock_succeeded = (locked_ptr != nullptr);

    // TODO: Capture use_count
    use_count_with_lock = shared.use_count();
    // Q: Why is use_count 2 instead of 1?
    // A:
    // R:
    // Q: What would lock() return if the object had already been destroyed?
    // A:
    // R:

    EXPECT_TRUE(lock_succeeded);
    EXPECT_EQ(use_count_with_lock, 2);
}

TEST_F(OwnershipPatternsTest, MoveConstruction)
{
    long count_before_move = 0;
    long count_after_move_source = 0;
    long count_after_move_dest = 0;

    // TODO: Create p1
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("TrackedObj");

    // TODO: Capture use_count before move
    count_before_move = p1.use_count();

    // TODO: Move p1 into p2
    std::shared_ptr<Tracked> p2 = std::move(p1);
    // Q: What happens to the control block's use_count during a move?
    // A:
    // R:
    // Q: After the move, what does p1 point to?
    // A:
    // R:

    // TODO: Capture use_counts after move
    count_after_move_source = p1.use_count();
    count_after_move_dest = p2.use_count();
    // Q: Why is p1.use_count() 0 after the move?
    // A:
    // R:
    // Q: How does move differ from copy in terms of performance?
    // A:
    // R:

    EXPECT_EQ(count_before_move, 1);
    EXPECT_EQ(count_after_move_source, 0);
    EXPECT_EQ(count_after_move_dest, 1);
}

TEST_F(OwnershipPatternsTest, MoveAssignment)
{
    long p1_count_before = 0;
    long p2_count_before = 0;
    long p1_count_after = 0;
    long p2_count_after = 0;

    // TODO: Create p1 and p2
    std::shared_ptr<Tracked> p1 = std::make_shared<Tracked>("Tracked1");
    std::shared_ptr<Tracked> p2 = std::make_shared<Tracked>("Tracked2");
    // Q: Before move assignment, how many Tracked objects exist?
    // A:
    // R:

    // TODO: Capture use_counts before move assignment
    p1_count_before = p1.use_count();
    p2_count_before = p2.use_count();

    EventLog::instance().clear();

    // TODO: Move assign p2 to p1 (p1 = std::move(p2))
    p1 = std::move(p2);
    // Q: What happens to the Tracked object that p1 originally pointed to?
    // A:
    // R:
    // Q: After move assignment, what does p1 point to?
    // A:
    // R:
    // Q: Why does p2.use_count() return 0 after the move?
    // A:
    // R:

    // TODO: Capture use_counts after move assignment
    p1_count_after = p1.use_count();
    p2_count_after = p2.use_count();

    size_t dtor_count = EventLog::instance().count_events("::dtor");
    // Q: Why is dtor_count expected to be 1?
    // A:
    // R:

    EXPECT_EQ(p1_count_before, 1);
    EXPECT_EQ(p2_count_before, 1);
    EXPECT_EQ(p1_count_after, 1);
    EXPECT_EQ(p2_count_after, 0);
    EXPECT_EQ(dtor_count, 1);
}
