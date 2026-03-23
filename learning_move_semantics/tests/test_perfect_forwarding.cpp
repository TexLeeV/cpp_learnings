#include "move_instrumentation.h"
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
    sink(std::forward<T>(param));
    
    // Q: What happens if you use sink(param) instead of sink(std::forward<T>(param))?
    // A: 
    // R: 
}

TEST_F(PerfectForwardingTest, BasicForwarding)
{
    MoveTracked obj("Forwarded");
    
    forward_to_sink(obj);
    forward_to_sink(std::move(obj));
    
    // Q: What does "perfect forwarding" preserve and how does T&& enable this?
    // A: 
    // R: 
    
    // Q: What would break if forward_to_sink used T& instead of T&&?
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
    MoveTracked obj("Imperfect");
    
    imperfect_forward(obj);
    
    // Q: What happens to the lvalue obj when imperfect_forward uses std::move(param)?
    // A: 
    // R: 
    
    // Q: What observable failure would occur if you used obj after this call?
    // A: 
    // R: 
    
    EXPECT_TRUE(true);
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique_impl(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

TEST_F(PerfectForwardingTest, VariadicForwarding)
{
    auto ptr = make_unique_impl<MoveTracked>("Variadic");
    
    // Q: How does Args&&... with std::forward<Args>(args)... preserve the value category of each argument independently?
    // A: 
    // R: 
    
    // Q: If you passed both lvalues and rvalues to make_unique_impl, what would happen to each?
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
    MoveTracked obj("Universal");
    
    auto rvalue_only = [](MoveTracked&& param)
    {
        EventLog::instance().record("rvalue_only called");
    };
    
    // Q: Why does MoveTracked&& in rvalue_only reject lvalues while T&& in templates accepts them?
    // A: 
    // R: 
    
    rvalue_only(std::move(obj));
    
    // Q: What is the difference between a deduced T&& and an explicit MoveTracked&&?
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
    MoveTracked obj("Deduction");
    
    type_deduction_forward(obj);
    type_deduction_forward(std::move(obj));
    
    // Q: When type_deduction_forward(obj) is called, what is T deduced as and what is T&& after reference collapsing?
    // A: 
    // R: 
    
    // Q: When type_deduction_forward(std::move(obj)) is called, what is T deduced as and what is T&& after reference collapsing?
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
    MoveTracked obj("Emplace");
    
    wrapper.emplace_back(obj);
    wrapper.emplace_back("Direct");
    
    // Q: What EventLog entries show the construction operations? How does emplace_back differ from push_back?
    // A: 
    // R: 
    
    // Q: When emplace_back("Direct") is called, how many MoveTracked objects are constructed?
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
