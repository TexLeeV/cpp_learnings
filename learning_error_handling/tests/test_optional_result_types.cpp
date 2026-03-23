// Test Suite: std::optional and Result Types
// Estimated Time: 3 hours
// Difficulty: Moderate


#include "instrumentation.h"
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <variant>
#include <sstream>

class OptionalResultTypesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EventLog::instance().clear();
    }
};

// ============================================================================
// std::optional Basics - Representing Optional Values
// ============================================================================

std::optional<int> parse_int(const std::string& str)
{
    EventLog::instance().record("parse_int: called with '" + str + "'");
    
    try
    {
        size_t pos;
        int value = std::stoi(str, &pos);
        if (pos != str.length())
        {
            EventLog::instance().record("parse_int: invalid format");
            return std::nullopt;
        }
        EventLog::instance().record("parse_int: success");
        return value;
    }
    catch (...)
    {
        EventLog::instance().record("parse_int: exception caught");
        return std::nullopt;
    }
}

// Q: What is the memory overhead of std::optional<int> compared to int?
// A:
// R:

// Q: How does std::optional represent "no value" internally?
// A:
// R:

TEST_F(OptionalResultTypesTest, Optional_BasicUsage)
{
    // Easy: std::optional represents values that may or may not exist
    
    auto result1 = parse_int("123");
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 123);
    EXPECT_EQ(result1.value(), 123);
    
    auto result2 = parse_int("abc");
    EXPECT_FALSE(result2.has_value());
    
    // Using value_or for default values
    EXPECT_EQ(result1.value_or(0), 123);
    EXPECT_EQ(result2.value_or(0), 0);
    
    EXPECT_EQ(EventLog::instance().count_events("parse_int: success"), 1);
    EXPECT_EQ(EventLog::instance().count_events("parse_int: exception caught"), 1);
}

// ============================================================================
// std::optional with Complex Types
// ============================================================================

std::optional<Tracked> create_tracked_conditionally(bool should_create)
{
    EventLog::instance().record("create_tracked: called");
    
    if (!should_create)
    {
        EventLog::instance().record("create_tracked: returning nullopt");
        return std::nullopt;
    }
    
    EventLog::instance().record("create_tracked: creating Tracked");
    // TODO: Return a Tracked object wrapped in optional
    return Tracked("Optional");
}

// Q: How many times is the Tracked constructor called when returning a value?
// A:
// R:

// Q: What happens to the Tracked object when std::nullopt is returned?
// A:
// R:

TEST_F(OptionalResultTypesTest, Optional_ComplexTypes)
{
    // Moderate: std::optional works with any type, including move-only types
    
    auto result1 = create_tracked_conditionally(true);
    EXPECT_TRUE(result1.has_value());
    
    // Verify Tracked was constructed (may involve moves)
    EXPECT_GE(EventLog::instance().count_events("::ctor"), 1);
    
    EventLog::instance().clear();
    
    auto result2 = create_tracked_conditionally(false);
    EXPECT_FALSE(result2.has_value());
    
    // Verify no Tracked object was created
    EXPECT_EQ(EventLog::instance().count_events("::ctor"), 0);
}

// ============================================================================
// Result Type Pattern - Error Information with Values
// ============================================================================

enum class ParseError
{
    Success,
    EmptyString,
    InvalidFormat,
    OutOfRange
};

template<typename T, typename E>
class Result
{
public:
    static Result Ok(T value)
    {
        EventLog::instance().record("Result::Ok");
        return Result(std::move(value), E{});
    }
    
    static Result Err(E error)
    {
        EventLog::instance().record("Result::Err");
        return Result(std::nullopt, error);
    }
    
    bool is_ok() const { return value_.has_value(); }
    bool is_err() const { return !value_.has_value(); }
    
    const T& value() const { return *value_; }
    T& value() { return *value_; }
    
    const E& error() const { return error_; }
    
    T value_or(T default_value) const
    {
        return value_.value_or(std::move(default_value));
    }

private:
    Result(std::optional<T> value, E error)
    : value_(std::move(value))
    , error_(error)
    {
    }
    
    std::optional<T> value_;
    E error_;
};

Result<int, ParseError> parse_int_with_error(const std::string& str)
{
    EventLog::instance().record("parse_int_with_error: called");
    
    if (str.empty())
    {
        EventLog::instance().record("parse_int_with_error: empty string");
        return Result<int, ParseError>::Err(ParseError::EmptyString);
    }
    
    try
    {
        size_t pos;
        int value = std::stoi(str, &pos);
        
        if (pos != str.length())
        {
            EventLog::instance().record("parse_int_with_error: invalid format");
            return Result<int, ParseError>::Err(ParseError::InvalidFormat);
        }
        
        EventLog::instance().record("parse_int_with_error: success");
        return Result<int, ParseError>::Ok(value);
    }
    catch (const std::out_of_range&)
    {
        EventLog::instance().record("parse_int_with_error: out of range");
        return Result<int, ParseError>::Err(ParseError::OutOfRange);
    }
    catch (...)
    {
        EventLog::instance().record("parse_int_with_error: invalid format (exception)");
        return Result<int, ParseError>::Err(ParseError::InvalidFormat);
    }
}

