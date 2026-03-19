// Test Suite: Container Internals (vector, deque, map vs unordered_map)
// Estimated Time: 3 hours
// Difficulty: Easy
// C++ Standard: C++17

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <list>

class ContainerInternalsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// std::vector Growth Strategy
// ============================================================================

TEST_F(ContainerInternalsTest, Vector_GrowthStrategy)
{
    // Easy: Understanding vector capacity and reallocation
    
    std::vector<Tracked> vec;
    
    EventLog::instance().record("Initial capacity: " + std::to_string(vec.capacity()));
    
    // Q: What is the initial capacity of an empty vector?
    // A:
    // R:
    
    vec.push_back(Tracked("Item1"));
    size_t cap1 = vec.capacity();
    EventLog::instance().record("After 1 push: capacity=" + std::to_string(cap1));
    
    vec.push_back(Tracked("Item2"));
    size_t cap2 = vec.capacity();
    EventLog::instance().record("After 2 push: capacity=" + std::to_string(cap2));
    
    vec.push_back(Tracked("Item3"));
    size_t cap3 = vec.capacity();
    EventLog::instance().record("After 3 push: capacity=" + std::to_string(cap3));
    
    // Q: How does vector capacity grow? (e.g., linear, exponential)
    // A:
    // R:
    
    // Q: Why does vector grow exponentially rather than by a fixed amount?
    // A:
    // R:
    
    EXPECT_GT(cap3, 0);
}

TEST_F(ContainerInternalsTest, Vector_ReallocationCost)
{
    // Moderate: Observing reallocation through move operations
    
    std::vector<Tracked> vec;
    vec.reserve(2);
    
    vec.push_back(Tracked("A"));
    vec.push_back(Tracked("B"));
    
    EventLog::instance().clear();
    
    // Force reallocation
    vec.push_back(Tracked("C"));
    
    // Q: What happens to existing elements during reallocation?
    // A:
    // R:
    
    // Verify moves occurred (A and B moved to new storage)
    EXPECT_GE(EventLog::instance().count_events("::move_ctor"), 2);
    
    // Q: Why does vector move existing elements instead of copying?
    // A:
    // R:
}

TEST_F(ContainerInternalsTest, Vector_ReserveVsResize)
{
    // Easy: Understanding reserve vs resize
    
    std::vector<Tracked> vec1;
    vec1.reserve(5);
    
    // Q: How many Tracked objects were constructed after reserve(5)?
    // A:
    // R:
    
    EXPECT_EQ(vec1.size(), 0);
    EXPECT_GE(vec1.capacity(), 5);
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 0);
    
    EventLog::instance().clear();
    
    std::vector<Tracked> vec2;
    vec2.resize(5, Tracked("Default"));
    
    // Q: How many Tracked objects were constructed after resize(5)?
    // A:
    // R:
    
    EXPECT_EQ(vec2.size(), 5);
    // One ctor for the default value, then copies/moves for the 5 elements
    EXPECT_GE(EventLog::instance().count_events("::ctor"), 1);
}

// ============================================================================
// std::deque Structure
// ============================================================================

TEST_F(ContainerInternalsTest, Deque_NoReallocation)
{
    // Moderate: deque doesn't invalidate references on push
    
    std::deque<Tracked> deq;
    
    deq.push_back(Tracked("A"));
    deq.push_back(Tracked("B"));
    
    EventLog::instance().clear();
    
    // Add many elements - no reallocation of existing elements
    for (int i = 0; i < 100; ++i)
    {
        deq.push_back(Tracked("Item"));
    }
    
    // Q: How many move operations occurred on existing elements A and B?
    // A:
    // R:
    
    // Verify A and B were not moved (deque uses chunked storage)
    size_t move_count = EventLog::instance().count_events("Tracked(A)::move");
    move_count += EventLog::instance().count_events("Tracked(B)::move");
    EXPECT_EQ(move_count, 0);
    
    // Q: How does deque achieve this without reallocation?
    // A:
    // R:
}

TEST_F(ContainerInternalsTest, Deque_FrontAndBackInsertion)
{
    // Easy: deque supports efficient insertion at both ends
    
    std::deque<int> deq;
    
    deq.push_back(1);
    deq.push_front(0);
    deq.push_back(2);
    deq.push_front(-1);
    
    EXPECT_EQ(deq.size(), 4);
    EXPECT_EQ(deq[0], -1);
    EXPECT_EQ(deq[1], 0);
    EXPECT_EQ(deq[2], 1);
    EXPECT_EQ(deq[3], 2);
    
    // Q: What is the time complexity of push_front for vector vs deque?
    // A:
    // R:
}

