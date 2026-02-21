#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

// Test Suite 2: Circular Reference + Mutex Deadlocks
// Focus: Combining shared_ptr cycles with mutex contention

class CircularReferenceDeadlocksTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Parent-Child Circular Reference with Mutex
// ============================================================================

class Child;

class Parent
{
public:
    explicit Parent(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    void set_child(std::shared_ptr<Child> child)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        child_ = child;
    }
    
    std::shared_ptr<Child> get_child()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return child_;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::shared_ptr<Child> child_;
    std::mutex mutex_;
};

class Child
{
public:
    explicit Child(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    void set_parent(std::shared_ptr<Parent> parent)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        parent_ = parent;
    }
    
    std::shared_ptr<Parent> get_parent()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return parent_;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::shared_ptr<Parent> parent_;
    std::mutex mutex_;
};

// DEADLOCK VERSION: Circular shared_ptr + mutex contention
void setup_parent_child_deadlock(std::shared_ptr<Parent> parent, std::shared_ptr<Child> child)
{
    std::lock_guard<std::mutex> lock1(parent->get_mutex());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    parent->set_child(child);
}

void setup_child_parent_deadlock(std::shared_ptr<Child> child, std::shared_ptr<Parent> parent)
{
    std::lock_guard<std::mutex> lock1(child->get_mutex());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    child->set_parent(parent);
}

TEST_F(CircularReferenceDeadlocksTest, Scenario1_ParentChildCircularWithMutex_Broken)
{
    auto parent = std::make_shared<Parent>("Parent");
    auto child = std::make_shared<Child>("Child");
    
    std::atomic<bool> deadlock_detected(false);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_detected = true;
            return;
        }
        setup_parent_child_deadlock(parent, child);
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_detected = true;
            return;
        }
        setup_child_parent_deadlock(child, parent);
    });
    
    t1.join();
    t2.join();
    
    // Q: What are the TWO problems in this scenario (ownership + synchronization)?
    // A: 
    // R: 
    
    // Q: Does the circular shared_ptr reference cause the deadlock, or the mutex ordering?
    // A: 
    // R: 
}

// FIX VERSION: Break cycle with weak_ptr AND fix lock ordering
void setup_parent_child_fixed(std::shared_ptr<Parent> parent, std::shared_ptr<Child> child)
{
    // TODO: Implement deadlock-free parent-child setup
    // Hint: Use weak_ptr for back-reference AND proper lock ordering
    
    // YOUR CODE HERE
}

void setup_child_parent_fixed(std::shared_ptr<Child> child, std::shared_ptr<Parent> parent)
{
    // TODO: Implement deadlock-free child-parent setup
    
    // YOUR CODE HERE
}

TEST_F(CircularReferenceDeadlocksTest, Scenario1_ParentChildCircularWithMutex_Fixed)
{
    auto parent = std::make_shared<Parent>("Parent");
    auto child = std::make_shared<Child>("Child");
    
    std::thread t1([&]() { setup_parent_child_fixed(parent, child); });
    std::thread t2([&]() { setup_child_parent_fixed(child, parent); });
    
    t1.join();
    t2.join();
    
    EXPECT_TRUE(parent->get_child() != nullptr);
    // Note: If using weak_ptr in Child, check differently
}

// ============================================================================
// Scenario 2: Doubly-Linked List with Mutex Deadlock
// ============================================================================

class Node
{
public:
    explicit Node(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    void set_next(std::shared_ptr<Node> next)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        next_ = next;
    }
    
    void set_prev(std::shared_ptr<Node> prev)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        prev_ = prev;
    }
    
    std::shared_ptr<Node> get_next()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return next_;
    }
    
    std::shared_ptr<Node> get_prev()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return prev_;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::shared_ptr<Node> next_;
    std::shared_ptr<Node> prev_;
    std::mutex mutex_;
};

// DEADLOCK VERSION: Bidirectional linking with wrong lock order
void link_nodes_deadlock(std::shared_ptr<Node> n1, std::shared_ptr<Node> n2)
{
    // Lock n1, then n2
    std::lock_guard<std::mutex> lock1(n1->get_mutex());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock2(n2->get_mutex());
    
    n1->set_next(n2);
    n2->set_prev(n1);
}

