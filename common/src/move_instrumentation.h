#ifndef MOVE_INSTRUMENTATION_H
#define MOVE_INSTRUMENTATION_H

#include "instrumentation.h"
#include <utility>
#include <type_traits>

class MoveTracked
{
public:
    explicit MoveTracked(const std::string& name);
    MoveTracked(const MoveTracked& other);
    MoveTracked(MoveTracked&& other) noexcept;
    MoveTracked& operator=(const MoveTracked& other);
    MoveTracked& operator=(MoveTracked&& other) noexcept;
    ~MoveTracked();

    std::string name() const;
    int id() const;
    bool is_moved_from() const;

private:
    std::string name_;
    int id_;
    bool moved_from_;
    static int next_id_;
};

class Resource
{
public:
    explicit Resource(const std::string& name);
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource(Resource&& other) noexcept;
    Resource& operator=(Resource&& other) noexcept;
    ~Resource();

    std::string name() const;
    int id() const;
    bool is_valid() const;

private:
    std::string name_;
    int id_;
    bool valid_;
    static int next_id_;
};

template<typename T>
MoveTracked make_value(T&& arg)
{
    EventLog::instance().record("make_value() called with " +
        std::string(std::is_lvalue_reference<T>::value ? "lvalue" : "rvalue"));
    return MoveTracked(std::forward<T>(arg));
}

#endif
