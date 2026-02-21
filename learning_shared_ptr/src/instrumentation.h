#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include <string>
#include <vector>
#include <sstream>
#include <memory>

class EventLog
{
public:
    static EventLog& instance();
    
    void record(const std::string& event);
    void clear();
    std::vector<std::string> events() const;
    std::string dump() const;
    size_t count_events(const std::string& substring) const;
    
private:
    EventLog() = default;
    std::vector<std::string> events_;
};

class Tracked
{
public:
    explicit Tracked(const std::string& name);
    Tracked(const Tracked& other);
    Tracked(Tracked&& other) noexcept;
    Tracked& operator=(const Tracked& other);
    Tracked& operator=(Tracked&& other) noexcept;
    ~Tracked();
    
    std::string name() const;
    int id() const;
    
private:
    std::string name_;
    int id_;
    static int next_id_;
};

template<typename T>
class LoggingDeleter
{
public:
    explicit LoggingDeleter(const std::string& deleter_name = "LoggingDeleter")
    : deleter_name_(deleter_name)
    {
    }
    
    void operator()(T* ptr) const
    {
        std::ostringstream oss;
        oss << deleter_name_ << "::operator() called on " << static_cast<void*>(ptr);
        EventLog::instance().record(oss.str());
        delete ptr;
    }
    
private:
    std::string deleter_name_;
};

template<typename T>
class LoggingArrayDeleter
{
public:
    explicit LoggingArrayDeleter(const std::string& deleter_name = "LoggingArrayDeleter")
    : deleter_name_(deleter_name)
    {
    }
    
    void operator()(T* ptr) const
    {
        std::ostringstream oss;
        oss << deleter_name_ << "::operator() called on array " << static_cast<void*>(ptr);
        EventLog::instance().record(oss.str());
        delete[] ptr;
    }
    
private:
    std::string deleter_name_;
};

#endif
