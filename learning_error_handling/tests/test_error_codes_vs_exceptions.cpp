// Test Suite: Error Codes vs Exceptions (When to Use Which)
// Estimated Time: 2 hours
// Difficulty: Easy

#include "instrumentation.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <gtest/gtest.h>
#include <optional>
#include <system_error>

class ErrorCodesVsExceptionsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// Error Code Pattern - Traditional C-style and std::error_code
// ============================================================================

// Traditional C-style error code pattern
enum class FileError
{
    Success = 0,
    NotFound,
    PermissionDenied,
    AlreadyExists,
    IoError
};

struct FileResult
{
    std::string content;
    FileError error;
};

FileResult read_file_error_code(const std::string& path)
{
    EventLog::instance().record("read_file_error_code: called");

    std::ifstream file(path);
    if (!file.is_open())
    {
        EventLog::instance().record("read_file_error_code: file not found");
        return {"", FileError::NotFound};
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    EventLog::instance().record("read_file_error_code: success");
    return {content, FileError::Success};
}

// Q: What happens to the call stack when read_file_error_code returns an error?
// A:
// R:

// Q: How does the caller know whether to check the error field?
// A:
// R:

TEST_F(ErrorCodesVsExceptionsTest, ErrorCodePattern_BasicUsage)
{
    // Easy: Error codes require explicit checking

    FileResult result = read_file_error_code("/nonexistent/file.txt");

    EXPECT_EQ(result.error, FileError::NotFound);
    EXPECT_EQ(result.content, "");
    EXPECT_EQ(EventLog::instance().count_events("read_file_error_code: called"), 1);
    EXPECT_EQ(EventLog::instance().count_events("read_file_error_code: file not found"), 1);
}

// ============================================================================
// Exception Pattern - RAII-friendly error handling
// ============================================================================

std::string read_file_exception(const std::string& path)
{
    EventLog::instance().record("read_file_exception: called");

    std::ifstream file(path);
    if (!file.is_open())
    {
        EventLog::instance().record("read_file_exception: throwing exception");
        throw std::runtime_error("File not found: " + path);
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    EventLog::instance().record("read_file_exception: success");
    return content;
}

// Q: What happens to the call stack when read_file_exception throws?
// A:
// R:

// Q: What observable difference exists between error codes and exceptions for stack unwinding?
// A:
// R:

TEST_F(ErrorCodesVsExceptionsTest, ExceptionPattern_BasicUsage)
{
    // Easy: Exceptions automatically propagate up the stack

    try
    {
        std::string content = read_file_exception("/nonexistent/file.txt");
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_NE(std::string(e.what()).find("File not found"), std::string::npos);
    }

    EXPECT_EQ(EventLog::instance().count_events("read_file_exception: called"), 1);
    EXPECT_EQ(EventLog::instance().count_events("read_file_exception: throwing exception"), 1);
    EXPECT_EQ(EventLog::instance().count_events("read_file_exception: success"), 0);
}

// ============================================================================
// Comparing Error Propagation
// ============================================================================

// Error code propagation - must check at every level
FileResult process_with_error_codes(const std::string& path)
{
    EventLog::instance().record("process_with_error_codes: start");

    FileResult result = read_file_error_code(path);
    if (result.error != FileError::Success)
    {
        EventLog::instance().record("process_with_error_codes: propagating error");
        return result;
    }

    EventLog::instance().record("process_with_error_codes: processing content");
    // TODO: Add processing logic here
    return result;
}

// Exception propagation - automatic
std::string process_with_exceptions(const std::string& path)
{
    EventLog::instance().record("process_with_exceptions: start");

    // TODO: Call read_file_exception and let exceptions propagate
    std::string content = read_file_exception(path);

    EventLog::instance().record("process_with_exceptions: processing content");
    return content;
}

// Q: What happens if you forget to check result.error in process_with_error_codes?
// A:
// R:

// Q: What happens if you don't catch exceptions in process_with_exceptions?
// A:
// R:

TEST_F(ErrorCodesVsExceptionsTest, ErrorPropagation_ErrorCodes)
{
    // Moderate: Error codes require explicit propagation at each level

    FileResult result = process_with_error_codes("/nonexistent/file.txt");

    EXPECT_EQ(result.error, FileError::NotFound);
    EXPECT_EQ(EventLog::instance().count_events("process_with_error_codes: start"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_with_error_codes: propagating error"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_with_error_codes: processing content"), 0);
}

TEST_F(ErrorCodesVsExceptionsTest, ErrorPropagation_Exceptions)
{
    // Moderate: Exceptions propagate automatically through call stack

    try
    {
        std::string content = process_with_exceptions("/nonexistent/file.txt");
        FAIL() << "Should have propagated exception";
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_NE(std::string(e.what()).find("File not found"), std::string::npos);
    }

    EXPECT_EQ(EventLog::instance().count_events("process_with_exceptions: start"), 1);
    EXPECT_EQ(EventLog::instance().count_events("process_with_exceptions: processing content"), 0);
}

// ============================================================================
// Resource Cleanup with Error Codes vs Exceptions
// ============================================================================

class ResourceWithErrorCodes
{
public:
    explicit ResourceWithErrorCodes(const std::string& name) : name_(name), acquired_(false)
    {
        EventLog::instance().record("Resource(" + name_ + ")::ctor");
    }

    FileError acquire()
    {
        EventLog::instance().record("Resource(" + name_ + ")::acquire");
        acquired_ = true;
        return FileError::Success;
    }

    void release()
    {
        if (acquired_)
        {
            EventLog::instance().record("Resource(" + name_ + ")::release");
            acquired_ = false;
        }
    }

    ~ResourceWithErrorCodes()
    {
        EventLog::instance().record("Resource(" + name_ + ")::dtor");
        if (acquired_)
        {
            EventLog::instance().record("Resource(" + name_ + ")::dtor - leaked!");
        }
    }

private:
    std::string name_;
    bool acquired_;
};

FileResult operation_with_error_codes_manual_cleanup()
{
    EventLog::instance().record("operation: start");

    ResourceWithErrorCodes r1("R1");
    FileError err = r1.acquire();
    if (err != FileError::Success)
    {
        return {"", err};
    }

    ResourceWithErrorCodes r2("R2");
    err = r2.acquire();
    if (err != FileError::Success)
    {
        r1.release(); // Must manually clean up R1
        return {"", err};
    }

    // Simulate error after acquiring both resources
    EventLog::instance().record("operation: simulating error");
    r2.release();
    r1.release();
    return {"", FileError::IoError};
}

// Q: What happens if you forget to call r1.release() in the error path?
// A:
// R:

// Q: How does the cleanup complexity scale with the number of resources?
// A:
// R:

class ResourceWithExceptions
{
public:
    explicit ResourceWithExceptions(const std::string& name) : name_(name)
    {
        EventLog::instance().record("Resource(" + name_ + ")::ctor");
        EventLog::instance().record("Resource(" + name_ + ")::acquire");
    }

    ~ResourceWithExceptions()
    {
        EventLog::instance().record("Resource(" + name_ + ")::release");
        EventLog::instance().record("Resource(" + name_ + ")::dtor");
    }

private:
    std::string name_;
};

std::string operation_with_exceptions_automatic_cleanup()
{
    EventLog::instance().record("operation: start");

    ResourceWithExceptions r1("R1");
    ResourceWithExceptions r2("R2");

    // Simulate error after acquiring both resources
    EventLog::instance().record("operation: throwing exception");
    throw std::runtime_error("Simulated error");
}

// Q: What happens to r1 and r2 when the exception is thrown?
// A:
// R:

TEST_F(ErrorCodesVsExceptionsTest, ResourceCleanup_ErrorCodes)
{
    // Moderate: Error codes require manual cleanup at each error point

    FileResult result = operation_with_error_codes_manual_cleanup();

    EXPECT_EQ(result.error, FileError::IoError);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R1)::acquire"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R2)::acquire"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R1)::release"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R2)::release"), 1);

    // Verify no leaks
    EXPECT_EQ(EventLog::instance().count_events("leaked!"), 0);
}

TEST_F(ErrorCodesVsExceptionsTest, ResourceCleanup_Exceptions)
{
    // Moderate: Exceptions trigger automatic RAII cleanup via stack unwinding

    try
    {
        std::string result = operation_with_exceptions_automatic_cleanup();
        FAIL() << "Should have thrown exception";
    }
    catch (const std::runtime_error&)
    {
        // Exception caught
    }

    // Verify automatic cleanup happened in reverse order (R2 then R1)
    EXPECT_EQ(EventLog::instance().count_events("Resource(R1)::acquire"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R2)::acquire"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R1)::release"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R2)::release"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R1)::dtor"), 1);
    EXPECT_EQ(EventLog::instance().count_events("Resource(R2)::dtor"), 1);
}

// ============================================================================
// When to Use Which Pattern
// ============================================================================

// Q: Why are exceptions better suited for RAII-heavy code?
// A:
// R:

// Q: Why are error codes better for parsing and validation?
// A:
// R:

// Q: When would you choose error codes over exceptions in performance-critical code?
// A:
// R:

// ============================================================================
// std::error_code - Modern C++ Error Code Pattern
// ============================================================================

enum class NetworkError
{
    Success = 0,
    Timeout,
    ConnectionRefused,
    HostUnreachable
};

// Register custom error category
class NetworkErrorCategory : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "network";
    }

    std::string message(int ev) const override
    {
        switch (static_cast<NetworkError>(ev))
        {
        case NetworkError::Success:
            return "Success";
        case NetworkError::Timeout:
            return "Connection timeout";
        case NetworkError::ConnectionRefused:
            return "Connection refused";
        case NetworkError::HostUnreachable:
            return "Host unreachable";
        default:
            return "Unknown error";
        }
    }
};