TEST_F(CircularReferenceDeadlocksTest, Scenario2_DoublyLinkedListDeadlock_Broken)
{
    auto n1 = std::make_shared<Node>("N1");
    auto n2 = std::make_shared<Node>("N2");
    auto n3 = std::make_shared<Node>("N3");
    
    std::atomic<int> deadlock_count(0);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        link_nodes_deadlock(n1, n2);
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        link_nodes_deadlock(n2, n3);
    });
    
    std::thread t3([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        link_nodes_deadlock(n3, n1);
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: Why is a doubly-linked list particularly prone to deadlocks?
    // A: 
    // R: 
    
    // Q: What happens to the shared_ptr reference count when we create a cycle?
    // A: 
    // R: 
    
    EXPECT_GT(deadlock_count.load(), 0);
}

// FIX VERSION: Use weak_ptr for back-pointers AND proper locking
void link_nodes_fixed(std::shared_ptr<Node> n1, std::shared_ptr<Node> n2)
{
    // TODO: Implement deadlock-free bidirectional linking
    // Hint: Use weak_ptr for prev pointers AND std::lock() for multiple mutexes
    
    // YOUR CODE HERE
}

TEST_F(CircularReferenceDeadlocksTest, Scenario2_DoublyLinkedListDeadlock_Fixed)
{
    auto n1 = std::make_shared<Node>("N1");
    auto n2 = std::make_shared<Node>("N2");
    auto n3 = std::make_shared<Node>("N3");
    
    std::thread t1([&]() { link_nodes_fixed(n1, n2); });
    std::thread t2([&]() { link_nodes_fixed(n2, n3); });
    std::thread t3([&]() { link_nodes_fixed(n3, n1); });
    
    t1.join();
    t2.join();
    t3.join();
    
    EXPECT_TRUE(n1->get_next() != nullptr);
}

// ============================================================================
// Scenario 3: Observer Pattern with Circular References
// ============================================================================

class Observer;

class Subject
{
public:
    explicit Subject(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    void attach(std::shared_ptr<Observer> obs)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.push_back(obs);
    }
    
    void notify();
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::vector<std::shared_ptr<Observer>> observers_;
    std::mutex mutex_;
};

class Observer
{
public:
    explicit Observer(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    void update(std::shared_ptr<Subject> subj)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        subject_ = subj;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::shared_ptr<Subject> subject_;
    std::mutex mutex_;
};

// DEADLOCK VERSION: Notify holds subject lock, calls observer which needs observer lock
void Subject::notify()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    for (auto& obs : observers_)
    {
        obs->update(std::make_shared<Subject>("temp"));
    }
}

TEST_F(CircularReferenceDeadlocksTest, Scenario3_ObserverPatternCircular_Broken)
{
    auto subject = std::make_shared<Subject>("Subject");
    auto obs1 = std::make_shared<Observer>("Obs1");
    auto obs2 = std::make_shared<Observer>("Obs2");
    
    subject->attach(obs1);
    subject->attach(obs2);
    
    std::atomic<int> deadlock_count(0);
    
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        subject->notify();
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        std::lock_guard<std::mutex> lock(obs1->get_mutex());
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });
    
    std::thread t3([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        std::lock_guard<std::mutex> lock(obs2->get_mutex());
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: Why is calling observer methods while holding subject lock dangerous?
    // A: 
    // R: 
    
    // Q: Should observers hold shared_ptr to subject? What's the alternative?
    // A: 
    // R: 
}

// FIX VERSION: Use weak_ptr for observer back-reference AND release lock before callback
// TODO: Modify Observer class to use weak_ptr and implement safe notify
void notify_fixed(Subject& subject)
{
    // TODO: Implement deadlock-free notification
    // Hint: Copy observers, release lock, then notify
    
    // YOUR CODE HERE
}

TEST_F(CircularReferenceDeadlocksTest, Scenario3_ObserverPatternCircular_Fixed)
{
    auto subject = std::make_shared<Subject>("Subject");
    auto obs1 = std::make_shared<Observer>("Obs1");
    auto obs2 = std::make_shared<Observer>("Obs2");
    
    subject->attach(obs1);
    subject->attach(obs2);
    
    std::thread t1([&]() { notify_fixed(*subject); });
    std::thread t2([&]()
    {
        std::lock_guard<std::mutex> lock(obs1->get_mutex());
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });
    std::thread t3([&]()
    {
        std::lock_guard<std::mutex> lock(obs2->get_mutex());
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    });
    
    t1.join();
    t2.join();
    t3.join();
}

// ============================================================================
// Scenario 4: Graph with Circular Dependencies and Locks
// ============================================================================

class GraphNode
{
public:
    explicit GraphNode(const std::string& name)
    : data_(std::make_shared<Tracked>(name))
    {
    }
    
    void add_edge(std::shared_ptr<GraphNode> neighbor)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        neighbors_.push_back(neighbor);
    }
    
    std::vector<std::shared_ptr<GraphNode>> get_neighbors()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return neighbors_;
    }
    
    std::mutex& get_mutex()
    {
        return mutex_;
    }
    
