// Test Suite: GoogleMock for Mocking
// Estimated Time: 4 hours
// Difficulty: Moderate

#include "instrumentation.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;

class GoogleMockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// TEST 1: Basic Mock Object and Expectations - Easy
// ============================================================================

class Database
{
public:
    virtual ~Database() = default;
    virtual bool connect(const std::string& connection_string) = 0;
    virtual int query(const std::string& sql) = 0;
    virtual void disconnect() = 0;
};

class MockDatabase : public Database
{
public:
    MOCK_METHOD(bool, connect, (const std::string& connection_string), (override));
    MOCK_METHOD(int, query, (const std::string& sql), (override));
    MOCK_METHOD(void, disconnect, (), (override));
};

class UserService
{
public:
    explicit UserService(std::shared_ptr<Database> db) : db_(std::move(db))
    {
        EventLog::instance().record("UserService::ctor");
    }

    bool initialize()
    {
        EventLog::instance().record("UserService::initialize()");
        return db_->connect("localhost:5432");
    }

    int get_user_count()
    {
        return db_->query("SELECT COUNT(*) FROM users");
    }

private:
    std::shared_ptr<Database> db_;
};

TEST_F(GoogleMockTest, BasicMockExpectations)
{
    auto mock_db = std::make_shared<MockDatabase>();

    EXPECT_CALL(*mock_db, connect(_)).WillOnce(Return(true));

    EXPECT_CALL(*mock_db, query(_)).WillOnce(Return(42));

    UserService service(mock_db);

    bool connected = service.initialize();
    int count = service.get_user_count();

    EXPECT_TRUE(connected);
    EXPECT_EQ(count, 42);

    // Q: EXPECT_CALL sets expectations on the mock. What happens if query() is never
    // Q: called before the test ends? What error does GoogleMock report?
    // A:
    // R:

    // Q: The mock_db is passed as shared_ptr. What is the use_count() after UserService
    // Q: is constructed? Who shares ownership?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("UserService::initialize()"), 1);
}

// ============================================================================
// TEST 2: Argument Matchers and Verification - Moderate
// ============================================================================

class EmailService
{
public:
    virtual ~EmailService() = default;
    virtual bool send(const std::string& to, const std::string& subject, const std::string& body) = 0;
};

class MockEmailService : public EmailService
{
public:
    MOCK_METHOD(bool, send, (const std::string& to, const std::string& subject, const std::string& body), (override));
};

class NotificationManager
{
public:
    explicit NotificationManager(std::shared_ptr<EmailService> email) : email_(std::move(email))
    {
    }

    void notify_user(const std::string& email_addr, const std::string& message)
    {
        email_->send(email_addr, "Notification", message);
        EventLog::instance().record("NotificationManager::notify_user()");
    }

private:
    std::shared_ptr<EmailService> email_;
};

TEST_F(GoogleMockTest, ArgumentMatchers)
{
    auto mock_email = std::make_shared<MockEmailService>();

    EXPECT_CALL(*mock_email,
                send(::testing::StartsWith("user@"), ::testing::Eq("Notification"), ::testing::HasSubstr("alert")))
        .WillOnce(Return(true));

    NotificationManager manager(mock_email);
    manager.notify_user("user@example.com", "Important alert message");

    // Q: The matcher StartsWith("user@") checks the first argument. What happens if
    // Q: notify_user() is called with "admin@example.com"? What error occurs?
    // A:
    // R:

    // Q: HasSubstr("alert") matches any string containing "alert". What flexibility does
    // Q: this provide compared to Eq("Important alert message")?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("notify_user()"), 1);
}

// ============================================================================
// TEST 3: Mock Call Sequences and Ordering - Moderate
// ============================================================================

class FileSystem
{
public:
    virtual ~FileSystem() = default;
    virtual bool open(const std::string& path) = 0;
    virtual int read(char* buffer, int size) = 0;
    virtual void close() = 0;
};

class MockFileSystem : public FileSystem
{
public:
    MOCK_METHOD(bool, open, (const std::string& path), (override));
    MOCK_METHOD(int, read, (char* buffer, int size), (override));
    MOCK_METHOD(void, close, (), (override));
};

class FileReader
{
public:
    explicit FileReader(std::shared_ptr<FileSystem> fs) : fs_(std::move(fs))
    {
    }

    std::string read_file(const std::string& path)
    {
        if (!fs_->open(path))
        {
            return "";
        }

        char buffer[100];
        int bytes = fs_->read(buffer, 100);
        fs_->close();

        EventLog::instance().record("FileReader::read_file() bytes=" + std::to_string(bytes));
        return std::string(buffer, bytes);
    }

private:
    std::shared_ptr<FileSystem> fs_;
};

