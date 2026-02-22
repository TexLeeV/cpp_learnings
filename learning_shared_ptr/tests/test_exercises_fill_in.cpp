#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

class FillInExercisesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// Demonstration: What does the instrumentation actually log?
TEST_F(FillInExercisesTest, Demo_InstrumentationOutput)
{
    std::cout << "\n=== Instrumentation Demo ===\n";

    {
        std::cout << "Creating shared_ptr...\n";
        std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Demo");
        std::cout << "use_count: " << ptr.use_count() << "\n";

        std::cout << "Copying shared_ptr...\n";
        std::shared_ptr<Tracked> ptr2 = ptr;
        std::cout << "use_count: " << ptr.use_count() << "\n";

        std::cout << "Resetting ptr2...\n";
        ptr2.reset();
        std::cout << "use_count: " << ptr.use_count() << "\n";

        std::cout << "About to exit scope (ptr will be destroyed)...\n";
    }

    std::cout << "\n=== EventLog Contents ===\n";
    std::cout << EventLog::instance().dump();
    std::cout << "=========================\n\n";
}

// Exercise 1: Implement a class that uses enable_shared_from_this
// TODO: Complete the Widget class so it can safely return a shared_ptr to itself
class Widget : public std::enable_shared_from_this<Widget>
{
public:
    explicit Widget(const std::string& name)
    : tracked_(name)
    {
    }
    
    // TODO: Implement this method to return a shared_ptr to this object
    std::shared_ptr<Widget> get_self()
    {
        // YOUR CODE HERE
        return shared_from_this();
    }
    
private:
    Tracked tracked_;
};

TEST_F(FillInExercisesTest, Exercise1_EnableSharedFromThis)
{
    std::shared_ptr<Widget> w1 = std::make_shared<Widget>("W1");
    std::shared_ptr<Widget> w2 = w1;
    w1.reset();
    if (w2)
    {
        std::cout << "is NOT nullptr" << std::endl;
    }
    else
    {
        std::cout << "is nullptr" << std::endl;
    }
    // std::shared_ptr<Widget> w1 = std::make_shared<Widget>("W1");
    // std::shared_ptr<Widget> w2 = w1->get_self();
    
    // EXPECT_EQ(w1.use_count(), 2);
    // EXPECT_EQ(w2.use_count(), 2);
    // EXPECT_EQ(w1.get(), w2.get());
}

// Exercise 2: Implement a weak_ptr cache
// TODO: Complete the Cache class to store weak_ptr and auto-cleanup expired entries
class Cache
{
public:
    std::shared_ptr<Tracked> get(const std::string& key)
    {
        // TODO: Implement cache logic:
        // 1. Check if key exists in cache
        auto it = cache_.find(key);
        
        // 2. If exists, try to lock the weak_ptr
        if (it != cache_.end())
        {
            std::shared_ptr<Tracked> locked = it->second.lock();
            // 3. If lock succeeds, return the shared_ptr
            if (locked)
            {
                return locked;
            }
        }
        
        // 4. If lock fails or key doesn't exist, create new resource
        std::shared_ptr<Tracked> new_resource = std::make_shared<Tracked>(key);
        
        // 5. Store weak_ptr in cache and return shared_ptr
        cache_[key] = new_resource;
        return new_resource;
    }
    