private:
    std::shared_ptr<Tracked> data_;
    std::vector<std::shared_ptr<GraphNode>> neighbors_;
    std::mutex mutex_;
};

// DEADLOCK VERSION: Creating bidirectional edges with wrong lock order
void create_bidirectional_edge_deadlock(std::shared_ptr<GraphNode> n1, std::shared_ptr<GraphNode> n2)
{
    std::lock_guard<std::mutex> lock1(n1->get_mutex());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock2(n2->get_mutex());
    
    n1->add_edge(n2);
    n2->add_edge(n1);
}

TEST_F(CircularReferenceDeadlocksTest, Scenario4_GraphCircularDependencies_Broken)
{
    auto n1 = std::make_shared<GraphNode>("N1");
    auto n2 = std::make_shared<GraphNode>("N2");
    auto n3 = std::make_shared<GraphNode>("N3");
    
    std::atomic<int> deadlock_count(0);
    
    // Create triangle: N1<->N2, N2<->N3, N3<->N1
    std::thread t1([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        create_bidirectional_edge_deadlock(n1, n2);
    });
    
    std::thread t2([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        create_bidirectional_edge_deadlock(n2, n3);
    });
    
    std::thread t3([&]()
    {
        std::timed_mutex timeout;
        if (!timeout.try_lock_for(std::chrono::milliseconds(100)))
        {
            deadlock_count++;
            return;
        }
        create_bidirectional_edge_deadlock(n3, n1);
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    // Q: In a graph structure, why can't we simply "always lock in order"?
    // A: 
    // R: 
    
    // Q: What's the memory leak risk with circular shared_ptr references in a graph?
    // A: 
    // R: 
    
    EXPECT_GT(deadlock_count.load(), 0);
}

// FIX VERSION: Use weak_ptr for one direction AND proper locking
void create_bidirectional_edge_fixed(std::shared_ptr<GraphNode> n1, std::shared_ptr<GraphNode> n2)
{
    // TODO: Implement deadlock-free bidirectional edge creation
    // Hint: Consider using weak_ptr for back-edges OR std::lock() for multiple mutexes
    
    // YOUR CODE HERE
}

TEST_F(CircularReferenceDeadlocksTest, Scenario4_GraphCircularDependencies_Fixed)
{
    auto n1 = std::make_shared<GraphNode>("N1");
    auto n2 = std::make_shared<GraphNode>("N2");
    auto n3 = std::make_shared<GraphNode>("N3");
    
    std::thread t1([&]() { create_bidirectional_edge_fixed(n1, n2); });
    std::thread t2([&]() { create_bidirectional_edge_fixed(n2, n3); });
    std::thread t3([&]() { create_bidirectional_edge_fixed(n3, n1); });
    
    t1.join();
    t2.join();
    t3.join();
    
    EXPECT_TRUE(n1->get_neighbors().size() > 0);
}
