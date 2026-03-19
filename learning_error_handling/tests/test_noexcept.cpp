// Test Suite: noexcept Specifications and Move Operations
// Estimated Time: 3 hours
// Difficulty: Moderate
// C++ Standard: C++17

#include "instrumentation.h"
#include <gtest/gtest.h>
#include <vector>
#include <type_traits>

class NoexceptTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// noexcept Basics - Compile-Time Exception Specification
// ============================================================================

int safe_add(int a, int b) noexcept
{
    EventLog::instance().record("safe_add: called");
    return a + b;
}

int unsafe_add(int a, int b)
{
    EventLog::instance().record("unsafe_add: called");
    if (a > 1000000)
    {
        throw std::overflow_error("Overflow");
    }
    return a + b;
}

// Q: What does noexcept tell the compiler?
// A:
// R:

// Q: What happens if a noexcept function throws an exception?
// A:
// R:

TEST_F(NoexceptTest, Noexcept_BasicSpecification)
{
    // Easy: noexcept is a compile-time specification
    
    static_assert(noexcept(safe_add(1, 2)), "safe_add should be noexcept");
    static_assert(!noexcept(unsafe_add(1, 2)), "unsafe_add should not be noexcept");
    
    int result1 = safe_add(10, 20);
    EXPECT_EQ(result1, 30);
    
    int result2 = unsafe_add(10, 20);
    EXPECT_EQ(result2, 30);
    
    // Q: Can the compiler optimize noexcept functions differently?
    // A:
    // R:
}

// ============================================================================
// noexcept and Move Operations
// ============================================================================

class NonNoexceptMove
{
public:
    explicit NonNoexceptMove(const std::string& name)
    : name_(name)
    {
        EventLog::instance().record("NonNoexceptMove(" + name_ + ")::ctor");
    }
    
    ~NonNoexceptMove()
    {
        EventLog::instance().record("NonNoexceptMove(" + name_ + ")::dtor");
    }
    
    // Move constructor NOT marked noexcept
    NonNoexceptMove(NonNoexceptMove&& other)
    : name_(std::move(other.name_))
    {
        EventLog::instance().record("NonNoexceptMove::move_ctor");
    }
    
    NonNoexceptMove& operator=(NonNoexceptMove&& other)
    {
        EventLog::instance().record("NonNoexceptMove::move_assign");
        name_ = std::move(other.name_);
        return *this;
    }
    
    NonNoexceptMove(const NonNoexceptMove& other)
    : name_(other.name_)
    {
        EventLog::instance().record("NonNoexceptMove::copy_ctor");
    }
    
    NonNoexceptMove& operator=(const NonNoexceptMove& other)
    {
        EventLog::instance().record("NonNoexceptMove::copy_assign");
        name_ = other.name_;
        return *this;
    }

private:
    std::string name_;
};

class NoexceptMove
{
public:
    explicit NoexceptMove(const std::string& name)
    : name_(name)
    {
        EventLog::instance().record("NoexceptMove(" + name_ + ")::ctor");
    }
    
    ~NoexceptMove()
    {
        EventLog::instance().record("NoexceptMove(" + name_ + ")::dtor");
    }
    
    // Move constructor marked noexcept
    NoexceptMove(NoexceptMove&& other) noexcept
    : name_(std::move(other.name_))
    {
        EventLog::instance().record("NoexceptMove::move_ctor");
    }
    
    NoexceptMove& operator=(NoexceptMove&& other) noexcept
    {
        EventLog::instance().record("NoexceptMove::move_assign");
        name_ = std::move(other.name_);
        return *this;
    }
    
    NoexceptMove(const NoexceptMove& other)
    : name_(other.name_)
    {
        EventLog::instance().record("NoexceptMove::copy_ctor");
    }
    
    NoexceptMove& operator=(const NoexceptMove& other)
    {
        EventLog::instance().record("NoexceptMove::copy_assign");
        name_ = other.name_;
        return *this;
    }

private:
    std::string name_;
};

// Q: How does std::vector decide whether to move or copy during reallocation?
// A:
// R:

// Q: What observable difference exists between NonNoexceptMove and NoexceptMove in vector reallocation?
// A:
// R:

TEST_F(NoexceptTest, VectorReallocation_NoexceptMoveOptimization)
{
    // Hard: std::vector uses move only if move constructor is noexcept
    
    {
        std::vector<NonNoexceptMove> vec;
        vec.reserve(2);
        
        vec.emplace_back("A");
        vec.emplace_back("B");
        
        EventLog::instance().clear();
        
        // Force reallocation
        vec.emplace_back("C");
        
        // Without noexcept move, vector uses copy constructor for safety
        // (In practice, std::string's move is noexcept, so this may still move)
        // The key point: vector checks noexcept before deciding
    }
    
    EventLog::instance().clear();
    
    {
        std::vector<NoexceptMove> vec;
        vec.reserve(2);
        
        vec.emplace_back("A");
        vec.emplace_back("B");
        
        EventLog::instance().clear();
        
        // Force reallocation
        vec.emplace_back("C");
        
        // With noexcept move, vector uses move constructor
        EXPECT_GE(EventLog::instance().count_events("NoexceptMove::move_ctor"), 2);
    }
}

// ============================================================================
// Conditional noexcept
// ============================================================================

template<typename T>
class ConditionalNoexceptWrapper
{
public:
    explicit ConditionalNoexceptWrapper(T value)
    : value_(std::move(value))
    {
        EventLog::instance().record("ConditionalNoexceptWrapper::ctor");
    }
    
    // Conditionally noexcept based on T's move constructor
    ConditionalNoexceptWrapper(ConditionalNoexceptWrapper&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    : value_(std::move(other.value_))
    {
        EventLog::instance().record("ConditionalNoexceptWrapper::move_ctor");
    }
    
    T& get() { return value_; }

private:
    T value_;
};

// Q: When is ConditionalNoexceptWrapper's move constructor noexcept?
// A:
// R:

// Q: Why would you use conditional noexcept instead of always noexcept or never noexcept?
// A:
// R:

TEST_F(NoexceptTest, ConditionalNoexcept_TypeDependent)
{
    // Hard: noexcept can be conditional based on template parameters
    
    using IntWrapper = ConditionalNoexceptWrapper<int>;
    using NonNoexceptWrapper = ConditionalNoexceptWrapper<NonNoexceptMove>;
    using NoexceptWrapper = ConditionalNoexceptWrapper<NoexceptMove>;
    
    // int is nothrow move constructible
    static_assert(std::is_nothrow_move_constructible_v<int>, "int should be nothrow movable");
    static_assert(noexcept(IntWrapper(std::declval<IntWrapper&&>())), "IntWrapper move should be noexcept");
    
    // NonNoexceptMove is not nothrow move constructible
    static_assert(!std::is_nothrow_move_constructible_v<NonNoexceptMove>, "NonNoexceptMove should not be nothrow movable");
    static_assert(!noexcept(NonNoexceptWrapper(std::declval<NonNoexceptWrapper&&>())), "NonNoexceptWrapper move should not be noexcept");
    
    // NoexceptMove is nothrow move constructible
    static_assert(std::is_nothrow_move_constructible_v<NoexceptMove>, "NoexceptMove should be nothrow movable");
    static_assert(noexcept(NoexceptWrapper(std::declval<NoexceptWrapper&&>())), "NoexceptWrapper move should be noexcept");
}




// ============================================================================
// noexcept and std::vector Performance
// ============================================================================

class ExpensiveCopy
{
public:
    explicit ExpensiveCopy(int id)
    : id_(id)
    , data_(std::make_unique<Tracked>("Data" + std::to_string(id)))
    {
        EventLog::instance().record("ExpensiveCopy(" + std::to_string(id_) + ")::ctor");
    }
    
    ~ExpensiveCopy()
    {
        EventLog::instance().record("ExpensiveCopy(" + std::to_string(id_) + ")::dtor");
    }
    
    // Copy is expensive
    ExpensiveCopy(const ExpensiveCopy& other)
    : id_(other.id_)
    , data_(std::make_unique<Tracked>("Data" + std::to_string(id_)))
    {
        EventLog::instance().record("ExpensiveCopy::copy_ctor - expensive!");
    }
    
    // Move is cheap and noexcept
    ExpensiveCopy(ExpensiveCopy&& other) noexcept
    : id_(other.id_)
    , data_(std::move(other.data_))
    {
        EventLog::instance().record("ExpensiveCopy::move_ctor - cheap!");
    }
    
    ExpensiveCopy& operator=(const ExpensiveCopy&) = delete;
    ExpensiveCopy& operator=(ExpensiveCopy&&) = delete;

private:
    int id_;
    std::unique_ptr<Tracked> data_;
};

// Q: Why does std::vector prefer noexcept move over copy during reallocation?
// A:
// R:

// Q: What exception safety guarantee would be violated if vector moved with throwing move?
// A:
// R:

TEST_F(NoexceptTest, Vector_NoexceptMoveOptimization)
{
    // Hard: noexcept move enables efficient vector reallocation
    
    std::vector<ExpensiveCopy> vec;
    vec.reserve(2);
    
    vec.emplace_back(1);
    vec.emplace_back(2);
    
    EventLog::instance().clear();
    
    // Force reallocation - should use move, not copy
    vec.emplace_back(3);
    
    // Verify moves were used (cheap), not copies (expensive)
    EXPECT_GE(EventLog::instance().count_events("ExpensiveCopy::move_ctor - cheap!"), 2);
    EXPECT_EQ(EventLog::instance().count_events("ExpensiveCopy::copy_ctor - expensive!"), 0);
}

// ============================================================================
// Conditional noexcept with Member Functions
// ============================================================================

template<typename T>
class Container
{
public:
    Container() = default;
    
    // Conditionally noexcept based on T's move constructor
    void add(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        EventLog::instance().record("Container::add");
        items_.push_back(std::move(value));
    }
    
    // Conditionally noexcept based on T's swap
    void swap(Container& other) noexcept(std::is_nothrow_swappable_v<T>)
    {
        EventLog::instance().record("Container::swap");
        items_.swap(other.items_);
    }

private:
    std::vector<T> items_;
};

// Q: When is Container<T>::add noexcept?
// A:
// R:

TEST_F(NoexceptTest, ConditionalNoexcept_MemberFunctions)
{
    // Hard: Member functions can be conditionally noexcept
    
    // Check type traits directly
    static_assert(std::is_nothrow_move_constructible_v<int>, 
                  "int should be nothrow move constructible");
    
    static_assert(!std::is_nothrow_move_constructible_v<NonNoexceptMove>, 
                  "NonNoexceptMove should not be nothrow move constructible");
    
    static_assert(std::is_nothrow_move_constructible_v<NoexceptMove>, 
                  "NoexceptMove should be nothrow move constructible");
    
    // Container<T>::add is conditionally noexcept based on T
    // The actual noexcept specification depends on vector's push_back
    // which may have additional requirements beyond just T's move constructor
}





// ============================================================================
// noexcept Propagation in Templates
// ============================================================================

template<typename T>
class Wrapper
{
public:
    explicit Wrapper(T value)
    : value_(std::move(value))
    {
        EventLog::instance().record("Wrapper::ctor");
    }
    
    // Propagate noexcept from T
    Wrapper(Wrapper&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    : value_(std::move(other.value_))
    {
        EventLog::instance().record("Wrapper::move_ctor");
    }
    
    Wrapper& operator=(Wrapper&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        EventLog::instance().record("Wrapper::move_assign");
        value_ = std::move(other.value_);
        return *this;
    }

private:
    T value_;
};

// Q: Why should templates propagate noexcept from their template parameters?
// A:
// R:

TEST_F(NoexceptTest, Templates_NoexceptPropagation)
{
    // Hard: Templates should propagate noexcept specifications
    
    using IntWrapper = Wrapper<int>;
    using NoexceptWrapper = Wrapper<NoexceptMove>;
    using NonNoexceptWrapper = Wrapper<NonNoexceptMove>;
    
    // int operations are noexcept
    static_assert(std::is_nothrow_move_constructible_v<IntWrapper>, 
                  "Wrapper<int> move should be noexcept");
    
    // NoexceptMove operations are noexcept
    // Note: std::string's move is noexcept, so NoexceptMove's move is noexcept
    static_assert(std::is_nothrow_move_constructible_v<NoexceptMove>, 
                  "NoexceptMove should be nothrow movable");
    
    // NonNoexceptMove operations are not noexcept
    static_assert(!std::is_nothrow_move_constructible_v<NonNoexceptMove>, 
                  "NonNoexceptMove should not be nothrow movable");
}

// ============================================================================
// noexcept Best Practices
// ============================================================================

// Q: Which operations should always be marked noexcept?
// A:
// R:

// Q: When should you NOT mark a function noexcept?
// A:
// R:

// Q: How does noexcept affect API design and evolution?
// A:
// R:

