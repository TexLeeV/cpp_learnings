// Test Suite: Algorithm Complexity and Parallel Algorithms
// Estimated Time: 3 hours
// Difficulty: Moderate


#include "instrumentation.h"
#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <numeric>
#if defined(__has_include)
#if __has_include(<execution>)
#include <execution>
#define HAS_STD_EXECUTION_POLICIES 1
#else
#define HAS_STD_EXECUTION_POLICIES 0
#endif
#else
#include <execution>
#define HAS_STD_EXECUTION_POLICIES 1
#endif

#if HAS_STD_EXECUTION_POLICIES
#if !defined(__cpp_lib_execution) || (__cpp_lib_execution < 201603L)
#undef HAS_STD_EXECUTION_POLICIES
#define HAS_STD_EXECUTION_POLICIES 0
#endif
#endif

class AlgorithmsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Algorithm Complexity Guarantees
// ============================================================================

TEST_F(AlgorithmsTest, Find_LinearComplexity)
{
    // Easy: std::find has linear complexity
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    auto it = std::find(vec.begin(), vec.end(), 3);
    
    EXPECT_NE(it, vec.end());
    EXPECT_EQ(*it, 3);
    
    // Q: What is the time complexity of std::find?
    // A:
    // R:
    
    // Q: Why can't std::find be faster than O(n)?
    // A:
    // R:
}

TEST_F(AlgorithmsTest, BinarySearch_LogarithmicComplexity)
{
    // Moderate: std::binary_search requires sorted range
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    bool found = std::binary_search(vec.begin(), vec.end(), 3);
    EXPECT_TRUE(found);
    
    // Q: What is the time complexity of std::binary_search?
    // A:
    // R:
    
    // Q: What precondition must be met for binary_search?
    // A:
    // R:
    
    std::vector<int> unsorted = {5, 2, 4, 1, 3};
    bool found_unsorted = std::binary_search(unsorted.begin(), unsorted.end(), 3);
    
    // Q: What happens if you call binary_search on unsorted data?
    // A:
    // R:
}

TEST_F(AlgorithmsTest, Sort_ComplexityGuarantee)
{
    // Moderate: std::sort has O(n log n) complexity guarantee
    
    std::vector<Tracked> vec;
    vec.push_back(Tracked("C"));
    vec.push_back(Tracked("A"));
    vec.push_back(Tracked("B"));
    
    EventLog::instance().clear();
    
    std::sort(vec.begin(), vec.end(), [](const Tracked& a, const Tracked& b)
    {
        EventLog::instance().record("Comparator called");
        return a.name() < b.name();
    });
    
    // Q: What is the worst-case complexity of std::sort?
    // A:
    // R:
    
    // Verify comparisons happened
    EXPECT_GT(EventLog::instance().count_events("Comparator called"), 0);
    
    // Q: How does std::sort differ from std::stable_sort?
    // A:
    // R:
}

// ============================================================================
// Parallel Algorithms (C++17)
// ============================================================================

TEST_F(AlgorithmsTest, ParallelAlgorithms_ExecutionPolicies)
{
    // Hard: C++17 parallel algorithms with execution policies
    
    std::vector<int> vec(1000);
    std::iota(vec.begin(), vec.end(), 0);
    
#if HAS_STD_EXECUTION_POLICIES
    // Sequential execution
    auto result1 = std::find(std::execution::seq, vec.begin(), vec.end(), 500);
    EXPECT_NE(result1, vec.end());
    
    // Parallel execution (may use multiple threads)
    auto result2 = std::find(std::execution::par, vec.begin(), vec.end(), 500);
    EXPECT_NE(result2, vec.end());
#else
    // Fallback for standard libraries without execution policy support
    auto result1 = std::find(vec.begin(), vec.end(), 500);
    auto result2 = std::find(vec.begin(), vec.end(), 500);
    EXPECT_NE(result1, vec.end());
    EXPECT_NE(result2, vec.end());
#endif
    
    // Q: What is the difference between std::execution::seq and std::execution::par?
    // A:
    // R:
    
    // Q: When would parallel execution be slower than sequential?
    // A:
    // R:
}

