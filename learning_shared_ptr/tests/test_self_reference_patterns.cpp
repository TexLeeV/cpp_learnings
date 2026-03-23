#include "instrumentation.h"

#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

class SelfReferencePatternsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// Complete implementation - study this pattern
class AsyncOperation : public std::enable_shared_from_this<AsyncOperation>
{
public:
    explicit AsyncOperation(const std::string& name) : tracked_(name)
    {
    }

    void start(std::function<void(std::shared_ptr<AsyncOperation>)> callback)
    {
        callback(shared_from_this());
    }

    std::string name() const
    {
        return tracked_.name();
    }

private:
    Tracked tracked_;
};

TEST_F(SelfReferencePatternsTest, AsyncOperationWithSharedFromThis)
{
    long captured_use_count = 0;

    std::shared_ptr<AsyncOperation> op = std::make_shared<AsyncOperation>("Op1");
    long initial_count = op.use_count();

    // Q: Why is initial_count 1 before calling start()?
    // A:
    // R:

    op->start([&captured_use_count](std::shared_ptr<AsyncOperation> self) { captured_use_count = self.use_count(); });

    // Q: Why does captured_use_count equal 2 inside the callback?
    // A:
    // R:

    // Q: What would happen if start() passed `this` instead of shared_from_this()?
    // A:
    // R:

    EXPECT_EQ(initial_count, 1);
    EXPECT_EQ(captured_use_count, 2);
}

// Complete implementation - study this pattern
class CancellableTask : public std::enable_shared_from_this<CancellableTask>
{
public:
    explicit CancellableTask(const std::string& name) : tracked_(name), cancelled_(false)
    {
    }

    std::function<void()> create_callback()
    {
        std::weak_ptr<CancellableTask> weak_self = shared_from_this();

        return [weak_self]() {
            if (std::shared_ptr<CancellableTask> self = weak_self.lock())
            {
                if (!self->cancelled_)
                {
                    EventLog::instance().record("CancellableTask callback executed");
                }
            }
            else
            {
                EventLog::instance().record("CancellableTask callback skipped (expired)");
            }
        };
    }

    void cancel()
    {
        cancelled_ = true;
    }

private:
    Tracked tracked_;
    bool cancelled_;
};

TEST_F(SelfReferencePatternsTest, CancellableCallbackWithWeakFromThis)
{
    std::function<void()> callback;

    {
        std::shared_ptr<CancellableTask> task = std::make_shared<CancellableTask>("Task1");
        callback = task->create_callback();

        // Q: Why does create_callback() capture weak_ptr instead of shared_ptr?
        // A:
        // R:
    }

    EventLog::instance().clear();
    callback();

    // Q: What does weak_ptr::lock() return when called inside the callback?
    // A:
    // R:

    // Q: What would happen if the callback captured shared_ptr instead of weak_ptr?
    // A:
    // R:

    auto events = EventLog::instance().events();
    bool skipped = false;

    for (const auto& event : events)
    {
        if (event.find("skipped (expired)") != std::string::npos)
        {
            skipped = true;
        }
    }

    EXPECT_TRUE(skipped);
}

TEST_F(SelfReferencePatternsTest, SharedFromThisRequiresSharedPtr)
{
    std::shared_ptr<CancellableTask> task = std::make_shared<CancellableTask>("Task2");
    long use_count = task.use_count();

    // Q: What is the use_count before creating the callback?
    // A:
    // R:

    std::function<void()> callback = task->create_callback();

    // Q: Does creating the callback change the use_count? Why or why not?
    // A:
    // R:

    EventLog::instance().clear();
    callback();

    // Q: Why does the callback execute successfully (not skip)?
    // A:
    // R:

    auto events = EventLog::instance().events();
    bool executed = false;

    for (const auto& event : events)
    {
        if (event.find("callback executed") != std::string::npos)
        {
            executed = true;
        }
    }

    EXPECT_EQ(use_count, 1);
    EXPECT_TRUE(executed);
}

// Complete implementation - study this pattern
class Timer : public std::enable_shared_from_this<Timer>
{
public:
    explicit Timer(const std::string& name) : tracked_(name)
    {
    }

    void schedule(std::function<void(std::shared_ptr<Timer>)> callback)
    {
        callback_ = [self = shared_from_this(), callback]() { callback(self); };
    }

    void execute()
    {
        if (callback_)
        {
            callback_();
        }
    }

private:
    Tracked tracked_;
    std::function<void()> callback_;
};

TEST_F(SelfReferencePatternsTest, TimerWithSelfReference)
{
    long captured_use_count = 0;

    auto timer = std::make_shared<Timer>("Timer1");

    timer->schedule([&captured_use_count](std::shared_ptr<Timer> self) {
        captured_use_count = self.use_count();
    });

    timer->execute();

    EXPECT_EQ(captured_use_count, 3);
}

TEST_F(SelfReferencePatternsTest, MultipleCallbacksWithSharedFromThis)
{
    auto task = std::make_shared<CancellableTask>("Task1");

    auto cb1 = task->create_callback();
    auto cb2 = task->create_callback();
    auto cb3 = task->create_callback();

    long use_count = task.use_count();

    EXPECT_EQ(use_count, 1);
}

// Complete implementation - study this pattern
class Connection : public std::enable_shared_from_this<Connection>
{
public:
    explicit Connection(const std::string& name) : tracked_(name)
    {
    }

    void async_read(std::function<void(std::shared_ptr<Connection>)> handler)
    {
        handler(shared_from_this());
    }

    void async_write(std::function<void(std::shared_ptr<Connection>)> handler)
    {
        handler(shared_from_this());
    }

private:
    Tracked tracked_;
};

TEST_F(SelfReferencePatternsTest, ChainedAsyncOperations)
{
    long read_use_count = 0;
    long write_use_count = 0;

    auto conn = std::make_shared<Connection>("Conn1");

    conn->async_read([&read_use_count, &write_use_count](std::shared_ptr<Connection> self) {
        read_use_count = self.use_count();
        self->async_write([&write_use_count](std::shared_ptr<Connection> inner_self) {
            write_use_count = inner_self.use_count();
        });
    });

    EXPECT_EQ(read_use_count, 2);
    EXPECT_EQ(write_use_count, 3);
}

TEST_F(SelfReferencePatternsTest, WeakFromThisInCallback)
{
    auto task = std::make_shared<CancellableTask>("Task1");

    auto callback = task->create_callback();

    long use_count = task.use_count();

    EXPECT_EQ(use_count, 1);
}

// Complete implementation - study this pattern
class EventEmitter : public std::enable_shared_from_this<EventEmitter>
{
public:
    explicit EventEmitter(const std::string& name) : tracked_(name)
    {
    }

    void on_event(std::function<void(std::shared_ptr<EventEmitter>)> handler)
    {
        std::weak_ptr<EventEmitter> weak_self = shared_from_this();
        handlers_.push_back([weak_self, handler]() {
            if (auto self = weak_self.lock())
            {
                handler(self);
            }
        });
    }

    void emit()
    {
        for (auto& handler : handlers_)
        {
            handler();
        }
    }

private:
    Tracked tracked_;
    std::vector<std::function<void()>> handlers_;
};

TEST_F(SelfReferencePatternsTest, EventEmitterWithWeakFromThis)
{
    int handler_count = 0;

    auto emitter = std::make_shared<EventEmitter>("Emitter1");

    emitter->on_event([&handler_count](std::shared_ptr<EventEmitter> self) {
        handler_count++;
    });
    emitter->on_event([&handler_count](std::shared_ptr<EventEmitter> self) {
        handler_count++;
    });

    emitter->emit();

    EXPECT_EQ(handler_count, 2);
}

TEST_F(SelfReferencePatternsTest, SelfReferenceLifetimeExtension)
{
    std::function<void(std::shared_ptr<AsyncOperation>)> callback;

    {
        auto op = std::make_shared<AsyncOperation>("Op1");

        op->start([&callback](std::shared_ptr<AsyncOperation> self) {
            callback = [self](std::shared_ptr<AsyncOperation>) {};
        });
    }

    auto events = EventLog::instance().events();
    size_t dtor_count = 0;

    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }

    EXPECT_EQ(dtor_count, 0);
}

TEST_F(SelfReferencePatternsTest, WeakSelfNoLifetimeExtension)
{
    std::function<void()> callback;

    {
        auto task = std::make_shared<CancellableTask>("Task1");

        callback = task->create_callback();
    }

    auto events = EventLog::instance().events();
    size_t dtor_count = 0;

    for (const auto& event : events)
    {
        if (event.find("::dtor") != std::string::npos)
        {
            ++dtor_count;
        }
    }

    EXPECT_EQ(dtor_count, 1);
}
