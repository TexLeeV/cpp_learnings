#include "instrumentation.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

class StructuralPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// Helper class with circular reference potential (BAD - causes leak)
class Node
{
public:
    explicit Node(const std::string& name)
    : tracked_(name)
    {
    }
    
    void set_next(std::shared_ptr<Node> next)
    {
        next_ = next;
    }
    
    std::shared_ptr<Node> next() const
    {
        return next_;
    }
    
private:
    Tracked tracked_;
    std::shared_ptr<Node> next_;
};

TEST_F(StructuralPatternsTest, CircularReferenceMemoryLeak)
{
    long node1_count = 0;
    long node2_count = 0;
    long after_cycle_node1_count = 0;
    long after_cycle_node2_count = 0;
    
    {
        std::shared_ptr<Node> node1 = std::make_shared<Node>("Node1");
        std::shared_ptr<Node> node2 = std::make_shared<Node>("Node2");
        
        node1_count = node1.use_count();
        node2_count = node2.use_count();
        
        node1->set_next(node2);
        node2->set_next(node1);
        
        after_cycle_node1_count = node1.use_count();
        after_cycle_node2_count = node2.use_count();
        
        // Q: Why does the reference count increase from 1 to 2 for both nodes after creating the cycle?
        // A:
        // A:
        // R:
        
        // Q: When node1 and node2 go out of scope at the closing brace, what prevents their destructors from being called?
        // A:
        // R:
        
        // Q: Walk through what happens when node1 tries to be destroyed: what is its reference count, and what keeps it alive?
        // A:
        // R:
        
        // Q: What observable signal confirms the memory leak?
        // A:
        // R:
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
    
    EXPECT_EQ(node1_count, 1);
    EXPECT_EQ(node2_count, 1);
    EXPECT_EQ(after_cycle_node1_count, 2);
    EXPECT_EQ(after_cycle_node2_count, 2);
    EXPECT_EQ(dtor_count, 0);  // Memory leak!
}

// Helper class that breaks cycles with weak_ptr (GOOD)
class WeakNode
{
public:
    explicit WeakNode(const std::string& name)
    : tracked_(name)
    {
    }
    
    void set_next(std::shared_ptr<WeakNode> next)
    {
        next_ = next;
    }
    
    std::shared_ptr<WeakNode> next() const
    {
        return next_.lock();
    }
    
private:
    Tracked tracked_;
    std::weak_ptr<WeakNode> next_;
};

TEST_F(StructuralPatternsTest, BreakingCycleWithWeakPtr)
{
    long node1_count = 0;
    long node2_count = 0;
    long after_weak_cycle_node1_count = 0;
    long after_weak_cycle_node2_count = 0;
    
    {
        std::shared_ptr<WeakNode> node1 = std::make_shared<WeakNode>("Node1");
        std::shared_ptr<WeakNode> node2 = std::make_shared<WeakNode>("Node2");
        
        node1_count = node1.use_count();
        node2_count = node2.use_count();
        
        node1->set_next(node2);
        node2->set_next(node1);
        
        after_weak_cycle_node1_count = node1.use_count();
        after_weak_cycle_node2_count = node2.use_count();
        
        // Q: Does weak_ptr increase use_count when assigned?
        // A:
        // R:
        
        // Q: Why does the reference count remain at 1 for both nodes after setting up the cycle?
        // A:
        // A:
        // R:
        
        // Q: When node1 goes out of scope, what is its reference count and can it be destroyed?
        // A:
        // A:
        // R:
        
        // Q: How does weak_ptr break the cycle that shared_ptr creates?
        // A:
        // R:
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
    
    EXPECT_EQ(node1_count, 1);
    EXPECT_EQ(node2_count, 1);
    EXPECT_EQ(after_weak_cycle_node1_count, 1);
    EXPECT_EQ(after_weak_cycle_node2_count, 1);
    EXPECT_EQ(dtor_count, 2);  // No leak!
}

// Parent-Child with circular reference (BAD)
class Parent
{
public:
    explicit Parent(const std::string& name)
    : tracked_(name)
    {
    }
    
    void add_child(std::shared_ptr<class Child> child)
    {
        children_.push_back(child);
    }
    
    size_t child_count() const
    {
        return children_.size();
    }
    
private:
    Tracked tracked_;
    std::vector<std::shared_ptr<class Child>> children_;
};

class Child
{
public:
    explicit Child(const std::string& name)
    : tracked_(name)
    {
    }
    
    void set_parent(std::shared_ptr<Parent> parent)
    {
        parent_ = parent;
    }
    
    std::shared_ptr<Parent> parent() const
    {
        return parent_;
    }
    
private:
    Tracked tracked_;
    std::shared_ptr<Parent> parent_;
};

TEST_F(StructuralPatternsTest, ParentChildCircularReference)
{
    long parent_count = 0;
    long child_count = 0;
    long after_link_parent_count = 0;
    long after_link_child_count = 0;
    
    {
        std::shared_ptr<Parent> parent = std::make_shared<Parent>("Parent");
        std::shared_ptr<Child> child = std::make_shared<Child>("Child");
        
        parent_count = parent.use_count();
        child_count = child.use_count();
        
        parent->add_child(child);
        child->set_parent(parent);
        
        after_link_parent_count = parent.use_count();
        after_link_child_count = child.use_count();
        
        // Q: After parent->add_child(child), what is child's reference count and why?
        // A:
        // R:
        
        // Q: After child->set_parent(parent), what is parent's reference count and why?
        // A:
        // R:
        
        // Q: Why do both parent and child have a reference count of 2 after linking?
        // A:
        // R:
        
        // Q: When parent goes out of scope, why can't it be destroyed?
        // A:
        // R:
        
        // Q: In a parent-child relationship, which direction should typically use weak_ptr to avoid cycles?
        // A:
        // A:
        // R:
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
    
    EXPECT_EQ(parent_count, 1);
    EXPECT_EQ(child_count, 1);
    EXPECT_EQ(after_link_parent_count, 2);
    EXPECT_EQ(after_link_child_count, 2);
    EXPECT_EQ(dtor_count, 0);  // Memory leak!
}

// Parent-Child with weak_ptr (GOOD)
class WeakParent
{
public:
    explicit WeakParent(const std::string& name)
    : tracked_(name)
    {
    }
    
    void add_child(std::shared_ptr<class WeakChild> child)
    {
        children_.push_back(child);
    }
    
    size_t child_count() const
    {
        return children_.size();
    }
    
private:
    Tracked tracked_;
    std::vector<std::shared_ptr<class WeakChild>> children_;
};

class WeakChild
{
public:
    explicit WeakChild(const std::string& name)
    : tracked_(name)
    {
    }
    
    void set_parent(std::shared_ptr<WeakParent> parent)
    {
        parent_ = parent;
    }
    
    std::shared_ptr<WeakParent> parent() const
    {
        return parent_.lock();
    }
    
private:
    Tracked tracked_;
    std::weak_ptr<WeakParent> parent_;
};

TEST_F(StructuralPatternsTest, ParentChildWithWeakPtr)
{
    long parent_count = 0;
    long child_count = 0;
    long after_link_parent_count = 0;
    long after_link_child_count = 0;
    
    {
        std::shared_ptr<WeakParent> parent = std::make_shared<WeakParent>("Parent");
        std::shared_ptr<WeakChild> child = std::make_shared<WeakChild>("Child");
        
        parent_count = parent.use_count();
        child_count = child.use_count();
        
        parent->add_child(child);
        child->set_parent(parent);
        
        after_link_parent_count = parent.use_count();
        after_link_child_count = child.use_count();
        
        // Q: After parent->add_child(child), what is child's reference count?
        // A:
        // AA: I never considered this inline forward declaration syntax: "std::vector<std::shared_ptr<class WeakChild>> children_;".  Is this possible in c++03?  It's neat
        // R:
        
        // Q: After child->set_parent(parent), what is parent's reference count and why didn't it increase?
        // A:
        // R:
        
        // Q: Why is the child's reference count 2 but the parent's reference count is 1?
        // A:
        // R:
        
        // Q: When parent goes out of scope, can it be destroyed even though child holds a weak_ptr to it?
        // A:
        // R:
        
        // Q: What happens when child tries to access the parent after parent is destroyed?
        // A:
        // R:
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
    
    EXPECT_EQ(parent_count, 1);
    EXPECT_EQ(child_count, 1);
    EXPECT_EQ(after_link_parent_count, 1);
    EXPECT_EQ(after_link_child_count, 2);
    EXPECT_EQ(dtor_count, 2);  // No leak!
}

// Graph with weak_ptr edges
class GraphNode
{
public:
    explicit GraphNode(const std::string& name)
    : tracked_(name)
    {
    }
    
    void add_edge(std::shared_ptr<GraphNode> neighbor)
    {
        neighbors_.push_back(neighbor);
    }
    
    size_t neighbor_count() const
    {
        return neighbors_.size();
    }
    
private:
    Tracked tracked_;
    std::vector<std::weak_ptr<GraphNode>> neighbors_;
};

TEST_F(StructuralPatternsTest, GraphStructureWithWeakPtr)
{
    long node1_count = 0;
    long node2_count = 0;
    long node3_count = 0;
    
    {
        std::shared_ptr<GraphNode> node1 = std::make_shared<GraphNode>("Node1");
        std::shared_ptr<GraphNode> node2 = std::make_shared<GraphNode>("Node2");
        std::shared_ptr<GraphNode> node3 = std::make_shared<GraphNode>("Node3");
        // node1: 1
        // node2: 1
        // node3: 1
        
        node1->add_edge(node2);
        // node1: 1
        // node2: 1  (weak_ptr doesn't increment!)
        // node3: 1
        node1->add_edge(node3);
        // node1: 1
        // node2: 1
        // node3: 1  (weak_ptr doesn't increment!)
        node2->add_edge(node3);
        // node1: 1
        // node2: 1
        // node3: 1  (weak_ptr doesn't increment!)
        node3->add_edge(node1);
        // node1: 1  (weak_ptr doesn't increment!)
        // node2: 1
        // node3: 1
        
        node1_count = node1.use_count();
        node2_count = node2.use_count();
        node3_count = node3.use_count();
        
        // Q: Why do all three nodes have a reference count of 1 despite having edges between them?
        // A:
        // R:
        
        // Q: What would happen if GraphNode stored shared_ptr instead of weak_ptr in its neighbors_ vector?
        // A:
        // A:
        // A:
        // A:
        // R:
        
        // Q: How does storing weak_ptr edges allow arbitrary graph structures without leaks?
        // A:
        // R:
        
        // Q: What happens when you try to traverse an edge to a node that has been destroyed?
        // A:
        // A:
        // A:
        // R:
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
    
    EXPECT_EQ(node1_count, 1);
    EXPECT_EQ(node2_count, 1);
    EXPECT_EQ(node3_count, 1);
    EXPECT_EQ(dtor_count, 3);
}

TEST_F(StructuralPatternsTest, SelfReferencingNode)
{
    long before_self_ref = 0;
    long after_self_ref = 0;
    
    {
        std::shared_ptr<Node> node = std::make_shared<Node>("SelfNode");
        
        before_self_ref = node.use_count();
        
        node->set_next(node);
        
        after_self_ref = node.use_count();
        
        // Q: Why does the reference count increase from 1 to 2 when the node references itself?
        // A:
        // A:
        // R:
        
        // Q: When node goes out of scope, what is its use_count?
        // A:
        // R:
        
        // Q: Why can't the node be destroyed even though the local variable goes out of scope?
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
        // R:
        // R:
        // R:
        // R:
        
        // Q: Is a self-reference fundamentally different from a circular reference between two nodes?
        // A:
        // R:
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
    
    EXPECT_EQ(before_self_ref, 1);
    EXPECT_EQ(after_self_ref, 2);
    EXPECT_EQ(dtor_count, 0);  // Self-reference creates leak!
}