TEST_F(AlgorithmsTest, ParallelSort_ThreadSafety)
{
    // Hard: Parallel algorithms require thread-safe operations
    
    std::vector<int> vec = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    
#if HAS_STD_EXECUTION_POLICIES
    // TODO: Sort with parallel execution policy
    std::sort(std::execution::par, vec.begin(), vec.end());
#else
    // Fallback for standard libraries without execution policy support
    std::sort(vec.begin(), vec.end());
#endif
    
    EXPECT_TRUE(std::is_sorted(vec.begin(), vec.end()));
    
    // Q: What requirements does the comparator have for parallel sort?
    // A:
    // R:
}

// ============================================================================
// Algorithm Composition
// ============================================================================

TEST_F(AlgorithmsTest, Transform_Mapping)
{
    // Easy: std::transform applies function to range
    
    std::vector<int> input = {1, 2, 3, 4, 5};
    std::vector<int> output;
    
    std::transform(input.begin(), input.end(), std::back_inserter(output),
                   [](int x) { return x * 2; });
    
    EXPECT_EQ(output, std::vector<int>({2, 4, 6, 8, 10}));
    
    // Q: Can transform modify the input range in-place?
    // A:
    // R:
}

TEST_F(AlgorithmsTest, Accumulate_Reduction)
{
    // Easy: std::accumulate reduces range to single value
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    int sum = std::accumulate(vec.begin(), vec.end(), 0);
    EXPECT_EQ(sum, 15);
    
    // Custom operation
    int product = std::accumulate(vec.begin(), vec.end(), 1,
                                  [](int acc, int x) { return acc * x; });
    EXPECT_EQ(product, 120);
    
    // Q: What is the difference between accumulate and reduce?
    // A:
    // R:
}

// ============================================================================
// Algorithm Predicates
// ============================================================================

TEST_F(AlgorithmsTest, Predicates_UnaryAndBinary)
{
    // Moderate: Understanding predicate requirements
    
    std::vector<int> vec = {1, 2, 3, 4, 5, 6};
    
    // Unary predicate
    auto is_even = [](int x) { return x % 2 == 0; };
    
    auto it = std::find_if(vec.begin(), vec.end(), is_even);
    EXPECT_EQ(*it, 2);
    
    int count = std::count_if(vec.begin(), vec.end(), is_even);
    EXPECT_EQ(count, 3);
    
    // Q: What is a unary predicate?
    // A:
    // R:
    
    // Binary predicate for sorting
    std::sort(vec.begin(), vec.end(), std::greater<int>());
    EXPECT_EQ(vec[0], 6);
    
    // Q: What requirements must a predicate satisfy?
    // A:
    // R:
}

// ============================================================================
// Algorithm Return Values
// ============================================================================

TEST_F(AlgorithmsTest, Algorithm_IteratorReturns)
{
    // Moderate: Understanding what algorithms return
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // find returns iterator to found element or end()
    auto it1 = std::find(vec.begin(), vec.end(), 3);
    EXPECT_EQ(*it1, 3);
    
    auto it2 = std::find(vec.begin(), vec.end(), 99);
    EXPECT_EQ(it2, vec.end());
    
    // Q: Why do algorithms return iterators instead of indices?
    // A:
    // R:
    
    // remove returns iterator to new logical end
    auto new_end = std::remove(vec.begin(), vec.end(), 3);
    
    // Q: Does std::remove actually erase elements from the vector?
    // A:
    // R:
    
    EXPECT_EQ(vec.size(), 5);  // Size unchanged
    vec.erase(new_end, vec.end());  // Actually remove
    EXPECT_EQ(vec.size(), 4);
}
