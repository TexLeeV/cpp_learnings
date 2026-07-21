// Test Suite: Iterators and Invalidation
// Estimated Time: 1-2 hours
// Difficulty: Moderate
// C++ Standard: C++20

#include "instrumentation.h"

#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include <list>
#include <type_traits>
#include <vector>

class IteratorsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Categories and Traits (Easy)
// ============================================================================

TEST_F(IteratorsTest, CategoriesAndTraitsDistinguishContainers)
{
    using VecCat = typename std::iterator_traits<std::vector<int>::iterator>::iterator_category;
    using ListCat = typename std::iterator_traits<std::list<int>::iterator>::iterator_category;

    static_assert(std::is_same_v<VecCat, std::random_access_iterator_tag>);
    static_assert(std::is_same_v<ListCat, std::bidirectional_iterator_tag>);

    std::vector<int> v{10, 20, 30};
    auto vit = v.begin();
    vit += 2;
    EXPECT_EQ(*vit, 30);

    std::list<int> lst{10, 20, 30};
    auto lit = lst.begin();
    ++lit;
    ++lit;
    EXPECT_EQ(*lit, 30);

    // Q: Why can `vit += 2` compile for vector but the same expression is invalid for
    //    a list iterator?
    // A:
    // R:

    // Q: Why do algorithms query `iterator_traits` at compile time?
    // A:
    // R:
}

// ============================================================================
// Scenario 2: Vector Invalidation on Reallocation (Moderate)
// ============================================================================

TEST_F(IteratorsTest, VectorReallocationInvalidatesIterators)
{
    std::vector<int> v;
    v.reserve(2);
    v.push_back(1);
    v.push_back(2);

    const auto* data_before = v.data();
    auto it = v.begin();
    EXPECT_EQ(*it, 1);

    const auto cap_before = v.capacity();
    v.push_back(3);

    // Q: After capacity grew, why must you treat the old `it` as unusable?
    // A:
    // R:

    EXPECT_GT(v.capacity(), cap_before);
    EXPECT_NE(v.data(), data_before);
    EXPECT_EQ(*v.begin(), 1);
}

// ============================================================================
// Scenario 3: List Iterators Stable Across Insert (Moderate)
// ============================================================================

TEST_F(IteratorsTest, ListInsertDoesNotInvalidateExistingIterators)
{
    std::list<int> lst{1, 2, 3};
    auto it = lst.begin();
    ++it;
    EXPECT_EQ(*it, 2);

    lst.insert(lst.begin(), 0);
    lst.push_back(4);

    // Q: Why does `it` still name the element `2` after inserts at both ends?
    // A:
    // R:

    EXPECT_EQ(*it, 2);
    EXPECT_EQ(lst.front(), 0);
    EXPECT_EQ(lst.back(), 4);
    EXPECT_EQ(lst.size(), 5u);
}

// ============================================================================
// Scenario 4: Erase-Remove Idiom (Moderate)
// ============================================================================

TEST_F(IteratorsTest, EraseRemoveIdiomErasesSafely)
{
    std::vector<int> v{1, 2, 3, 2, 4, 2, 5};

    // Q: After `std::remove` alone, why is `v.size()` still 7?
    // A:
    // R:

    auto new_end = std::remove(v.begin(), v.end(), 2);
    EXPECT_EQ(v.size(), 7u);

    v.erase(new_end, v.end());

    // Q: What does the iterator returned by `std::remove` mark, and why must
    //    `erase` consume it to shrink the container?
    // A:
    // R:

    EXPECT_EQ(v, (std::vector<int>{1, 3, 4, 5}));
}