// Q: What advantage does Result<T, E> have over std::optional<T>?
// A:
// R:

// Q: How does Result<T, E> compare to throwing exceptions?
// A:
// R:

TEST_F(OptionalResultTypesTest, ResultType_ErrorInformation)
{
    // Moderate: Result types carry both success values and error information
    
    auto result1 = parse_int_with_error("123");
    EXPECT_TRUE(result1.is_ok());
    EXPECT_EQ(result1.value(), 123);
    
    auto result2 = parse_int_with_error("");
    EXPECT_TRUE(result2.is_err());
    EXPECT_EQ(result2.error(), ParseError::EmptyString);
    
    auto result3 = parse_int_with_error("12abc");
    EXPECT_TRUE(result3.is_err());
    EXPECT_EQ(result3.error(), ParseError::InvalidFormat);
    
    auto result4 = parse_int_with_error("999999999999999999999");
    EXPECT_TRUE(result4.is_err());
    EXPECT_EQ(result4.error(), ParseError::OutOfRange);
}

// ============================================================================
// Error Propagation with Result Types
// ============================================================================

Result<int, ParseError> parse_and_double(const std::string& str)
{
    EventLog::instance().record("parse_and_double: start");
    
    // TODO: Parse the string and return error if parsing fails
    auto result = parse_int_with_error(str);
    if (result.is_err())
    {
        EventLog::instance().record("parse_and_double: propagating error");
        return Result<int, ParseError>::Err(result.error());
    }
    
    EventLog::instance().record("parse_and_double: doubling value");
    return Result<int, ParseError>::Ok(result.value() * 2);
}

// Q: How does error propagation with Result types compare to exceptions?
// A:
// R:

// Q: What happens if you forget to check is_err() before calling value()?
// A:
// R:

TEST_F(OptionalResultTypesTest, ResultType_ErrorPropagation)
{
    // Moderate: Result types require explicit error propagation
    
    auto result1 = parse_and_double("10");
    EXPECT_TRUE(result1.is_ok());
    EXPECT_EQ(result1.value(), 20);
    
    auto result2 = parse_and_double("abc");
    EXPECT_TRUE(result2.is_err());
    EXPECT_EQ(result2.error(), ParseError::InvalidFormat);
    
    EXPECT_EQ(EventLog::instance().count_events("parse_and_double: propagating error"), 1);
    EXPECT_EQ(EventLog::instance().count_events("parse_and_double: doubling value"), 1);
}


// ============================================================================
// std::variant for Multiple Error Types
// ============================================================================

enum class NetworkError
{
    Timeout,
    ConnectionRefused,
    HostUnreachable
};

enum class ParseError2
{
    InvalidFormat,
    OutOfRange
};

using FetchResult = std::variant<std::string, NetworkError, ParseError2>;

FetchResult fetch_and_parse(const std::string& url)
{
    EventLog::instance().record("fetch_and_parse: start");
    
    // Simulate network fetch
    if (url == "timeout.example.com")
    {
        EventLog::instance().record("fetch_and_parse: network timeout");
        return NetworkError::Timeout;
    }
    
    if (url == "refused.example.com")
    {
        EventLog::instance().record("fetch_and_parse: connection refused");
        return NetworkError::ConnectionRefused;
    }
    
    // Simulate parsing
    if (url == "invalid.example.com")
    {
        EventLog::instance().record("fetch_and_parse: parse error");
        return ParseError2::InvalidFormat;
    }
    
    EventLog::instance().record("fetch_and_parse: success");
    return std::string("data from ") + url;
}

// Q: How does std::variant differ from std::optional for error handling?
// A:
// R:

// Q: What is the memory overhead of std::variant<std::string, NetworkError, ParseError2>?
// A:
// R:

TEST_F(OptionalResultTypesTest, Variant_MultipleErrorTypes)
{
    // Hard: std::variant can represent success or multiple error types
    
    auto result1 = fetch_and_parse("success.example.com");
    EXPECT_TRUE(std::holds_alternative<std::string>(result1));
    EXPECT_EQ(std::get<std::string>(result1), "data from success.example.com");
    
    auto result2 = fetch_and_parse("timeout.example.com");
    EXPECT_TRUE(std::holds_alternative<NetworkError>(result2));
    EXPECT_EQ(std::get<NetworkError>(result2), NetworkError::Timeout);
    
    auto result3 = fetch_and_parse("invalid.example.com");
    EXPECT_TRUE(std::holds_alternative<ParseError2>(result3));
    EXPECT_EQ(std::get<ParseError2>(result3), ParseError2::InvalidFormat);
}

