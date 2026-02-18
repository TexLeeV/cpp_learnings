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
        // A: Because it created a circular dependency of shared_ptr ownerships
        // A: node1.next_ -> node2 -> node2.next_ -> node1
        // R: Correct. More precisely: node1 initially has use_count=1 (held by the local variable). When node2->set_next(node1) executes, node2's next_ member now holds a second shared_ptr to node1, incrementing its count to 2. Same logic for node2.
        // R: For junior SWEs: Each node is referenced twice: once by the local variable, once by the other node's next_ pointer.
        
        // Q: When node1 and node2 go out of scope at the closing brace, what prevents their destructors from being called?
        // A: node2's next_ circular dependency on node 1
        // R: Correct understanding. More precisely: When the local variables go out of scope, each node's use_count decrements from 2 to 1. But use_count=1 means "still alive"—node1 is kept alive by node2.next_, and node2 is kept alive by node1.next_. Neither can reach 0, so neither destructor runs.
        // R: For junior SWEs: Each node's count drops to 1 (not 0), so they stay alive, holding references to each other forever.
        
        // Q: Walk through what happens when node1 tries to be destroyed: what is its reference count, and what keeps it alive?
        // A: node1.reset() -> node1's use_count is still 1 due to node2.next_.  If you call node2->set_next(nullptr), then node1 will be released
        // R: Correct. When the local variable node1 goes out of scope, its destructor decrements node1's use_count from 2 to 1. The remaining reference is held by node2.next_. Since use_count=1 (not 0), node1 cannot be destroyed. You correctly identified that breaking the cycle (e.g., node2->set_next(nullptr)) would allow cleanup.
        // R: For junior SWEs: node1's count goes to 1 because node2 still holds a pointer to it. To fix it, you'd need to manually break the cycle.
        
        // Q: What observable signal confirms the memory leak?
        // A: My head
        // R: Your mental model is correct, but the question asks for the observable signal in the instrumentation. The observable signal is dtor_count=0 in the EventLog—zero destructor calls means the Tracked objects were never destroyed. In production, observable signals include: memory growth over time, missing cleanup logs, or memory profiler reports showing unreachable allocations.
        // R: For junior SWEs: The test checks that dtor_count=0, meaning no destructors ran. That's the proof of the leak.
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
        // A: No, it does not
        // R: Correct. weak_ptr does not increment the strong reference count (use_count). It only increments the weak reference count, which tracks how many weak_ptr instances exist but doesn't prevent object destruction.
        // R: For junior SWEs: weak_ptr doesn't increase use_count. It observes the object without owning it.
        
        // Q: Why does the reference count remain at 1 for both nodes after setting up the cycle?
        // A: Because std::weak_ptr has its own atomic counter that is responsible for releasing the control_block memory
        // A: And weak_ptr's do not increment use_count when initialized
        // R: Correct. The control block maintains two counters: strong count (use_count) and weak count. weak_ptr increments only the weak count. Since only the local variables hold strong references, use_count stays at 1 for each node. The weak count prevents control block deallocation until all weak_ptr instances are gone, but doesn't prevent object destruction.
        // R: For junior SWEs: weak_ptr has its own counter separate from use_count. It doesn't keep the object alive.
        
        // Q: When node1 goes out of scope, what is its reference count and can it be destroyed?
        // A: It's reference count decrements to 0 and it is then destroyed
        // A: Because of this, node1's object is released but doesn't this mean that the control_block memory is leaked because of the dangling weak_ptr in node2?
        // R: Excellent question showing deep understanding. The control block is NOT leaked. The control block has two-phase cleanup: (1) when use_count reaches 0, the object is destroyed, (2) when weak_count reaches 0, the control block itself is deallocated. So node2's weak_ptr keeps the control block alive (to track that the object is expired), but the WeakNode object itself is destroyed. When node2 goes out of scope, its weak_ptr destructor decrements weak_count to 0, deallocating the control block.
        // R: For junior SWEs: The control block stays alive until all weak_ptrs are gone, but the object is destroyed when use_count hits 0. No leak.
        
        // Q: How does weak_ptr break the cycle that shared_ptr creates?
        // A: It breaks the cycle by allowing the de-allocation of memory for the WeakNode object because weak_ptr does not claim ownership(i.e. increment use_count)
        // R: Correct. weak_ptr breaks the cycle by not participating in ownership. With shared_ptr, both nodes kept each other alive (use_count never reached 0). With weak_ptr, the cycle exists structurally but not in terms of ownership—when the local variables go out of scope, use_count reaches 0 for both nodes, allowing destruction.
        // R: For junior SWEs: weak_ptr doesn't count as ownership, so the cycle can't keep objects alive.
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
        // A: 2.  Because add_child adds it to a std::shared_ptr, which increments count by 1
        // R: Correct. The child's use_count goes from 1 to 2. The local variable holds one reference, and parent's children_ vector holds the second reference (stored as shared_ptr<Child>).
        // R: For junior SWEs: Two shared_ptrs point to child: the local variable and the parent's vector.
        
        // Q: After child->set_parent(parent), what is parent's reference count and why?
        // A: 2.  Because add_parent adds it to a std::shared_ptr, which increments count by 1
        // R: Correct. The parent's use_count goes from 1 to 2. The local variable holds one reference, and child's parent_ member holds the second reference (stored as shared_ptr<Parent>).
        // R: For junior SWEs: Two shared_ptrs point to parent: the local variable and the child's member.
        
        // Q: Why do both parent and child have a reference count of 2 after linking?
        // A: Because the explanation above
        // R: Correct. Each object is referenced twice: once by its local variable, once by the other object's member. This bidirectional shared_ptr ownership creates the cycle.
        // R: For junior SWEs: Both have count=2 because they point to each other with shared_ptr.
        
        // Q: When parent goes out of scope, why can't it be destroyed?
        // A: Because child still holds an instance ownership of parent.
        // R: Correct. When the local variable parent goes out of scope, parent's use_count decrements from 2 to 1. The remaining reference is held by child.parent_, so parent cannot be destroyed. Similarly, child cannot be destroyed because parent.children_ holds a reference to it. They keep each other alive.
        // R: For junior SWEs: parent's count drops to 1 (not 0) because child still points to it.
        
        // Q: In a parent-child relationship, which direction should typically use weak_ptr to avoid cycles?
        // A: child should have a weak_ptr to parent to avoid cycles.  In this case of parent going out of scope, it will release the child ownership and child becomes "orphaned" and parent can be released
        // A: Additionally, the name "set_parent" kind of gives it away.  Unless I am completely wrong with my answer.  Correct me if I'm wrong, of course
        // R: Correct. Child-to-parent should use weak_ptr. The reasoning: parents typically own their children (strong ownership), while children merely observe or reference their parent (weak observation). When the parent is destroyed, children become orphaned—this is the correct semantic. Your intuition about the naming is spot-on: "set_parent" suggests observation, while "add_child" suggests ownership.
        // R: For junior SWEs: Parent owns children (shared_ptr), children observe parent (weak_ptr). This matches real-world relationships.
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
        // A: 2.  Increments use_count due to shared_ptr<WeakChild>
        // AA: I never considered this inline forward declaration syntax: "std::vector<std::shared_ptr<class WeakChild>> children_;".  Is this possible in c++03?  It's neat
        // R: Correct. Child's use_count is 2 (local variable + parent's vector). Re: forward declaration—yes, the "class" keyword in template arguments has been valid since C++98. It's called an elaborated-type-specifier and allows you to forward-declare within the template argument. Modern style often omits "class" if the type is already declared, but it's useful for avoiding header dependencies.
        // R: For junior SWEs: Child's count is 2 because parent owns it. The "class WeakChild" syntax is a forward declaration trick.
        
        // Q: After child->set_parent(parent), what is parent's reference count and why didn't it increase?
        // A: 1.  weak_ptr does not increment use_count
        // R: Correct. Parent's use_count remains 1 because weak_ptr doesn't increment the strong reference count. The child observes the parent without claiming ownership.
        // R: For junior SWEs: Parent's count stays 1 because weak_ptr doesn't count as ownership.
        
        // Q: Why is the child's reference count 2 but the parent's reference count is 1?
        // A: Because Parent has ownership of child to match the pattern of parent->child semantic ownership
        // R: Correct. This asymmetry reflects the ownership model: parent owns child (shared_ptr, use_count=2), child observes parent (weak_ptr, use_count=1). This is the correct pattern for parent-child relationships.
        // R: For junior SWEs: Parent owns child (count=2), child just watches parent (count=1).
        
        // Q: When parent goes out of scope, can it be destroyed even though child holds a weak_ptr to it?
        // A: It will be destroyed due to previous explanation prior
        // R: Correct. Parent's use_count drops from 1 to 0 when the local variable goes out of scope. The weak_ptr in child doesn't prevent destruction—it only observes. Parent is destroyed, child becomes orphaned but remains alive (its use_count is still 1 from parent's vector).
        // R: For junior SWEs: Yes, parent is destroyed. weak_ptr doesn't keep objects alive.
        
        // Q: What happens when child tries to access the parent after parent is destroyed?
        // A: It will return a nullptr and access that nullptr will result in a segfault
        // R: Almost correct. parent_.lock() returns an empty shared_ptr (which compares equal to nullptr), but accessing it won't automatically segfault—it depends on how you use it. If you dereference the empty shared_ptr directly (e.g., parent_.lock()->method()), that will segfault. Best practice: check the result: if (auto p = parent_.lock()) { p->method(); } else { /* parent is gone */ }
        // R: For junior SWEs: lock() returns empty shared_ptr if parent is dead. Check it before using to avoid crashes.
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
        // A: Because add_edge stores the object as a weak_ptr, which does not increment use_count
        // R: Correct! Your manual tracking above had an error—weak_ptr never increments use_count. All three nodes remain at use_count=1 (held only by their local variables). The edges exist structurally but don't create ownership.
        // R: For junior SWEs: weak_ptr edges don't count as ownership, so all counts stay at 1.
        
        // Q: What would happen if GraphNode stored shared_ptr instead of weak_ptr in its neighbors_ vector?
        // A: use_count will change for all of them
        // A: node1's use_count: 2
        // A: node2's use_count: 2
        // A: node3's use_count: 3
        // R: Correct! With shared_ptr edges: node1 would be referenced by (local var + node3's edge) = 2, node2 by (local var + node1's edge) = 2, node3 by (local var + node1's edge + node2's edge) = 3. This creates a cycle (node1→node2→node3→node1), and when local variables go out of scope, all three would leak because each still has use_count ≥ 1.
        // R: For junior SWEs: With shared_ptr edges, you'd get a cycle and all three nodes would leak.
        
        // Q: How does storing weak_ptr edges allow arbitrary graph structures without leaks?
        // A: Now when you release a shared_ptr, like node2, it will be deleted
        // R: Correct. With weak_ptr edges, only the local variables hold ownership. When they go out of scope, use_count drops to 0 for each node, allowing destruction. The edges don't prevent cleanup because they're weak references. This allows cycles, bidirectional edges, and arbitrary graph topologies without leaks.
        // R: For junior SWEs: weak_ptr edges don't prevent deletion, so nodes can be freed when their owners are done.
        
        // Q: What happens when you try to traverse an edge to a node that has been destroyed?
        // A: two cases:
        // A: 1. If you dereference it, will result into a segfault
        // A: 2. If you don't dereference it, nothing will happen
        // R: Correct understanding. More precisely: when you call weak_ptr.lock() on an edge to a destroyed node, it returns an empty shared_ptr. If you check it (if (auto neighbor = edge.lock()) {...}), you safely detect the dead node. If you blindly dereference (edge.lock()->method()), you get undefined behavior (typically segfault). Best practice: always check lock() result before use.
        // R: For junior SWEs: lock() returns empty if node is dead. Check it before using to avoid crashes.
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
        // A: Because it behaves like it always behaves.  Another shared_ptr claims reference ownership of the control block and Node object
        // A: To avoid this, added conditional if code is needed to ensure it can't reference itself.  Design by Contract pattern works here
        // R: Correct. node->set_next(node) copies the shared_ptr into the node's next_ member, incrementing use_count from 1 to 2. The node now has two shared_ptr references: the local variable and its own internal member. Your point about prevention is valid—defensive checks can prevent self-reference if it's unintended.
        // R: For junior SWEs: The node points to itself, so use_count goes to 2 (local variable + internal pointer).
        
        // Q: When node goes out of scope, what is its use_count?
        // A: This will in a memory leak and the use_count will go to 1 upon node going out of scope
        // R: Correct. When the local variable goes out of scope, use_count decrements from 2 to 1. The remaining reference is held by node.next_ (which points to itself). Since use_count=1, the node cannot be destroyed—it's a leak.
        // R: For junior SWEs: use_count drops to 1 (not 0), so the node stays alive forever—that's a leak.
        
        // Q: Why can't the node be destroyed even though the local variable goes out of scope?
        // A: Because Object is kept alive due to node->set_next(node);, thus increasing the use_count by 1
        // R: Correct. Process flow when local variable goes out of scope:
        // R: 1. Local variable `node` destructor runs
        // R: 2. Decrements control block use_count: 2 → 1
        // R: 3. Checks: use_count == 0? No (it's 1)
        // R: 4. Does NOT call Node destructor (use_count > 0)
        // R: 5. Does NOT deallocate Node object
        // R: 6. Local variable is gone, but Node object still exists in memory
        // R: 7. Node.next_ still holds a shared_ptr to the Node (use_count = 1)
        // R: 8. For Node destructor to run, use_count must reach 0
        // R: 9. For use_count to reach 0, Node.next_ must be destroyed
        // R: 10. For Node.next_ to be destroyed, Node destructor must run
        // R: Deadlock: Step 8 requires step 9, step 9 requires step 10, step 10 requires step 8.
        // R: The Node object exists in memory forever with no external references—a leak.
        // R: For junior SWEs: The node holds a pointer to itself, keeping itself alive. It can never be freed.
        
        // Q: Is a self-reference fundamentally different from a circular reference between two nodes?
        // A: Yes, seems as though self-reference result in an actual memory leak while circular reference still has dependency against other resources that can still go out of scope and release resources
        // R: Actually, they're fundamentally the same—both are memory leaks. A self-reference is just a circular reference with cycle length 1 (node→node), while a two-node cycle has length 2 (node1→node2→node1). Both result in use_count never reaching 0. The difference you're sensing might be: with two nodes, you could theoretically break the cycle by accessing one node and calling reset() on its pointer to the other. With a self-reference, once the local variable is gone, you have no way to access the node to break the cycle. But mechanically, both are leaks caused by cycles.
        // R: For junior SWEs: Both are leaks. Self-reference is just a cycle of length 1. Same problem, different shape.
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
