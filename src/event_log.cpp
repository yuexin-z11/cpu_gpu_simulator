#include "socsim/event_log.hpp"

namespace socsim {

void EventLog::record(Cycle cycle, EventKind kind, std::uint32_t id, std::uint32_t value) {
    if (!enabled_) return;
    events_.push_back(Event{cycle, kind, id, value});
}

}  // namespace socsim
