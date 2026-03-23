// Test Suite: Iterator Categories and Custom Iterators
// Estimated Time: 3 hours
// Difficulty: Moderate


#include "instrumentation.h"
#include <gtest/gtest.h>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>

class IteratorsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Iterator Categories
// ============================================================================

TEST_F(IteratorsTest, IteratorCategories_Hierarchy)
{
    // Easy: Understanding the five iterator categories
    
    std::vector<int> vec = {1, 2, 3};
    std::list<int> lst = {1, 2, 3};
    
    auto vec_it = vec.begin();
    auto lst_it = lst.begin();
    
    // Q: What iterator category does std::vector::iterator provide?
    // A:
    // R:
    
    // Q: What iterator category does std::list::iterator provide?
    // A:
    // R:
    
    // Random access iterator supports arithmetic
    vec_it += 2;
    EXPECT_EQ(*vec_it, 3);
    
    // Bidirectional iterator requires increment/decrement
    ++lst_it;
    ++lst_it;
    EXPECT_EQ(*lst_it, 3);
    
    // Q: Can you do lst_it += 2? Why or why not?
    // A:
    // R:
}

TEST_F(IteratorsTest, IteratorTraits_CompileTimeQuery)
{
    // Moderate: Using iterator_traits to query iterator properties
    
    using VecIter = std::vector<int>::iterator;
    using ListIter = std::list<int>::iterator;
    
    using VecCategory = typename std::iterator_traits<VecIter>::iterator_category;
    using ListCategory = typename std::iterator_traits<ListIter>::iterator_category;
    
    static_assert(std::is_same_v<VecCategory, std::random_access_iterator_tag>,
                  "vector iterator should be random access");
    
    static_assert(std::is_same_v<ListCategory, std::bidirectional_iterator_tag>,
                  "list iterator should be bidirectional");
    
    // Q: Why do algorithms need to query iterator categories?
    // A:
    // R:
}

// ============================================================================
// Custom Iterator Implementation
// ============================================================================

template<typename T>
class RangeIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    
    explicit RangeIterator(T value)
    : current_(value)
    {
        EventLog::instance().record("RangeIterator::ctor");
    }
    
    T operator*() const { return current_; }
    
    RangeIterator& operator++()
    {
        ++current_;
        return *this;
    }
    
    RangeIterator operator++(int)
    {
        RangeIterator temp = *this;
        ++current_;
        return temp;
    }
    
    bool operator==(const RangeIterator& other) const
    {
        return current_ == other.current_;
    }
    
    bool operator!=(const RangeIterator& other) const
    {
        return current_ != other.current_;
    }

private:
    T current_;
};

// Q: What typedefs must a custom iterator provide?
// A:
// R:

TEST_F(IteratorsTest, CustomIterator_ForwardIterator)
{
    // Hard: Implementing a custom forward iterator
    
    RangeIterator<int> begin(0);
    RangeIterator<int> end(5);
    
    std::vector<int> result;
    for (auto it = begin; it != end; ++it)
    {
        result.push_back(*it);
    }
    
    EXPECT_EQ(result, std::vector<int>({0, 1, 2, 3, 4}));
    
    // Q: Can RangeIterator be used with std::sort? Why or why not?
    // A:
    // R:
}

// ============================================================================
// Iterator Adaptors
// ============================================================================

TEST_F(IteratorsTest, ReverseIterator_Adaptor)
{
    // Easy: std::reverse_iterator adapts bidirectional iterators
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    std::vector<int> reversed;
    for (auto it = vec.rbegin(); it != vec.rend(); ++it)
    {
        reversed.push_back(*it);
    }
    
    EXPECT_EQ(reversed, std::vector<int>({5, 4, 3, 2, 1}));
    
    // Q: What does rbegin() return in terms of regular iterators?
    // A:
    // R:
    
    // Q: Can you use reverse_iterator with std::list?
    // A:
    // R:
}

TEST_F(IteratorsTest, BackInserter_OutputIterator)
{
    // Moderate: std::back_inserter creates output iterator
    
    std::vector<int> source = {1, 2, 3};
    std::vector<int> dest;
    
    std::copy(source.begin(), source.end(), std::back_inserter(dest));
    
    EXPECT_EQ(dest, source);
    
    // Q: What does back_inserter do differently than assigning to dest.begin()?
    // A:
    // R:
    
    // Q: What happens if you use dest.begin() instead of back_inserter with empty dest?
    // A:
    // R:
}

// ============================================================================
// Iterator Algorithms and Distance
// ============================================================================

TEST_F(IteratorsTest, IteratorDistance_Complexity)
{
    // Moderate: std::distance complexity varies by iterator category
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::list<int> lst = {1, 2, 3, 4, 5};
    
    auto vec_dist = std::distance(vec.begin(), vec.end());
    auto lst_dist = std::distance(lst.begin(), lst.end());
    
    EXPECT_EQ(vec_dist, 5);
    EXPECT_EQ(lst_dist, 5);
    
    // Q: What is the time complexity of std::distance for random access iterators?
    // A:
    // R:
    
    // Q: What is the time complexity of std::distance for bidirectional iterators?
    // A:
    // R:
}

TEST_F(IteratorsTest, IteratorAdvance_Optimization)
{
    // Moderate: std::advance optimizes based on iterator category
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    auto it = vec.begin();
    std::advance(it, 3);
    
    EXPECT_EQ(*it, 4);
    
    // Q: How does std::advance optimize for random access iterators?
    // A:
    // R:
    
    std::list<int> lst = {1, 2, 3, 4, 5};
    auto lst_it = lst.begin();
    std::advance(lst_it, 3);
    
    EXPECT_EQ(*lst_it, 4);
    
    // Q: How does std::advance work for bidirectional iterators?
    // A:
    // R:
}

// ============================================================================
// Iterator Invalidation Basics
// ============================================================================

TEST_F(IteratorsTest, Iterator_ValidityAfterModification)
{
    // Hard: Understanding when iterators remain valid
    
    std::vector<int> vec = {1, 2, 3};
    auto it = vec.begin();
    
    EXPECT_EQ(*it, 1);
    
    // Modify without reallocation
    vec[0] = 10;
    EXPECT_EQ(*it, 10);  // Iterator still valid
    
    // Q: When does vector invalidate iterators?
    // A:
    // R:
    
    vec.reserve(100);  // May reallocate
    // Q: Is 'it' still valid after reserve?
    // A:
    // R:
}

// ============================================================================
// const_iterator vs iterator
// ============================================================================

TEST_F(IteratorsTest, ConstIterator_Immutability)
{
    // Easy: const_iterator prevents modification
    
    std::vector<int> vec = {1, 2, 3};
    
    std::vector<int>::iterator it = vec.begin();
    *it = 10;  // OK
    EXPECT_EQ(vec[0], 10);
    
    std::vector<int>::const_iterator cit = vec.cbegin();
    // *cit = 20;  // Compile error
    
    EXPECT_EQ(*cit, 10);
    
    // Q: Can you convert iterator to const_iterator?
    // A:
    // R:
    
    // Q: Can you convert const_iterator to iterator?
    // A:
    // R:
}