    void cleanup()
    {
        // TODO: Remove all expired weak_ptr entries from cache
        for (auto it = cache_.begin(); it != cache_.end(); )
        {
            if (it->second.expired())
            {
                it = cache_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    
    size_t size() const
    {
        // YOUR CODE HERE
        return cache_.size();
    }
    
private:
    // TODO: Add appropriate data structure to store weak_ptr
    std::map<std::string, std::weak_ptr<Tracked>> cache_;
};

TEST_F(FillInExercisesTest, Exercise2_WeakPtrCache)
{
    Cache cache;

    // Q: How many shared_ptr instances exist after this call? Where are they?
    // A: 1
    // R: Correct. One shared_ptr exists: r1. The cache holds a weak_ptr which doesn't count toward use_count().
    std::shared_ptr<Tracked> r1 = cache.get("key1");
    EXPECT_EQ(cache.size(), 1);
    // Q: If the cache stored shared_ptr instead of weak_ptr, what would use_count() be here?
    // A: 2
    // R: Correct. One in r1, one in the cache. This would prevent the caller from controlling lifetime.
    EXPECT_EQ(r1.use_count(), 1);

    // Q: What does weak_ptr::lock() return? What happens to the reference count when assigned to r2?
    // A: Returns a shared_ptr and increments the count.  It ensure's that something else cares about the result still
    // R: Correct. lock() returns shared_ptr<Tracked>. Assigning to r2 increments the count from 1 to 2. The lock() operation atomically checks if the object still exists and creates a new shared_ptr if it does.
    std::shared_ptr<Tracked> r2 = cache.get("key1");
    EXPECT_EQ(cache.size(), 1);
    // Q: Walk through the cache lookup—what operation caused the count to increment from 1 to 2?
    // A: cache_ != end(), it->second().lock() returns a shared_ptr that is then returned and then assigned to r2, thus incrementing the count to 2
    // R: Correct trace. The key sequence: find() succeeds → lock() creates new shared_ptr (count becomes 2) → return by value (no additional increment due to move/copy elision) → assign to r2.
    EXPECT_EQ(r1.use_count(), 2);
    EXPECT_EQ(r1.get(), r2.get());

    // Q: After both reset() calls, what is the reference count? What happens to the object?
    // A: r1.reset() -> 1, r2.reset() -> 0 -> releases the resource
    // R: Correct. Count drops to 0, triggering the destructor. The Tracked object is destroyed, and its destructor logs "Tracked(key1)::dtor [id=N]" to EventLog.
    r1.reset();
    r2.reset();

    // Q: When cleanup() calls expired() on the weak_ptr, what will it return and why?
    // A: expired will return true and then erased at that iterater location
    // R: Correct. expired() returns true because the reference count reached 0 (the control block still exists but marks the object as destroyed). The weak_ptr is then erased from the map.
    // Q: What observable event in the instrumentation log confirms the object was destroyed?
    // A: I have not viewed the instumentation code.  Provide the actual result below
    // R: The instrumentation logs "[N] Tracked(key1)::dtor [id=M]" when the object is destroyed (after both reset() calls).
    // R: The EventLog is stored internally but not printed by default. See Demo_InstrumentationOutput test for an example.
    // R: To see it here, uncomment: std::cout << "\n" << EventLog::instance().dump();
    // R: Expected output: "[0] Tracked(key1)::ctor [id=1]" followed by "[1] Tracked(key1)::dtor [id=1]"
    cache.cleanup();
    EXPECT_EQ(cache.size(), 0);
}

// Exercise 3: Implement Observer pattern with automatic cleanup
// TODO: Complete the Subject class to manage observers using weak_ptr
class Observer
{
public:
    explicit Observer(const std::string& name)
    : tracked_(name)
    {
    }
    
    void notify()
    {
        EventLog::instance().record("Observer notified");
    }
    
private:
    Tracked tracked_;
};

class Subject
{
public:
    void attach(std::shared_ptr<Observer> observer)
    {
        // TODO: Store observer as weak_ptr
        observers_.push_back(observer);
    }
    
    void notify_all()
    {
        // TODO: Iterate through observers
        // 1. Try to lock each weak_ptr
        // 2. If lock succeeds, call notify()
        // 3. If lock fails, remove from list
        for (auto it = observers_.begin(); it != observers_.end(); )
        {
            std::shared_ptr<Observer> locked = it->lock();
            if (locked)
            {
                locked->notify();
                ++it;
            }
            else
            {
                it = observers_.erase(it);
            }
        }
    }
    
    size_t observer_count() const
    {
        // YOUR CODE HERE
        return observers_.size();
    }
    
private:
    // TODO: Add data structure to store weak_ptr<Observer>
    std::vector<std::weak_ptr<Observer>> observers_;
};

TEST_F(FillInExercisesTest, Exercise3_ObserverPattern)
{
    Subject subject;
    
    {
        std::shared_ptr<Observer> obs1 = std::make_shared<Observer>("Obs1");
        std::shared_ptr<Observer> obs2 = std::make_shared<Observer>("Obs2");
        
        subject.attach(obs1);
        subject.attach(obs2);
        
        EXPECT_EQ(subject.observer_count(), 2);
        
        EventLog::instance().clear();
        subject.notify_all();
        
        auto events = EventLog::instance().events();
        size_t notify_count = 0;
        for (const auto& event : events)
        {
            if (event.find("Observer notified") != std::string::npos)
            {
                ++notify_count;
            }
        }
        EXPECT_EQ(notify_count, 2);
    }
    
    subject.notify_all();
    EXPECT_EQ(subject.observer_count(), 0);
}

// Exercise 4: Implement custom deleter for C resource
// TODO: Create a custom deleter that logs when it's called
TEST_F(FillInExercisesTest, Exercise4_CustomDeleter)
{
    {
        FILE* file = std::tmpfile();
        
        // TODO: Create a shared_ptr with a custom deleter that:
        // 1. Logs "Custom deleter called" to EventLog
        // 2. Closes the file with fclose()
        // YOUR CODE HERE - create shared_ptr<FILE> with custom deleter
        std::shared_ptr<FILE> file_ptr(file, [](FILE* f)
        {
            EventLog::instance().record("Custom deleter called");
            if (f)
            {
                fclose(f);
            }
        });
        
        EXPECT_NE(file_ptr.get(), nullptr);
    }
    
    auto events = EventLog::instance().events();
    bool deleter_called = false;
    for (const auto& event : events)
    {
        if (event.find("Custom deleter called") != std::string::npos)
        {
            deleter_called = true;
        }
    }
    EXPECT_TRUE(deleter_called);
}

// Exercise 5: Implement aliasing constructor usage
// TODO: Use aliasing constructor to create shared_ptr to member
TEST_F(FillInExercisesTest, Exercise5_AliasingConstructor)
{
    struct Container
    {
        Tracked member;
        explicit Container(const std::string& name)
        : member(name)
        {
        }
    };
    
    std::shared_ptr<Container> owner = std::make_shared<Container>("Owner");
    
    // TODO: Create a shared_ptr<Tracked> that:
    // 1. Points to owner->member
    // 2. Shares ownership with owner (keeps Container alive)
    // 3. Use the aliasing constructor
    // YOUR CODE HERE
    std::shared_ptr<Tracked> alias(owner, &owner->member);
    
    EXPECT_EQ(owner.use_count(), 2);
    EXPECT_EQ(alias.use_count(), 2);
    
    owner.reset();
    
    EXPECT_EQ(alias.use_count(), 1);
    EXPECT_NE(alias.get(), nullptr);
}

// Exercise 6: Implement copy-on-write string
// TODO: Complete the CowString class
class CowString
{
public:
    explicit CowString(const std::string& str)
    : data_(std::make_shared<Tracked>(str))
    {
        // TODO: Initialize data_ with a shared_ptr to Tracked
        // YOUR CODE HERE
    }
    
    std::string get() const
    {
        // TODO: Return the string from data_
        // YOUR CODE HERE
        return data_->name();
    }
    
    void set(const std::string& str)
    {
        // TODO: Implement copy-on-write:
        // 1. Check if use_count() > 1
        // 2. If yes, create new Tracked with str
        // 3. If no, can modify in place (but for simplicity, always create new)
        // YOUR CODE HERE
        if (data_.use_count() > 1)
        {
            data_ = std::make_shared<Tracked>(str);
        }
        else
        {
            data_ = std::make_shared<Tracked>(str);
        }
    }
    
    long use_count() const
    {
        // YOUR CODE HERE
        return data_.use_count();
    }
    
private:
    // TODO: Add shared_ptr<Tracked> member
    std::shared_ptr<Tracked> data_;
};

TEST_F(FillInExercisesTest, Exercise6_CopyOnWrite)
{
    CowString s1("Original");
    EXPECT_EQ(s1.use_count(), 1);
    
    CowString s2 = s1;
    EXPECT_EQ(s1.use_count(), 2);
    EXPECT_EQ(s2.use_count(), 2);
    
    s2.set("Modified");
    EXPECT_EQ(s1.use_count(), 1);
    EXPECT_EQ(s2.use_count(), 1);
    EXPECT_EQ(s1.get(), "Original");
    EXPECT_EQ(s2.get(), "Modified");
}

// Exercise 7: Break a circular reference
// TODO: Fix the Node class to avoid memory leak
class Node
{
public:
    explicit Node(const std::string& name)
    : tracked_(name)
    {
    }
    
    void set_next(std::shared_ptr<Node> next)
    {
        // TODO: Store next in a way that doesn't create circular reference
        // Hint: Should this be shared_ptr or weak_ptr?
        // YOUR CODE HERE
        next_ = next;
    }
    
    std::shared_ptr<Node> next() const
    {
        // TODO: Return the next node (may need to lock if using weak_ptr)
        // YOUR CODE HERE
        return next_.lock();
    }
    
private:
    Tracked tracked_;
    // TODO: Add member to store next node
    std::weak_ptr<Node> next_;
};

TEST_F(FillInExercisesTest, Exercise7_BreakCircularReference)
{
    {
        std::shared_ptr<Node> node1 = std::make_shared<Node>("Node1");
        std::shared_ptr<Node> node2 = std::make_shared<Node>("Node2");
        
        node1->set_next(node2);
        node2->set_next(node1);
        
        EXPECT_EQ(node1.use_count(), 1);
        EXPECT_EQ(node2.use_count(), 1);
    }
    
    auto events = EventLog::instance().events();
    size_t dtor_count = 0;
    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }
    EXPECT_EQ(dtor_count, 2);
}
