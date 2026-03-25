// Test Suite: Lambda Expressions (Captures, Mutable, Generic)
// Estimated Time: 3 hours
// Difficulty: Easy
// C++11/14

#include "instrumentation.h"

#include <algorithm>
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

class LambdasTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Scenario 1: Basic Lambda Syntax (Easy)
// ============================================================================

TEST_F(LambdasTest, BasicLambdaSyntax)
{
    // Q: What is a lambda expression in C++?
    // A:
    // R:

    auto simple_lambda = []() { return 42; };

    // Q: What is the type of simple_lambda?
    // A:
    // R:

    EXPECT_EQ(simple_lambda(), 42);

    // Lambda with parameters
    auto add = [](int a, int b) { return a + b; };

    EXPECT_EQ(add(3, 4), 7);

    // Q: Can two lambdas with identical signatures be assigned to the same variable?
    // A:
    // R:

    // auto another_add = [](int a, int b) { return a + b; };
    // add = another_add;  // This would fail to compile
}

// ============================================================================
// Scenario 2: Capture by Value (Easy)
// ============================================================================

TEST_F(LambdasTest, CaptureByValue)
{
    int x = 10;
    int y = 20;

    // Q: What does [x, y] mean in the capture list?
    // A:
    // R:

    auto lambda = [x, y]() { return x + y; };

    EXPECT_EQ(lambda(), 30);

    // Modify original variables
    x = 100;
    y = 200;

    // Q: What does lambda() return now?
    // A:
    // R:

    EXPECT_EQ(lambda(), 30);

    // Q: Why didn't the lambda see the modified values?
    // A:
    // R:
}

// ============================================================================
// Scenario 3: Capture by Reference (Moderate)
// ============================================================================

TEST_F(LambdasTest, CaptureByReference)
{
    int counter = 0;

    // Q: What does [&counter] mean?
    // A:
    // R:

    auto increment = [&counter]() { counter++; };

    increment();
    increment();
    increment();

    EXPECT_EQ(counter, 3);

    // Q: What lifetime hazard exists with capture by reference?
    // A:
    // R:

    std::function<void()> dangerous_lambda;

    {
        int local_var = 42;
        dangerous_lambda = [&local_var]() {
            EventLog::instance().record("Accessing local_var: " + std::to_string(local_var));
        };
    }

    // Q: What happens if we call dangerous_lambda() here?
    // A:
    // R:

    // DANGER: Uncommenting this would be undefined behavior
    // dangerous_lambda();
}

// ============================================================================
// Scenario 4: Capture All (Moderate)
// ============================================================================

TEST_F(LambdasTest, CaptureAll)
{
    int a = 1;
    int b = 2;
    int c = 3;

    // Q: What does [=] capture?
    // A:
    // R:

    auto lambda_value = [=]() { return a + b + c; };

    EXPECT_EQ(lambda_value(), 6);

    // Q: What does [&] capture?
    // A:
    // R:

    auto lambda_ref = [&]() {
        a++;
        b++;
        c++;
    };

    lambda_ref();

    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 3);
    EXPECT_EQ(c, 4);

    // Q: What does [=, &c] mean?
    // A:
    // R:

    int d = 10;
    auto mixed = [=, &c]() { c = a + b + d; };

    mixed();

    // a=2, b=3, d=10 -> c should be 15
    EXPECT_EQ(c, 15);
}

// ============================================================================
// Scenario 5: Mutable Lambdas (Moderate)
// ============================================================================

TEST_F(LambdasTest, MutableLambdas)
{
    int x = 5;

    auto immutable = [x]() {
        // x++;  // This would fail to compile
        return x;
    };

    // Q: Why can't we modify x inside a non-mutable lambda?
    // A:
    // R:

    auto mutable_lambda = [x]() mutable {
        x++;
        return x;
    };

    EXPECT_EQ(mutable_lambda(), 6);
    EXPECT_EQ(mutable_lambda(), 7);
    EXPECT_EQ(mutable_lambda(), 8);

    // Q: What is the value of the original x?
    // A:
    // R:

    EXPECT_EQ(x, 5);

    // Q: Where is the lambda's copy of x stored?
    // A:
    // R:
}

// ============================================================================
// Scenario 6: Lambdas with Tracked Objects (Moderate)
// ============================================================================

TEST_F(LambdasTest, LambdasWithTrackedObjects)
{
    std::shared_ptr<Tracked> ptr = std::make_shared<Tracked>("Lambda");

    EventLog::instance().clear();

    // Q: What happens to ptr's use_count when captured by value?
    // A:
    // R:

    auto lambda_by_value = [ptr]() { return ptr->name(); };

    EXPECT_EQ(ptr.use_count(), 2);

    // Q: What observable signal in EventLog shows the capture?
    // A:
    // R:

    // Capture by reference
    auto lambda_by_ref = [&ptr]() { return ptr->name(); };

    // Q: What is ptr's use_count with reference capture?
    // A:
    // R:

    EXPECT_EQ(ptr.use_count(), 2);
}

// ============================================================================
// Scenario 7: Generic Lambdas (C++14) (Moderate)
// ============================================================================

TEST_F(LambdasTest, GenericLambdas)
{
    // Q: What does auto in the parameter list enable?
    // A:
    // R:

    auto generic_add = [](auto a, auto b) { return a + b; };

    EXPECT_EQ(generic_add(1, 2), 3);
    EXPECT_EQ(generic_add(1.5, 2.5), 4.0);

    // Q: How many different instantiations of this lambda exist?
    // A:
    // R:

    std::vector<int> vec = {1, 2, 3, 4, 5};

    // TODO: Use a generic lambda with std::transform to double each element
    std::vector<int> doubled(vec.size());
    std::transform(vec.begin(), vec.end(), doubled.begin(), [](auto x) { return x * 2; });

    EXPECT_EQ(doubled[0], 2);
    EXPECT_EQ(doubled[4], 10);
}

// ============================================================================
// Scenario 8: Lambda as Function Parameter (Hard)
// ============================================================================

template <typename Func> void apply_to_tracked(Func&& func)
{
    Tracked obj("FuncParam");
    func(obj);
}

TEST_F(LambdasTest, LambdaAsFunctionParameter)
{
    int call_count = 0;

    // Q: What is the advantage of template parameter over std::function?
    // A:
    // R:

    apply_to_tracked([&call_count](Tracked& obj) {
        EventLog::instance().record("Lambda called with " + obj.name());
        call_count++;
    });

    EXPECT_EQ(call_count, 1);

    // Q: How many Tracked objects were constructed?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 1);

    // Using std::function
    std::function<void(Tracked&)> func = [&call_count](Tracked& obj) { call_count++; };

    apply_to_tracked(func);

    EXPECT_EQ(call_count, 2);

    // Q: What is the performance cost of std::function compared to template parameter?
    // A:
    // R:
}
