// Test Suite: Iterator Invalidation Rules
// Estimated Time: 3 hours
// Difficulty: Hard
// C++ Standard: C++17

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <unordered_map>
#include <algorithm>

class IteratorInvalidationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Vector Iterator Invalidation
// ============================================================================

TEST_F(IteratorInvalidationTest, Vector_InvalidationOnReallocation)
{
    // Hard: Vector iterators invalidate on reallocation
    
    std::vector<int> vec;
    vec.reserve(2);
    
    vec.push_back(1);
    vec.push_back(2);
    
    auto it = vec.begin();
    int* ptr = &vec[0];
    
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*ptr, 1);
    
    // Force reallocation
    vec.push_back(3);
    
    // Q: Are 'it' and 'ptr' still valid after reallocation?
    // A:
    // R:
    
    // Q: How can you detect if reallocation occurred?
    // A:
    // R:
    
    // Safe: get new iterator
    auto new_it = vec.begin();
    EXPECT_EQ(*new_it, 1);
}

TEST_F(IteratorInvalidationTest, Vector_InvalidationOnErase)
{
    // Moderate: Erase invalidates iterators at and after erase point
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    auto it1 = vec.begin();
    auto it2 = vec.begin() + 2;  // Points to 3
    auto it5 = vec.begin() + 4;  // Points to 5
    
    EXPECT_EQ(*it2, 3);
    
    // Erase element at position 2
    vec.erase(it2);
    
    // Q: Which iterators are invalidated after erasing position 2?
    // A:
    // R:
    
    // it1 still valid (before erase point)
    EXPECT_EQ(*it1, 1);
    
    // it2 and it5 are invalidated
    // Using them would be undefined behavior
}

TEST_F(IteratorInvalidationTest, Vector_InvalidationOnInsert)
{
    // Moderate: Insert may invalidate all iterators
    
    std::vector<int> vec = {1, 2, 3};
    vec.reserve(10);  // Ensure no reallocation
    
    auto it = vec.begin() + 1;
    EXPECT_EQ(*it, 2);
    
    // Insert without reallocation
    vec.insert(vec.begin(), 0);
    
    // Q: Is 'it' still valid after insert (no reallocation)?
    // A:
    // R:
    
    // Even without reallocation, insert invalidates all iterators
    // because elements shift
}

// ============================================================================
// List Iterator Invalidation
// ============================================================================

TEST_F(IteratorInvalidationTest, List_StableIterators)
{
    // Easy: List iterators remain valid on insert/erase
    
    std::list<Tracked> lst;
    lst.push_back(Tracked("A"));
    lst.push_back(Tracked("B"));
    lst.push_back(Tracked("C"));
    
    auto it_a = lst.begin();
    auto it_b = std::next(lst.begin());
    auto it_c = std::next(lst.begin(), 2);
    
    EXPECT_EQ(it_a->name(), "A");
    EXPECT_EQ(it_b->name(), "B");
    EXPECT_EQ(it_c->name(), "C");
    
    EventLog::instance().clear();
    
    // Insert in middle
    lst.insert(it_b, Tracked("Middle"));
    
    // Q: Are it_a, it_b, it_c still valid after insert?
    // A:
    // R:
    
    EXPECT_EQ(it_a->name(), "A");
    EXPECT_EQ(it_b->name(), "B");
    EXPECT_EQ(it_c->name(), "C");
    
    // Erase middle element
    lst.erase(it_b);
    
    // Q: Which iterators are invalidated after erasing it_b?
    // A:
    // R:
    
    EXPECT_EQ(it_a->name(), "A");
    EXPECT_EQ(it_c->name(), "C");
    // it_b is now invalid
}

// ============================================================================
// Deque Iterator Invalidation
// ============================================================================

TEST_F(IteratorInvalidationTest, Deque_PartialInvalidation)
{
    // Hard: Deque has complex invalidation rules
    
    std::deque<int> deq = {1, 2, 3, 4, 5};
    
    auto it_middle = deq.begin() + 2;
    EXPECT_EQ(*it_middle, 3);
    
    // Insert at front
    deq.push_front(0);
    
    // Q: Is it_middle still valid after push_front?
    // A:
    // R:
    
    // Insert at back
    deq.push_back(6);
    
    // Q: Is it_middle still valid after push_back?
    // A:
    // R:
    
    // Insert in middle
    deq.insert(deq.begin() + 3, 99);
    
    // Q: Is it_middle still valid after insert in middle?
    // A:
    // R:
}

