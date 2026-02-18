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

int Tracked::next_id_ = 1;

Tracked::Tracked(const std::string& name)
: name_(name)
, id_(next_id_++)
{
    std::ostringstream oss;
    oss << "Tracked(" << name_ << ")::ctor [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
}

Tracked::Tracked(const Tracked& other)
: name_(other.name_)
, id_(next_id_++)
{
    std::ostringstream oss;
    oss << "Tracked(" << name_ << ")::copy_ctor from [id=" << other.id_ << "] to [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
}

Tracked::Tracked(Tracked&& other) noexcept
: name_(std::move(other.name_))
, id_(other.id_)
{
    std::ostringstream oss;
    oss << "Tracked(" << name_ << ")::move_ctor [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
    other.id_ = -1;
}

Tracked& Tracked::operator=(const Tracked& other)
{
    std::ostringstream oss;
    oss << "Tracked(" << name_ << ")::copy_assign from [id=" << other.id_ << "] to [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
    name_ = other.name_;
    return *this;
}

Tracked& Tracked::operator=(Tracked&& other) noexcept
{
    std::ostringstream oss;
    oss << "Tracked(" << name_ << ")::move_assign from [id=" << other.id_ << "] to [id=" << id_ << "]";
    EventLog::instance().record(oss.str());
    name_ = std::move(other.name_);
    int old_id = id_;
    id_ = other.id_;
    other.id_ = old_id;
    return *this;
}

Tracked::~Tracked()
{
    if (id_ != -1)
    {
        std::ostringstream oss;
        oss << "Tracked(" << name_ << ")::dtor [id=" << id_ << "]";
        EventLog::instance().record(oss.str());
    }
}

std::string Tracked::name() const
{
    return name_;
}

int Tracked::id() const
{
    return id_;
}
