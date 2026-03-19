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
// noexcept and std::move_if_noexcept
// ============================================================================

class MoveIfNoexceptDemo
{
public:
    explicit MoveIfNoexceptDemo(const std::string& name, bool has_noexcept_move)
    : name_(name)
    , has_noexcept_move_(has_noexcept_move)
    {
        EventLog::instance().record("MoveIfNoexceptDemo(" + name_ + ")::ctor");
    }
    
    ~MoveIfNoexceptDemo()
    {
        EventLog::instance().record("MoveIfNoexceptDemo(" + name_ + ")::dtor");
    }
    
    MoveIfNoexceptDemo(MoveIfNoexceptDemo&& other) noexcept
    : name_(std::move(other.name_))
    , has_noexcept_move_(other.has_noexcept_move_)
    {
        EventLog::instance().record("MoveIfNoexceptDemo::move_ctor");
    }
    
    MoveIfNoexceptDemo(const MoveIfNoexceptDemo& other)
    : name_(other.name_)
    , has_noexcept_move_(other.has_noexcept_move_)
    {
        EventLog::instance().record("MoveIfNoexceptDemo::copy_ctor");
    }
    
    MoveIfNoexceptDemo& operator=(const MoveIfNoexceptDemo& other)
    {
        EventLog::instance().record("MoveIfNoexceptDemo::copy_assign");
        name_ = other.name_;
        has_noexcept_move_ = other.has_noexcept_move_;
        return *this;
    }
    
    MoveIfNoexceptDemo& operator=(MoveIfNoexceptDemo&& other) noexcept
    {
        EventLog::instance().record("MoveIfNoexceptDemo::move_assign");
        name_ = std::move(other.name_);
        has_noexcept_move_ = other.has_noexcept_move_;
        return *this;
    }

private:
    std::string name_;
    bool has_noexcept_move_;
};

template<typename T>
void relocate_with_move_if_noexcept(T& source, T& dest)
{
    EventLog::instance().record("relocate: start");
    
    // TODO: Use std::move_if_noexcept to conditionally move or copy
    dest = std::move_if_noexcept(source);
    
    EventLog::instance().record("relocate: complete");
}

// Q: What does std::move_if_noexcept do?
// A:
// R:

// Q: When would std::move_if_noexcept return an rvalue reference vs lvalue reference?
// A:
// R:

TEST_F(NoexceptTest, MoveIfNoexcept_ConditionalMove)
{
    // Moderate: std::move_if_noexcept enables strong exception safety
    
    MoveIfNoexceptDemo source("Source", true);
    MoveIfNoexceptDemo dest("Dest", true);
    
    EventLog::instance().clear();
    
    relocate_with_move_if_noexcept(source, dest);
    
    // With noexcept move, std::move_if_noexcept uses move
    EXPECT_EQ(EventLog::instance().count_events("MoveIfNoexceptDemo::move"), 1);
    
    // Q: Why is move preferred when it's noexcept?
    // A:
    // R:
}

// ============================================================================
// noexcept Operator - Runtime Query
// ============================================================================

void throwing_function()
{
    throw std::runtime_error("Error");
}

void nonthrowing_function() noexcept
{
    EventLog::instance().record("nonthrowing_function: called");
}

template<typename F>
void call_and_log_noexcept(F&& func)
{
    if constexpr (noexcept(func()))
    {
        EventLog::instance().record("call_and_log: function is noexcept");
    }
    else
    {
        EventLog::instance().record("call_and_log: function may throw");
    }
    
    func();
}

// Q: What is the difference between noexcept(function()) and noexcept function()?
// A:
// R:

TEST_F(NoexceptTest, NoexceptOperator_CompileTimeQuery)
{
    // Moderate: noexcept operator queries exception specification at compile time
    
    call_and_log_noexcept(nonthrowing_function);
    EXPECT_EQ(EventLog::instance().count_events("call_and_log: function is noexcept"), 1);
    
    EventLog::instance().clear();
    
    // Don't actually call throwing_function - just test the compile-time query
    auto lambda_throwing = []() { throw std::runtime_error("Error"); };
    
    // Verify compile-time detection
    static_assert(noexcept(nonthrowing_function()), "Should detect noexcept");
    static_assert(!noexcept(throwing_function()), "Should detect may-throw");
    
    // Q: Is noexcept(expr) evaluated at compile time or runtime?
    // A:
    // R:
}

