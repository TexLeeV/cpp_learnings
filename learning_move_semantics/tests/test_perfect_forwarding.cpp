#include "instrumentation.h"
#include <gtest/gtest.h>
#include <utility>
#include <memory>

class PerfectForwardingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

template<typename T>
void sink(T&& param)
{
    EventLog::instance().record("sink called");
}

template<typename T>
void forward_to_sink(T&& param)
{
    // TODO: Forward param to sink using std::forward
    // YOUR CODE HERE
    
    // Q: What happens if you don't use std::forward here?
    // A: 
    // R: 
}

TEST_F(PerfectForwardingTest, BasicForwarding)
{
    // TODO: Create obj with name "Forwarded"
    // YOUR CODE HERE
    
    // TODO: Call forward_to_sink with obj (lvalue)
    // YOUR CODE HERE
    
    // TODO: Call forward_to_sink with std::move(obj)
    // YOUR CODE HERE
    
    // Q: What is "perfect forwarding"?
    // A: 
    // R: 
    
    // Q: Why do we need both std::forward and T&& to achieve perfect forwarding?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t sink_calls = EventLog::instance().count_events("sink called");
    
    EXPECT_EQ(sink_calls, 2);
}

template<typename T>
void imperfect_forward(T&& param)
{
    // Q: What is wrong with using std::move here instead of std::forward?
    // A: 
    // R: 
    
    sink(std::move(param));
}

TEST_F(PerfectForwardingTest, ImperfectForwardingProblem)
{
    // TODO: Create obj with name "Imperfect"
    // YOUR CODE HERE
    
    // TODO: Call imperfect_forward with obj (lvalue)
    // YOUR CODE HERE
    
    // Q: What happened to the lvalue when using std::move in forwarding?
    // A: 
    // R: 
    
    // Q: Why is this dangerous?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique_impl(Args&&... args)
{
    // TODO: Create unique_ptr by forwarding args to T's constructor
    // return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    
    // Q: Why do we use Args&&... (variadic universal reference)?
    // A: 
    // R: 
    
    // Q: What happens to each argument in args when forwarded?
    // A: 
    // R: 
    
    return nullptr;
}

TEST_F(PerfectForwardingTest, VariadicForwarding)
{
    // TODO: Call make_unique_impl with multiple arguments
    // YOUR CODE HERE
    
    // Q: How does std::forward preserve the value category of multiple arguments?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

template<typename T>
void universal_ref(T&& param)
{
    // Q: Is T&& always an rvalue reference?
    // A: 
    // R: 
    
    // Q: What is a "universal reference" (forwarding reference)?
    // A: 
    // R: 
    
    // Q: When is T&& a universal reference vs rvalue reference?
    // A: 
    // R: 
}

TEST_F(PerfectForwardingTest, UniversalReferenceVsRvalueReference)
{
    // TODO: Create obj with name "Universal"
    // YOUR CODE HERE
    
    // Explicit type parameter - T&& is rvalue reference
    auto rvalue_only = [](MoveTracked&& param)
    {
        EventLog::instance().record("rvalue_only called");
    };
    
    // Q: Can you call rvalue_only with obj (lvalue)?
    // A: 
    // R: 
    
    // TODO: Try calling rvalue_only with std::move(obj)
    // YOUR CODE HERE
    
    // Q: Why can't rvalue_only accept lvalues?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    EXPECT_GE(EventLog::instance().count_events("rvalue_only called"), 1);
}

template<typename T>
void type_deduction_forward(T&& param)
{
    // Q: If you call this with an lvalue, what is T deduced as?
    // A: 
    // R: 
    
    // Q: If you call this with an rvalue, what is T deduced as?
    // A: 
    // R: 
    
    // Q: After reference collapsing, what is T&& in each case?
    // A: 
    // R: 
}

TEST_F(PerfectForwardingTest, TypeDeductionRules)
{
    // TODO: Create obj with name "Deduction"
    // YOUR CODE HERE
    
    // TODO: Call type_deduction_forward with obj (lvalue)
    // YOUR CODE HERE
    
    // TODO: Call type_deduction_forward with std::move(obj)
    // YOUR CODE HERE
    
    // Q: What are the type deduction rules for T&&?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

template<typename T>
class Wrapper
{
public:
    // Q: Is T&& in a class template a universal reference?
    // A: 
    // R: 
    
    void set(T&& value)
    {
        // Q: Is T&& here a universal reference?
        // A: 
        // R: 
    }
    
    template<typename U>
    void forward_set(U&& value)
    {
        // Q: Is U&& here a universal reference?
        // A: 
        // R: 
    }
};

TEST_F(PerfectForwardingTest, UniversalReferenceContext)
{
    // Q: Why is type deduction required for universal references?
    // A: 
    // R: 
    
    // Q: In what contexts do you see T&&?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

template<typename... Args>
void forward_multiple(Args&&... args)
{
    // TODO: Forward all args to a container
    // YOUR CODE HERE
    
    // Q: How does parameter pack expansion work with std::forward?
    // A: 
    // R: 
    
    // Q: Can you forward different types with different value categories?
    // A: 
    // R: 
}

TEST_F(PerfectForwardingTest, ParameterPackForwarding)
{
    // TODO: Create multiple objects
    // YOUR CODE HERE
    
    // TODO: Call forward_multiple with mixed lvalues and rvalues
    // YOUR CODE HERE
    
    // Q: How does perfect forwarding handle heterogeneous arguments?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

template<typename T>
T&& forward_reference_collapsing(T&& param)
{
    // Q: What happens when T is deduced as MoveTracked&?
    // A: 
    // R: 
    
    // Q: What is the result of MoveTracked& && after reference collapsing?
    // A: 
    // R: 
    
    return std::forward<T>(param);
}

TEST_F(PerfectForwardingTest, ReferenceCollapsing)
{
    // Q: What are the four reference collapsing rules?
    // A: 
    // R: 
    
    // Q: Why do rvalue references collapse to lvalue references in some cases?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

struct EmplaceWrapper
{
    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        items_.push_back(MoveTracked(std::forward<Args>(args)...));
    }
    
    std::vector<MoveTracked> items_;
};

TEST_F(PerfectForwardingTest, EmplaceBackPattern)
{
    EmplaceWrapper wrapper;
    
    // TODO: Create obj with name "Emplace"
    // YOUR CODE HERE
    
    // TODO: Call emplace_back with obj (lvalue)
    // YOUR CODE HERE
    
    // TODO: Call emplace_back with "Direct" (construct in-place)
    // YOUR CODE HERE
    
    // Q: What is the advantage of emplace_back over push_back?
    // A: 
    // R: 
    
    // Q: How does perfect forwarding enable in-place construction?
    // A: 
    // R: 
    
    auto events = EventLog::instance().events();
    size_t ctor_count = EventLog::instance().count_events("::ctor [id=");
    
    EXPECT_GE(ctor_count, 2);
}

template<typename T>
void deduce_and_forward(T&& param)
{
    // Q: After T is deduced, how does std::forward<T> know the original value category?
    // A: 
    // R: 
    
    // Q: What information is encoded in the template parameter T?
    // A: 
    // R: 
}

TEST_F(PerfectForwardingTest, ForwardingMechanism)
{
    // Q: How does std::forward differ from static_cast<T&&>?
    // A: 
    // R: 
    
    // Q: Why can't we just use std::move everywhere?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}
