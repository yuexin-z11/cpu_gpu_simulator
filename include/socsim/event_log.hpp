#pragma once
// Optional record of what happened each cycle, used by the replay window.
// Recording is off by default and never changes simulation results.

#include <cstdint>
#include <vector>

#include "socsim/types.hpp"

namespace socsim {

enum class EventKind : std::uint8_t {
    CpuPhase,      // value: 0 = setup, 1 = launch, 2 = waiting on fence, 3 = check
    WarpIssue,     // id = warp, value = active lane mask
    CacheAccess,   // id = cache (0 cpu_l1, 1 gpu_l1, 2 l2), value = 1 hit / 0 miss
    DramAccess,    // value = bytes
    KernelDone,
};

struct Event {
    Cycle cycle = 0;
    EventKind kind = EventKind::CpuPhase;
    std::uint32_t id = 0;
    std::uint32_t value = 0;
};

class EventLog {
public:
    void set_enabled(bool on) { enabled_ = on; }
    bool enabled() const { return enabled_; }

    void record(Cycle cycle, EventKind kind, std::uint32_t id, std::uint32_t value);
    const std::vector<Event>& events() const { return events_; }
    void clear() { events_.clear(); }

private:
    bool enabled_ = false;
    std::vector<Event> events_;
};

}  // namespace socsim