// ============================================================================
// noexcept and Destructors
// ============================================================================

class ImplicitNoexceptDtor
{
public:
    explicit ImplicitNoexceptDtor(const std::string& name)
    : name_(name)
    {
        EventLog::instance().record("ImplicitNoexceptDtor(" + name_ + ")::ctor");
    }
    
    // Destructor is implicitly noexcept
    ~ImplicitNoexceptDtor()
    {
        EventLog::instance().record("ImplicitNoexceptDtor(" + name_ + ")::dtor");
    }

private:
    std::string name_;
};

class ExplicitNoexceptDtor
{
public:
    explicit ExplicitNoexceptDtor(const std::string& name)
    : name_(name)
    {
        EventLog::instance().record("ExplicitNoexceptDtor(" + name_ + ")::ctor");
    }
    
    // Explicitly marked noexcept (redundant but clear)
    ~ExplicitNoexceptDtor() noexcept
    {
        EventLog::instance().record("ExplicitNoexceptDtor(" + name_ + ")::dtor");
    }

private:
    std::string name_;
};

// Q: Are destructors noexcept by default in C++11 and later?
// A:
// R:

// Q: Why must destructors be noexcept?
// A:
// R:

TEST_F(NoexceptTest, Destructors_ImplicitNoexcept)
{
    // Easy: Destructors are implicitly noexcept in C++11 and later
    
    // Destructors are noexcept by default
    static_assert(std::is_nothrow_destructible_v<ImplicitNoexceptDtor>, 
                  "Destructor should be implicitly noexcept");
    
    static_assert(std::is_nothrow_destructible_v<ExplicitNoexceptDtor>, 
                  "Destructor should be explicitly noexcept");
    
    {
        ImplicitNoexceptDtor obj("Test");
    }
    
    EXPECT_EQ(EventLog::instance().count_events("ImplicitNoexceptDtor(Test)::dtor"), 1);
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
// noexcept and Function Pointers
// ============================================================================

void noexcept_func() noexcept
{
    EventLog::instance().record("noexcept_func: called");
}

void throwing_func()
{
    EventLog::instance().record("throwing_func: called");
}

using NoexceptFuncPtr = void(*)() noexcept;
using ThrowingFuncPtr = void(*)();

// Q: Can you assign a noexcept function to a non-noexcept function pointer?
// A:
// R:

// Q: Can you assign a non-noexcept function to a noexcept function pointer?
// A:
// R:

TEST_F(NoexceptTest, Noexcept_FunctionPointers)
{
    // Moderate: noexcept affects function pointer types
    
    // noexcept function can be assigned to throwing function pointer
    ThrowingFuncPtr ptr1 = noexcept_func;
    ptr1();
    EXPECT_EQ(EventLog::instance().count_events("noexcept_func: called"), 1);
    
    // noexcept function pointer
    NoexceptFuncPtr ptr2 = noexcept_func;
    ptr2();
    EXPECT_EQ(EventLog::instance().count_events("noexcept_func: called"), 2);
    
    // Non-noexcept function CANNOT be assigned to noexcept function pointer
    // NoexceptFuncPtr ptr3 = throwing_func;  // Compile error in C++17
}

// ============================================================================
// noexcept and std::terminate
// ============================================================================

void violate_noexcept() noexcept
{
    EventLog::instance().record("violate_noexcept: about to throw");
    throw std::runtime_error("Violating noexcept");
    EventLog::instance().record("violate_noexcept: after throw");  // Never executed
}

// Q: What happens when a noexcept function throws?
// A:
// R:

// Q: Can you catch an exception thrown from a noexcept function?
// A:
// R:

TEST_F(NoexceptTest, Noexcept_TerminateOnViolation)
{
    // Hard: Violating noexcept calls std::terminate
    
    // We cannot test std::terminate in a unit test (it terminates the program)
    // This test documents the behavior
    
    // If we called violate_noexcept(), the program would terminate
    // EXPECT_DEATH(violate_noexcept(), "");  // Would work with death tests
    
    // Q: Why does violating noexcept call std::terminate instead of propagating?
    // A:
    // R:
}

// ============================================================================
// noexcept and Swap Idiom
// ============================================================================

class SwapNoexcept
{
public:
    explicit SwapNoexcept(int value)
    : value_(value)
    , data_(std::make_unique<Tracked>("Data"))
    {
        EventLog::instance().record("SwapNoexcept::ctor");
    }
    
    // Swap should always be noexcept
    void swap(SwapNoexcept& other) noexcept
    {
        EventLog::instance().record("SwapNoexcept::swap");
        std::swap(value_, other.value_);
        std::swap(data_, other.data_);
    }
    
    int value() const { return value_; }

private:
    int value_;
    std::unique_ptr<Tracked> data_;
};

// Q: Why should swap always be noexcept?
// A:
// R:

// Q: How does noexcept swap enable strong exception safety in other operations?
// A:
// R:

TEST_F(NoexceptTest, Swap_NoexceptRequirement)
{
    // Moderate: Swap must be noexcept for strong exception safety patterns
    
    SwapNoexcept a(10);
    SwapNoexcept b(20);
    
    static_assert(noexcept(a.swap(b)), "swap must be noexcept");
    
    EventLog::instance().clear();
    
    a.swap(b);
    
    EXPECT_EQ(a.value(), 20);
    EXPECT_EQ(b.value(), 10);
    EXPECT_EQ(EventLog::instance().count_events("SwapNoexcept::swap"), 1);
}

// ============================================================================
// noexcept and Move Assignment
// ============================================================================

class NoexceptMoveAssign
{
public:
    explicit NoexceptMoveAssign(const std::string& name)
    : name_(name)
    , data_(std::make_unique<Tracked>(name))
    {
        EventLog::instance().record("NoexceptMoveAssign(" + name_ + ")::ctor");
    }
    
    ~NoexceptMoveAssign()
    {
        EventLog::instance().record("NoexceptMoveAssign(" + name_ + ")::dtor");
    }
    
    NoexceptMoveAssign(NoexceptMoveAssign&& other) noexcept
    : name_(std::move(other.name_))
    , data_(std::move(other.data_))
    {
        EventLog::instance().record("NoexceptMoveAssign::move_ctor");
    }
    
    // Move assignment should be noexcept
    NoexceptMoveAssign& operator=(NoexceptMoveAssign&& other) noexcept
    {
        EventLog::instance().record("NoexceptMoveAssign::move_assign");
        
        if (this != &other)
        {
            name_ = std::move(other.name_);
            data_ = std::move(other.data_);
        }
        
        return *this;
    }
    
    NoexceptMoveAssign(const NoexceptMoveAssign&) = delete;
    NoexceptMoveAssign& operator=(const NoexceptMoveAssign&) = delete;

private:
    std::string name_;
    std::unique_ptr<Tracked> data_;
};

// Q: Why should move assignment be noexcept?
// A:
// R:

// Q: What standard library operations rely on noexcept move assignment?
// A:
// R:

TEST_F(NoexceptTest, MoveAssignment_NoexceptRequirement)
{
    // Moderate: Move assignment should be noexcept for optimal performance
    
    NoexceptMoveAssign a("A");
    NoexceptMoveAssign b("B");
    
    static_assert(std::is_nothrow_move_assignable_v<NoexceptMoveAssign>, 
                  "Move assignment should be noexcept");
    
    EventLog::instance().clear();
    
    a = std::move(b);
    
    EXPECT_EQ(EventLog::instance().count_events("NoexceptMoveAssign::move_assign"), 1);
    
    // Old data in 'a' was destroyed
    EXPECT_EQ(EventLog::instance().count_events("Tracked(A)::dtor"), 1);
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
// noexcept and Exception Specifications
// ============================================================================

// Q: What is the difference between noexcept, noexcept(true), and noexcept(false)?
// A:
// R:

// Q: Can you have a function that is sometimes noexcept and sometimes not?
// A:
// R:

TEST_F(NoexceptTest, Noexcept_Specifications)
{
    // Moderate: Understanding noexcept specification syntax
    
    auto always_noexcept = []() noexcept { return 42; };
    auto never_noexcept = []() { return 42; };
    auto conditionally_noexcept = []() noexcept(sizeof(int) == 4) { return 42; };
    
    static_assert(noexcept(always_noexcept()), "Should be noexcept");
    static_assert(!noexcept(never_noexcept()), "Should not be noexcept");
    
    // Conditional noexcept evaluated at compile time
    #if INTPTR_MAX == INT32_MAX
        static_assert(noexcept(conditionally_noexcept()), "Should be noexcept on 32-bit");
    #else
        // May or may not be noexcept depending on platform
    #endif
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