// ============================================================================
// Map Iterator Invalidation
// ============================================================================

TEST_F(IteratorInvalidationTest, Map_StableIterators)
{
    // Moderate: Map iterators remain valid except for erased elements
    
    std::map<int, std::string> map;
    map[1] = "one";
    map[2] = "two";
    map[3] = "three";
    
    auto it1 = map.find(1);
    auto it2 = map.find(2);
    auto it3 = map.find(3);
    
    EXPECT_EQ(it2->second, "two");
    
    // Erase element 2
    map.erase(it2);
    
    // Q: Are it1 and it3 still valid after erasing it2?
    // A:
    // R:
    
    EXPECT_EQ(it1->second, "one");
    EXPECT_EQ(it3->second, "three");
    
    // Insert new element
    map[4] = "four";
    
    // Q: Are it1 and it3 still valid after insert?
    // A:
    // R:
    
    EXPECT_EQ(it1->second, "one");
    EXPECT_EQ(it3->second, "three");
}

// ============================================================================
// Unordered Map Iterator Invalidation
// ============================================================================

TEST_F(IteratorInvalidationTest, UnorderedMap_RehashInvalidation)
{
    // Hard: Unordered map invalidates on rehash
    
    std::unordered_map<int, std::string> map;
    map.reserve(3);  // Prevent rehash initially
    
    map[1] = "one";
    map[2] = "two";
    
    auto it1 = map.find(1);
    EXPECT_EQ(it1->second, "one");
    
    float initial_load = map.load_factor();
    
    // Add many elements to trigger rehash
    for (int i = 10; i < 100; ++i)
    {
        map[i] = "value";
    }
    
    // Q: Is it1 still valid after potential rehash?
    // A:
    // R:
    
    // Q: What operation triggers rehash in unordered containers?
    // A:
    // R:
}

// ============================================================================
// Safe Iteration Patterns
// ============================================================================

TEST_F(IteratorInvalidationTest, SafeErasure_EraseRemoveIdiom)
{
    // Moderate: Erase-remove idiom for safe removal
    
    std::vector<int> vec = {1, 2, 3, 2, 4, 2, 5};
    
    // Remove all 2s
    auto new_end = std::remove(vec.begin(), vec.end(), 2);
    vec.erase(new_end, vec.end());
    
    EXPECT_EQ(vec, std::vector<int>({1, 3, 4, 5}));
    
    // Q: Why is this pattern called "erase-remove idiom"?
    // A:
    // R:
    
    // Q: What does std::remove actually do to the elements?
    // A:
    // R:
}

TEST_F(IteratorInvalidationTest, SafeErasure_EraseInLoop)
{
    // Hard: Safe erasure while iterating
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // TODO: Erase all even numbers safely
    for (auto it = vec.begin(); it != vec.end(); )
    {
        if (*it % 2 == 0)
        {
            it = vec.erase(it);  // erase returns next valid iterator
        }
        else
        {
            ++it;
        }
    }
    
    EXPECT_EQ(vec, std::vector<int>({1, 3, 5}));
    
    // Q: Why must we use it = vec.erase(it) instead of vec.erase(it); ++it?
    // A:
    // R:
}

TEST_F(IteratorInvalidationTest, SafeErasure_ListErase)
{
    // Moderate: List erase returns next iterator
    
    std::list<int> lst = {1, 2, 3, 4, 5};
    
    for (auto it = lst.begin(); it != lst.end(); )
    {
        if (*it % 2 == 0)
        {
            it = lst.erase(it);
        }
        else
        {
            ++it;
        }
    }
    
    std::vector<int> result(lst.begin(), lst.end());
    EXPECT_EQ(result, std::vector<int>({1, 3, 5}));
    
    // Q: Does list::erase have the same invalidation rules as vector::erase?
    // A:
    // R:
}

// ============================================================================
// Iterator Invalidation Summary
// ============================================================================

TEST_F(IteratorInvalidationTest, Invalidation_ContainerComparison)
{
    // Hard: Understanding invalidation rules across containers
    
    // Q: Which container has the most stable iterators?
    // A:
    // R:
    
    // Q: Which container invalidates iterators most aggressively?
    // A:
    // R:
    
    // Q: When do map iterators get invalidated?
    // A:
    // R:
    
    // Q: When do unordered_map iterators get invalidated?
    // A:
    // R:
}