TEST_F(GoogleMockTest, MockCallSequences)
{
    auto mock_fs = std::make_shared<MockFileSystem>();

    {
        ::testing::InSequence seq;

        EXPECT_CALL(*mock_fs, open("test.txt")).WillOnce(Return(true));

        EXPECT_CALL(*mock_fs, read(_, 100))
            .WillOnce(::testing::DoAll(::testing::Invoke([](char* buf, int size) {
                                           std::string data = "hello";
                                           std::copy(data.begin(), data.end(), buf);
                                       }),
                                       Return(5)));

        EXPECT_CALL(*mock_fs, close());
    }

    FileReader reader(mock_fs);
    std::string content = reader.read_file("test.txt");

    EXPECT_EQ(content, "hello");

    // Q: The InSequence object enforces call ordering. What happens if read() is called
    // Q: before open()? What error does GoogleMock report?
    // A:
    // R:

    // Q: DoAll combines multiple actions: Invoke (to fill buffer) and Return (to return bytes).
    // Q: What execution order do these actions follow?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("read_file()"), 1);
}

// ============================================================================
// TEST 4: Mock Side Effects with SaveArg - Hard
// ============================================================================

class MessageQueue
{
public:
    virtual ~MessageQueue() = default;
    virtual void publish(const std::string& topic, const std::string& message) = 0;
    virtual std::string subscribe(const std::string& topic) = 0;
};

class MockMessageQueue : public MessageQueue
{
public:
    MOCK_METHOD(void, publish, (const std::string& topic, const std::string& message), (override));
    MOCK_METHOD(std::string, subscribe, (const std::string& topic), (override));
};

TEST_F(GoogleMockTest, MockSideEffects)
{
    auto mock_queue = std::make_shared<MockMessageQueue>();
    std::string captured_message;

    EXPECT_CALL(*mock_queue, publish("events", _)).WillOnce(::testing::SaveArg<1>(&captured_message));

    mock_queue->publish("events", "test message");

    EXPECT_EQ(captured_message, "test message");

    // Q: SaveArg<1> captures the second argument (index 1). What happens to captured_message
    // Q: when publish() is called? When is the value assigned?
    // A:
    // R:

    // Q: If publish() were called multiple times with WillRepeatedly, what value would
    // Q: captured_message contain? The first call or the last call?
    // A:
    // R:
}

// ============================================================================
// TEST 5: TODO - Implement Mock with Custom Actions - Hard
// ============================================================================

// TODO: Create a mock Cache interface with get/set methods
// TODO: Use Invoke to implement custom behavior that simulates cache hits/misses
// TODO: Track cache statistics (hits, misses) using the custom action

class Cache
{
public:
    virtual ~Cache() = default;
    virtual std::optional<std::string> get(const std::string& key) = 0;
    virtual void set(const std::string& key, const std::string& value) = 0;
};

class MockCache : public Cache
{
public:
    MOCK_METHOD(std::optional<std::string>, get, (const std::string& key), (override));
    MOCK_METHOD(void, set, (const std::string& key, const std::string& value), (override));
};

TEST_F(GoogleMockTest, DISABLED_MockCustomActions)
{
    auto mock_cache = std::make_shared<MockCache>();

    // TODO: Set up EXPECT_CALL with Invoke to simulate cache behavior
    // TODO: First get() returns nullopt (miss), after set(), get() returns value (hit)

    EXPECT_FALSE(mock_cache->get("key1").has_value());
    mock_cache->set("key1", "value1");
    EXPECT_EQ(mock_cache->get("key1").value(), "value1");

    // Q: When using Invoke with a lambda that captures state, where is that state stored?
    // Q: What is its lifetime relative to the mock object?
    // A:
    // R:
}

// ============================================================================
// TEST 6: Mock Ownership and Lifetime - Moderate
// ============================================================================

class Logger
{
public:
    virtual ~Logger()
    {
        EventLog::instance().record("Logger::dtor");
    }

    virtual void log(const std::string& message) = 0;
};

class MockLogger : public Logger
{
public:
    MockLogger()
    {
        EventLog::instance().record("MockLogger::ctor");
    }

    ~MockLogger() override
    {
        EventLog::instance().record("MockLogger::dtor");
    }

    MOCK_METHOD(void, log, (const std::string& message), (override));
};

class Application
{
public:
    explicit Application(std::shared_ptr<Logger> logger) : logger_(std::move(logger))
    {
    }

    void run()
    {
        logger_->log("Application started");
    }

private:
    std::shared_ptr<Logger> logger_;
};

TEST_F(GoogleMockTest, MockOwnershipLifetime)
{
    auto mock_logger = std::make_shared<MockLogger>();

    EXPECT_CALL(*mock_logger, log(_)).Times(AtLeast(1));

    {
        Application app(mock_logger);
        app.run();
    }

    // Q: After the Application is destroyed, mock_logger is still alive. What is the
    // Q: use_count() of the shared_ptr at this point?
    // A:
    // R:

    // Q: When the test ends and mock_logger goes out of scope, GoogleMock verifies
    // Q: expectations in the mock's destructor. What happens if expectations aren't met?
    // A:
    // R:

    EXPECT_EQ(EventLog::instance().count_events("MockLogger::ctor"), 1);
}