// ============================================================================
// std::map vs std::unordered_map
// ============================================================================

TEST_F(ContainerInternalsTest, Map_OrderedIteration)
{
    // Easy: std::map maintains sorted order
    
    std::map<int, std::string> ordered_map;
    ordered_map[3] = "three";
    ordered_map[1] = "one";
    ordered_map[2] = "two";
    
    std::vector<int> keys;
    for (const auto& pair : ordered_map)
    {
        keys.push_back(pair.first);
    }
    
    EXPECT_EQ(keys, std::vector<int>({1, 2, 3}));
    
    // Q: What data structure does std::map use internally?
    // A:
    // R:
    
    // Q: What is the time complexity of map::find?
    // A:
    // R:
}

TEST_F(ContainerInternalsTest, UnorderedMap_HashBased)
{
    // Moderate: std::unordered_map uses hash table
    
    std::unordered_map<int, std::string> hash_map;
    hash_map[3] = "three";
    hash_map[1] = "one";
    hash_map[2] = "two";
    
    // Iteration order is unspecified (based on hash)
    EXPECT_EQ(hash_map.size(), 3);
    
    // Q: What is the average time complexity of unordered_map::find?
    // A:
    // R:
    
    // Q: When would you choose map over unordered_map?
    // A:
    // R:
    
    // Check load factor
    float load = hash_map.load_factor();
    EXPECT_GT(load, 0.0f);
    EXPECT_LE(load, hash_map.max_load_factor());
}

// ============================================================================
// Container Memory Layout
// ============================================================================

TEST_F(ContainerInternalsTest, Vector_ContiguousMemory)
{
    // Moderate: vector guarantees contiguous storage
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    int* ptr = vec.data();
    
    // Q: What does vec.data() return?
    // A:
    // R:
    
    // Verify contiguous memory
    for (size_t i = 0; i < vec.size(); ++i)
    {
        EXPECT_EQ(ptr[i], vec[i]);
        EXPECT_EQ(&ptr[i], &vec[i]);
    }
    
    // Q: Which other STL containers guarantee contiguous storage?
    // A:
    // R:
}

TEST_F(ContainerInternalsTest, List_NodeBasedStorage)
{
    // Easy: std::list uses node-based storage
    
    std::list<Tracked> lst;
    
    lst.push_back(Tracked("A"));
    lst.push_back(Tracked("B"));
    lst.push_back(Tracked("C"));
    
    EventLog::instance().clear();
    
    // Insert in middle - no moves of existing elements
    auto it = lst.begin();
    ++it;
    lst.insert(it, Tracked("Middle"));
    
    // Q: How many existing elements were moved during insert?
    // A:
    // R:
    
    EXPECT_EQ(EventLog::instance().count_events("Tracked(A)::move"), 0);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(B)::move"), 0);
    EXPECT_EQ(EventLog::instance().count_events("Tracked(C)::move"), 0);
    
    // Q: What is the trade-off between list and vector for insertion?
    // A:
    // R:
}

// ============================================================================
// Small String Optimization (SSO)
// ============================================================================

TEST_F(ContainerInternalsTest, String_SmallStringOptimization)
{
    // Hard: std::string may use SSO for small strings
    
    std::string small = "short";
    std::string large = "this is a much longer string that exceeds SSO threshold";
    
    // Q: What is Small String Optimization?
    // A:
    // R:
    
    // Q: Why does SSO improve performance for small strings?
    // A:
    // R:
    
    // Note: SSO threshold is implementation-defined (typically 15-23 bytes)
    // We can't directly test SSO, but can observe its effects
}

// ============================================================================
// Container Complexity Guarantees
// ============================================================================

TEST_F(ContainerInternalsTest, Container_ComplexityGuarantees)
{
    // Moderate: Understanding time complexity of operations
    
    std::vector<int> vec = {1, 2, 3};
    std::list<int> lst = {1, 2, 3};
    std::deque<int> deq = {1, 2, 3};
    
    // Q: What is the complexity of vec.push_back() amortized?
    // A:
    // R:
    
    // Q: What is the complexity of lst.insert() at any position?
    // A:
    // R:
    
    // Q: What is the complexity of deq.push_front()?
    // A:
    // R:
    
    // Q: Which container provides O(1) random access?
    // A:
    // R:
}
