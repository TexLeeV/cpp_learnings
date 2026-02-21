#include "instrumentation.h"
#include <sstream>

EventLog& EventLog::instance()
{
    static EventLog log;
    return log;
}

void EventLog::record(const std::string& event)
{
    events_.push_back(event);
}

void EventLog::clear()
{
    events_.clear();
}

std::vector<std::string> EventLog::events() const
{
    return events_;
}

std::string EventLog::dump() const
{
    std::ostringstream oss;
    for (size_t i = 0; i < events_.size(); ++i)
    {
        oss << "[" << i << "] " << events_[i] << "\n";
    }
    return oss.str();
}

size_t EventLog::count_events(const std::string& substring) const
{
    size_t count = 0;
    for (const auto& event : events_)
    {
        if (event.find(substring) != std::string::npos)
        {
            ++count;
        }
    }
    return count;
}

int MoveTracked::next_id_ = 1;

MoveTracked::MoveTracked(const std::string& name)
: name_(name)
, id_(next_id_++)
, moved_from_(false)
{
    std::ostringstream oss;
    oss << "MoveTracked(" << name_ << ")::ctor [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
}

MoveTracked::MoveTracked(const MoveTracked& other)
: name_(other.name_)
, id_(next_id_++)
, moved_from_(false)
{
    std::ostringstream oss;
    oss << "MoveTracked(" << name_ << ")::copy_ctor from [id=" << other.id_ << "] to [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
}

MoveTracked::MoveTracked(MoveTracked&& other) noexcept
: name_(std::move(other.name_))
, id_(other.id_)
, moved_from_(false)
{
    std::ostringstream oss;
    oss << "MoveTracked(" << name_ << ")::move_ctor from [id=" << other.id_ << "]";
    EventLog::instance().record(oss.str());
    other.moved_from_ = true;
}

MoveTracked& MoveTracked::operator=(const MoveTracked& other)
{
    std::ostringstream oss;
    oss << "MoveTracked(" << name_ << ")::copy_assign from [id=" << other.id_ << "] to [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
    name_ = other.name_;
    return *this;
}

MoveTracked& MoveTracked::operator=(MoveTracked&& other) noexcept
{
    std::ostringstream oss;
    oss << "MoveTracked(" << name_ << ")::move_assign from [id=" << other.id_ << "] to [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
    name_ = std::move(other.name_);
    other.moved_from_ = true;
    return *this;
}

MoveTracked::~MoveTracked()
{
    if (!moved_from_)
    {
        std::ostringstream oss;
        oss << "MoveTracked(" << name_ << ")::dtor [id=" << id_ << "]";
        EventLog::instance().record(oss.str());
    }
    else
    {
        std::ostringstream oss;
        oss << "MoveTracked(moved-from)::dtor [id=" << id_ << "]";
        EventLog::instance().record(oss.str());
    }
}

std::string MoveTracked::name() const
{
    return name_;
}

int MoveTracked::id() const
{
    return id_;
}

bool MoveTracked::is_moved_from() const
{
    return moved_from_;
}

int Resource::next_id_ = 1;

Resource::Resource(const std::string& name)
: name_(name)
, id_(next_id_++)
, valid_(true)
{
    std::ostringstream oss;
    oss << "Resource(" << name_ << ")::ctor [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
}

Resource::Resource(Resource&& other) noexcept
: name_(std::move(other.name_))
, id_(other.id_)
, valid_(true)
{
    std::ostringstream oss;
    oss << "Resource(" << name_ << ")::move_ctor from [id=" << other.id_ << "]";
    EventLog::instance().record(oss.str());
    other.valid_ = false;
}

Resource& Resource::operator=(Resource&& other) noexcept
{
    std::ostringstream oss;
    oss << "Resource(" << name_ << ")::move_assign from [id=" << other.id_ << "] to [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
    name_ = std::move(other.name_);
    other.valid_ = false;
    return *this;
}

Resource::~Resource()
{
    if (valid_)
    {
        std::ostringstream oss;
        oss << "Resource(" << name_ << ")::dtor [id=" << id_ << "]";
        EventLog::instance().record(oss.str());
    }
    else
    {
        std::ostringstream oss;
        oss << "Resource(moved-from)::dtor [id=" << id_ << "]";
        EventLog::instance().record(oss.str());
    }
}

std::string Resource::name() const
{
    return name_;
}

int Resource::id() const
{
    return id_;
}

bool Resource::is_valid() const
{
    return valid_;
}