// ============================================================================
// Visitor Pattern with std::variant
// ============================================================================

std::string handle_fetch_result(const FetchResult& result)
{
    EventLog::instance().record("handle_fetch_result: start");
    
    // TODO: Use std::visit to handle each variant alternative
    return std::visit([](auto&& arg) -> std::string
    {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, std::string>)
        {
            EventLog::instance().record("visitor: success case");
            return arg;
        }
        else if constexpr (std::is_same_v<T, NetworkError>)
        {
            EventLog::instance().record("visitor: network error case");
            return "Network error";
        }
        else if constexpr (std::is_same_v<T, ParseError2>)
        {
            EventLog::instance().record("visitor: parse error case");
            return "Parse error";
        }
    }, result);
}

// Q: How does std::visit ensure all variant alternatives are handled?
// A:
// R:

TEST_F(OptionalResultTypesTest, Variant_VisitorPattern)
{
    // Hard: std::visit provides type-safe exhaustive handling
    
    auto result1 = fetch_and_parse("success.example.com");
    std::string msg1 = handle_fetch_result(result1);
    EXPECT_EQ(msg1, "data from success.example.com");
    EXPECT_EQ(EventLog::instance().count_events("visitor: success case"), 1);
    
    EventLog::instance().clear();
    
    auto result2 = fetch_and_parse("timeout.example.com");
    std::string msg2 = handle_fetch_result(result2);
    EXPECT_EQ(msg2, "Network error");
    EXPECT_EQ(EventLog::instance().count_events("visitor: network error case"), 1);
}

// ============================================================================
// Result Type with RAII Resources
// ============================================================================

class DatabaseConnection
{
public:
    explicit DatabaseConnection(const std::string& name)
    : name_(name)
    {
        EventLog::instance().record("DatabaseConnection(" + name_ + ")::ctor");
    }
    
    ~DatabaseConnection()
    {
        EventLog::instance().record("DatabaseConnection(" + name_ + ")::dtor");
    }
    
    DatabaseConnection(DatabaseConnection&& other) noexcept
    : name_(std::move(other.name_))
    {
        EventLog::instance().record("DatabaseConnection::move_ctor");
    }
    
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(DatabaseConnection&&) = delete;

private:
    std::string name_;
};

enum class DbError
{
    ConnectionFailed,
    AuthenticationFailed,
    Timeout
};

Result<DatabaseConnection, DbError> connect_to_database(const std::string& host)
{
    EventLog::instance().record("connect_to_database: called");
    
    if (host == "timeout.example.com")
    {
        EventLog::instance().record("connect_to_database: timeout");
        return Result<DatabaseConnection, DbError>::Err(DbError::Timeout);
    }
    
    if (host == "auth.example.com")
    {
        EventLog::instance().record("connect_to_database: auth failed");
        return Result<DatabaseConnection, DbError>::Err(DbError::AuthenticationFailed);
    }
    
    EventLog::instance().record("connect_to_database: success");
    return Result<DatabaseConnection, DbError>::Ok(DatabaseConnection(host));
}

// Q: What happens to the DatabaseConnection object when an error is returned?
// A:
// R:

// Q: How does Result<T, E> ensure the DatabaseConnection is properly destroyed?
// A:
// R:

TEST_F(OptionalResultTypesTest, ResultType_WithRAII)
{
    // Hard: Result types work seamlessly with RAII resources
    
    {
        auto result1 = connect_to_database("success.example.com");
        EXPECT_TRUE(result1.is_ok());
        EXPECT_EQ(EventLog::instance().count_events("DatabaseConnection(success.example.com)::ctor"), 1);
        
        // result1 goes out of scope here
    }
    
    // Verify DatabaseConnection was destroyed
    EXPECT_EQ(EventLog::instance().count_events("DatabaseConnection(success.example.com)::dtor"), 1);
    
    EventLog::instance().clear();
    
    {
        auto result2 = connect_to_database("timeout.example.com");
        EXPECT_TRUE(result2.is_err());
        EXPECT_EQ(result2.error(), DbError::Timeout);
        
        // No DatabaseConnection created
        EXPECT_EQ(EventLog::instance().count_events("DatabaseConnection"), 0);
    }
}


// ============================================================================
// Performance: Optional vs Exceptions
// ============================================================================

// Q: What is the performance cost of std::optional compared to exceptions in the success path?
// A:
// R:

// Q: What is the performance cost of std::optional compared to exceptions in the error path?
// A:
// R:

// Q: For a parser that fails 50% of the time, which approach is more efficient?
// A:
// R:

