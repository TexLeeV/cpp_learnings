// Test Suite: Comparators and Hash Functions
// Estimated Time: 3 hours
// Difficulty: Moderate

#include "instrumentation.h"

#include <algorithm>
#include <functional>
#include <gtest/gtest.h>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ComparatorsHashFunctionsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Custom Comparators for Ordered Containers
// ============================================================================

struct Person
{
    std::string name;
    int age;

    Person(std::string n, int a) : name(std::move(n)), age(a)
    {
        EventLog::instance().record("Person(" + name + ")::ctor");
    }
};

struct CompareByAge
{
    bool operator()(const Person& a, const Person& b) const
    {
        EventLog::instance().record("CompareByAge called");
        return a.age < b.age;
    }
};

TEST_F(ComparatorsHashFunctionsTest, CustomComparator_Set)
{
    // Moderate: Using custom comparator with std::set

    std::set<Person, CompareByAge> people;

    people.emplace("Alice", 30);
    people.emplace("Bob", 25);
    people.emplace("Charlie", 35);

    // Q: In what order are the people stored in the set?
    // A:
    // R:

    auto it = people.begin();
    EXPECT_EQ(it->name, "Bob");
    ++it;
    EXPECT_EQ(it->name, "Alice");
    ++it;
    EXPECT_EQ(it->name, "Charlie");

    // Q: What happens if two people have the same age?
    // A:
    // R:
}

TEST_F(ComparatorsHashFunctionsTest, Comparator_StrictWeakOrdering)
{
    // Hard: Understanding strict weak ordering requirements

    auto bad_comparator = [](int a, int b) {
        EventLog::instance().record("bad_comparator called");
        return a <= b; // WRONG: not strict weak ordering
    };

    auto good_comparator = [](int a, int b) {
        EventLog::instance().record("good_comparator called");
        return a < b; // CORRECT: strict weak ordering
    };

    // Q: What are the requirements for strict weak ordering?
    // A:
    // R:

    // Q: Why does <= violate strict weak ordering?
    // A:
    // R:

    std::vector<int> vec = {3, 1, 2};
    std::sort(vec.begin(), vec.end(), good_comparator);
    EXPECT_TRUE(std::is_sorted(vec.begin(), vec.end()));
}

// ============================================================================
// Hash Functions for Unordered Containers
// ============================================================================

struct PersonHash
{
    std::size_t operator()(const Person& p) const
    {
        EventLog::instance().record("PersonHash called for " + p.name);

        // Combine name and age hashes
        std::size_t h1 = std::hash<std::string>{}(p.name);
        std::size_t h2 = std::hash<int>{}(p.age);

        return h1 ^ (h2 << 1);
    }
};

struct PersonEqual
{
    bool operator()(const Person& a, const Person& b) const
    {
        EventLog::instance().record("PersonEqual called");
        return a.name == b.name && a.age == b.age;
    }
};

TEST_F(ComparatorsHashFunctionsTest, CustomHash_UnorderedSet)
{
    // Hard: Custom hash function for unordered containers

    std::unordered_set<Person, PersonHash, PersonEqual> people;

    people.emplace("Alice", 30);
    people.emplace("Bob", 25);

    // Q: Why do unordered containers need both a hash function and equality operator?
    // A:
    // R:

    EXPECT_EQ(people.size(), 2);

    // Try to insert duplicate
    people.emplace("Alice", 30);
    EXPECT_EQ(people.size(), 2); // No duplicate

    // Q: What happens when two different objects have the same hash?
    // A:
    // R:
}

TEST_F(ComparatorsHashFunctionsTest, HashCollisions_BucketStructure)
{
    // Hard: Understanding hash collision handling

    std::unordered_map<int, std::string> map;

    map[1] = "one";
    map[2] = "two";
    map[3] = "three";

    // Q: How does unordered_map handle hash collisions?
    // A:
    // R:

    size_t bucket_count = map.bucket_count();
    EXPECT_GT(bucket_count, 0);

    // Check load factor
    float load = map.load_factor();
    EXPECT_GT(load, 0.0f);

    // Q: What happens when load factor exceeds max_load_factor?
    // A:
    // R:
}

// ============================================================================
// Transparent Comparators (C++14)
// ============================================================================

TEST_F(ComparatorsHashFunctionsTest, TransparentComparator_HeterogeneousLookup)
{
    // Hard: std::less<> enables heterogeneous lookup

    std::set<std::string, std::less<>> string_set;
    string_set.insert("hello");
    string_set.insert("world");

    // Can find using const char* without creating temporary string
    auto it = string_set.find("hello");
    EXPECT_NE(it, string_set.end());

    // Q: What advantage does std::less<> provide over std::less<std::string>?
    // A:
    // R:

    // Q: What is "transparent comparison"?
    // A:
    // R:
}

// ============================================================================
// Comparator Consistency
// ============================================================================

TEST_F(ComparatorsHashFunctionsTest, Comparator_ConsistencyRequirement)
{
    // Moderate: Comparator must be consistent with equality

    struct InconsistentCompare
    {
        bool operator()(const Person& a, const Person& b) const
        {
            // Compare by age only
            return a.age < b.age;
        }
    };

    struct InconsistentEqual
    {
        bool operator()(const Person& a, const Person& b) const
        {
            // Compare by name and age
            return a.name == b.name && a.age == b.age;
        }
    };

    // This creates inconsistency: two people with same age but different names
    // would be "equal" by comparator but "not equal" by equality

    // Q: What problems arise when comparator and equality are inconsistent?
    // A:
    // R:
}

// ============================================================================
// Hash Function Quality
// ============================================================================

TEST_F(ComparatorsHashFunctionsTest, HashFunction_Distribution)
{
    // Hard: Good hash functions distribute values uniformly

    struct BadHash
    {
        std::size_t operator()(int x) const
        {
            return 42; // Always returns same hash!
        }
    };

    std::unordered_set<int, BadHash> bad_set;
    bad_set.insert(1);
    bad_set.insert(2);
    bad_set.insert(3);

    // Q: What is the time complexity of find() with BadHash?
    // A:
    // R:

    // Q: Why does a bad hash function degrade performance?
    // A:
    // R:

    // With bad hash, all elements collide (but bucket_count may be larger)
    EXPECT_GE(bad_set.bucket_count(), 1);
}
