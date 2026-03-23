// Test Suite: Behavioral Patterns (Observer, Strategy, Visitor, Command)
// Estimated Time: 7 hours
// Difficulty: Moderate


#include <gtest/gtest.h>
#include "instrumentation.h"
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

class BehavioralPatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Observer Pattern with Weak Pointers - Moderate
// ============================================================================

class Observer
{
public:
    virtual ~Observer() = default;
    virtual void update(const std::string& message) = 0;
};

class ConcreteObserver : public Observer
{
public:
    explicit ConcreteObserver(const std::string& name) : name_(name)
    {
        EventLog::instance().record("Observer::ctor name=" + name);
    }

    void update(const std::string& message) override
    {
        EventLog::instance().record("Observer " + name_ + " received: " + message);
    }

private:
    std::string name_;
};

class Subject
{
public:
    void attach(std::shared_ptr<Observer> observer)
    {
        observers_.push_back(observer);
    }

    void notify(const std::string& message)
    {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                [](const std::weak_ptr<Observer>& wp) { return wp.expired(); }),
            observers_.end()
        );

        for (auto& weak_obs : observers_)
        {
            if (auto obs = weak_obs.lock())
            {
                obs->update(message);
            }
        }
    }

private:
    std::vector<std::weak_ptr<Observer>> observers_;
};

TEST_F(BehavioralPatternsTest, ObserverPatternWeakPtr)
{
    Subject subject;

    auto obs1 = std::make_shared<ConcreteObserver>("Observer1");
    auto obs2 = std::make_shared<ConcreteObserver>("Observer2");

    subject.attach(obs1);
    subject.attach(obs2);

    subject.notify("Event A");

    obs1.reset();

    subject.notify("Event B");

    // Q: After obs1.reset(), the Subject still holds a weak_ptr to Observer1. What happens
    // Q: when notify() calls weak_ptr::lock() on the expired weak_ptr?
    // A:
    // R:

    // Q: Why use weak_ptr instead of shared_ptr in the observers_ vector? What problem
    // Q: does this solve?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Observer1 received"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Observer2 received"), 2);
}

// ============================================================================
// TEST 2: Strategy Pattern with std::function - Easy
// ============================================================================

using SortStrategy = std::function<void(std::vector<int>&)>;

class Context
{
public:
    void set_strategy(SortStrategy strategy)
    {
        strategy_ = std::move(strategy);
    }

    void execute(std::vector<int>& data)
    {
        if (strategy_)
        {
            strategy_(data);
        }
    }

private:
    SortStrategy strategy_;
};

TEST_F(BehavioralPatternsTest, StrategyPattern)
{
    Context context;
    std::vector<int> data = {3, 1, 4, 1, 5};

    context.set_strategy([](std::vector<int>& d)
    {
        std::sort(d.begin(), d.end());
        EventLog::instance().record("Strategy: sort ascending");
    });

    context.execute(data);

    EXPECT_EQ(data, std::vector<int>({1, 1, 3, 4, 5}));

    // Q: The strategy is stored as std::function. What happens when set_strategy() is
    // Q: called with a new lambda? Is the old lambda destroyed?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("sort ascending"), 1);
}

// ============================================================================
// TEST 3: Command Pattern with Undo/Redo - Hard
// ============================================================================

class Command
{
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class Receiver
{
public:
    Receiver() : value_(0) {}

    void add(int amount)
    {
        value_ += amount;
        EventLog::instance().record("Receiver::add(" + std::to_string(amount) + ")");
    }

    void subtract(int amount)
    {
        value_ -= amount;
        EventLog::instance().record("Receiver::subtract(" + std::to_string(amount) + ")");
    }

    int value() const { return value_; }

private:
    int value_;
};

class AddCommand : public Command
{
public:
    AddCommand(Receiver& receiver, int amount)
    : receiver_(receiver)
    , amount_(amount)
    {
    }

    void execute() override { receiver_.add(amount_); }
    void undo() override { receiver_.subtract(amount_); }

private:
    Receiver& receiver_;
    int amount_;
};

class CommandInvoker
{
public:
    void execute_command(std::unique_ptr<Command> cmd)
    {
        cmd->execute();
        history_.push_back(std::move(cmd));
    }

    void undo()
    {
        if (!history_.empty())
        {
            history_.back()->undo();
            history_.pop_back();
            EventLog::instance().record("Undo executed");
        }
    }

private:
    std::vector<std::unique_ptr<Command>> history_;
};

TEST_F(BehavioralPatternsTest, CommandPatternUndoRedo)
{
    Receiver receiver;
    CommandInvoker invoker;

    invoker.execute_command(std::make_unique<AddCommand>(receiver, 10));
    EXPECT_EQ(receiver.value(), 10);

    invoker.execute_command(std::make_unique<AddCommand>(receiver, 20));
    EXPECT_EQ(receiver.value(), 30);

    invoker.undo();
    EXPECT_EQ(receiver.value(), 10);

    invoker.undo();
    EXPECT_EQ(receiver.value(), 0);

    // Q: The CommandInvoker stores unique_ptr<Command> in history_. What happens to
    // Q: the Command objects when undo() calls pop_back()?
    // A:
    // R:

    // Q: AddCommand stores a reference to Receiver. What would happen if Receiver were
    // Q: destroyed before the Command? How could you make this safer?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("Undo executed"), 2);
}

// ============================================================================
// TEST 5: Chain of Responsibility with unique_ptr - Moderate
// ============================================================================

class Handler
{
public:
    virtual ~Handler() = default;

    void set_next(std::unique_ptr<Handler> next)
    {
        next_ = std::move(next);
    }

    virtual std::string handle(int level)
    {
        if (next_)
        {
            return next_->handle(level);
        }
        return "Unhandled";
    }

protected:
    std::unique_ptr<Handler> next_;
};

class ConcreteHandlerA : public Handler
{
public:
    std::string handle(int level) override
    {
        if (level < 10)
        {
            EventLog::instance().record("HandlerA processed level=" + std::to_string(level));
            return "HandlerA processed";
        }
        return Handler::handle(level);
    }
};

class ConcreteHandlerB : public Handler
{
public:
    std::string handle(int level) override
    {
        if (level >= 10 && level < 20)
        {
            EventLog::instance().record("HandlerB processed level=" + std::to_string(level));
            return "HandlerB processed";
        }
        return Handler::handle(level);
    }
};

class ConcreteHandlerC : public Handler
{
public:
    std::string handle(int level) override
    {
        if (level >= 20)
        {
            EventLog::instance().record("HandlerC processed level=" + std::to_string(level));
            return "HandlerC processed";
        }
        return Handler::handle(level);
    }
};

TEST_F(BehavioralPatternsTest, ChainOfResponsibility)
{
    auto handler_c = std::make_unique<ConcreteHandlerC>();
    auto handler_b = std::make_unique<ConcreteHandlerB>();
    handler_b->set_next(std::move(handler_c));

    auto handler_a = std::make_unique<ConcreteHandlerA>();
    handler_a->set_next(std::move(handler_b));

    EXPECT_EQ(handler_a->handle(5), "HandlerA processed");
    EXPECT_EQ(handler_a->handle(15), "HandlerB processed");
    EXPECT_EQ(handler_a->handle(25), "HandlerC processed");

    // Q: The chain is built from the end: C, then B->C, then A->B->C. Why must you
    // Q: build it in this order when using unique_ptr ownership?
    // A:
    // R:

    // Q: When handler_a is destroyed, what destructor chain occurs? In what order are
    // Q: the handlers destroyed?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("HandlerA processed"), 1);
    EXPECT_EQ(EventLog::instance().count_events("HandlerB processed"), 1);
}

// ============================================================================
// TEST 6: TODO - Implement State Machine Pattern - Hard
// ============================================================================

// TODO: Implement a state machine for a TCP connection with states:
// TODO: 1. Closed -> Opening -> Open -> Closing -> Closed
// TODO: 2. Each state handles events differently (connect, send, close)
// TODO: 3. Use unique_ptr to manage current state
// TODO: 4. State transitions transfer ownership

class State
{
public:
    virtual ~State() = default;
    virtual std::unique_ptr<State> connect() = 0;
    virtual std::unique_ptr<State> send(const std::string& data) = 0;
    virtual std::unique_ptr<State> close() = 0;
    virtual std::string get_name() const = 0;
};

class ClosedState : public State
{
public:
    std::unique_ptr<State> connect() override
    {
        // TODO: Transition to OpenState
        return nullptr;
    }

    std::unique_ptr<State> send(const std::string& data) override
    {
        // TODO: Invalid operation, return this
        return nullptr;
    }

    std::unique_ptr<State> close() override
    {
        // TODO: Already closed, return this
        return nullptr;
    }

    std::string get_name() const override
    {
        return "Closed";
    }
};

class Connection
{
public:
    Connection() : state_(std::make_unique<ClosedState>()) {}

    void connect()
    {
        // TODO: state_ = state_->connect();
    }

    void send(const std::string& data)
    {
        // TODO: state_ = state_->send(data);
    }

    void close()
    {
        // TODO: state_ = state_->close();
    }

    std::string get_state() const
    {
        return state_->get_name();
    }

private:
    std::unique_ptr<State> state_;
};

TEST_F(BehavioralPatternsTest, DISABLED_StateMachinePattern)
{
    Connection conn;

    EXPECT_EQ(conn.get_state(), "Closed");

    conn.connect();
    EXPECT_EQ(conn.get_state(), "Open");

    conn.close();
    EXPECT_EQ(conn.get_state(), "Closed");

    // Q: Each state transition returns a new unique_ptr<State>. What happens to the
    // Q: old state object when state_ is reassigned?
    // A:
    // R:
}
