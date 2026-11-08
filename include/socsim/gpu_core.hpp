#pragma once
// one SIMT GPU core: round-robin warp scheduling, one instruction per warp
// per issue, coalesced memory access through the GPU L1

#include <cstdint>
#include <vector>

#include "socsim/config.hpp"
#include "socsim/memory_system.hpp"
#include "socsim/stats.hpp"
#include "socsim/types.hpp"
#include "socsim/warp.hpp"

namespace socsim {

class EventLog;

class GpuCore {
public:
    GpuCore(const GpuConfig& cfg, MemorySystem& mem, EventLog& log);

    // reads the launch packet from memory and starts the kernel
    void launch(Cycle now);

    // issues at most one warp instruction. writes the completion fence
    // when the last warp finishes
    void step(Cycle now);

    bool busy() const { return busy_; }
    const GpuStats& stats() const { return stats_; }

private:
    void issue(Warp& w, Cycle now);
    // round-robin; returns size() if none ready
    std::uint32_t select_warp(Cycle now);

    GpuConfig cfg_;
    MemorySystem& mem_;
    EventLog& log_;
    std::vector<Warp> warps_;
    std::uint32_t rr_next_ = 0;
    bool busy_             = false;
    Cycle launch_cycle_    = 0;
    GpuStats stats_;
};

}  // namespace socsim