const NetworkErrorCategory& network_category()
{
    static NetworkErrorCategory instance;
    return instance;
}

std::error_code make_error_code(NetworkError e)
{
    return {static_cast<int>(e), network_category()};
}

namespace std
{
template <> struct is_error_code_enum<NetworkError> : true_type
{
};
} // namespace std

struct ConnectionResult
{
    std::string data;
    std::error_code error;
};

ConnectionResult connect_to_server(const std::string& host)
{
    EventLog::instance().record("connect_to_server: called");

    if (host == "timeout.example.com")
    {
        EventLog::instance().record("connect_to_server: timeout");
        return {"", make_error_code(NetworkError::Timeout)};
    }

    if (host == "refused.example.com")
    {
        EventLog::instance().record("connect_to_server: refused");
        return {"", make_error_code(NetworkError::ConnectionRefused)};
    }

    EventLog::instance().record("connect_to_server: success");
    return {"data", make_error_code(NetworkError::Success)};
}

// Q: What advantage does std::error_code provide over raw enum error codes?
// A:
// R:

TEST_F(ErrorCodesVsExceptionsTest, StdErrorCode_TypeSafety)
{
    // Moderate: std::error_code provides type-safe, composable error handling

    ConnectionResult result1 = connect_to_server("timeout.example.com");
    EXPECT_TRUE(result1.error); // Implicit bool conversion
    EXPECT_EQ(result1.error, NetworkError::Timeout);
    EXPECT_EQ(result1.error.message(), "Connection timeout");

    ConnectionResult result2 = connect_to_server("success.example.com");
    EXPECT_FALSE(result2.error); // Success is falsy
    EXPECT_EQ(result2.data, "data");

    // Q: How does std::error_code's bool conversion work?
    // A:
    // R:
}

// ============================================================================
// Mixing Error Codes and Exceptions
// ============================================================================

std::string fetch_with_retry(const std::string& host, int max_retries)
{
    EventLog::instance().record("fetch_with_retry: start");

    for (int attempt = 0; attempt < max_retries; ++attempt)
    {
        ConnectionResult result = connect_to_server(host);

        if (!result.error)
        {
            EventLog::instance().record("fetch_with_retry: success on attempt " + std::to_string(attempt));
            return result.data;
        }

        // Transient errors - retry
        if (result.error == NetworkError::Timeout)
        {
            EventLog::instance().record("fetch_with_retry: retry on timeout");
            continue;
        }

        // Permanent errors - throw exception
        EventLog::instance().record("fetch_with_retry: permanent error, throwing");
        throw std::system_error(result.error, "Permanent network error");
    }

    EventLog::instance().record("fetch_with_retry: max retries exceeded, throwing");
    throw std::runtime_error("Max retries exceeded");
}

// Q: Why use error codes for transient errors but exceptions for permanent errors?
// A:
// R:

TEST_F(ErrorCodesVsExceptionsTest, MixedPattern_RetryLogic)
{
    // Hard: Combining error codes (expected/transient) with exceptions (unexpected/permanent)

    // Transient error that succeeds on retry
    // TODO: Modify connect_to_server to succeed after first timeout
    // For now, test permanent error

    try
    {
        std::string data = fetch_with_retry("refused.example.com", 3);
        FAIL() << "Should have thrown on permanent error";
    }
    catch (const std::system_error& e)
    {
        EXPECT_EQ(e.code(), NetworkError::ConnectionRefused);
    }

    EXPECT_EQ(EventLog::instance().count_events("fetch_with_retry: start"), 1);
    EXPECT_EQ(EventLog::instance().count_events("fetch_with_retry: permanent error, throwing"), 1);

    // Q: What observable signal confirms that no retry happened for permanent errors?
    // A:
    // R:
}
