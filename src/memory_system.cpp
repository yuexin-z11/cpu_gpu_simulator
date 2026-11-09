#include "socsim/memory_system.hpp"

namespace socsim {

MemorySystem::MemorySystem(const ChipConfig& cfg)
    : cpu_l1_("cpu_l1", cfg.cpu_l1),
      gpu_l1_("gpu_l1", cfg.gpu_l1),
      l2_("l2", cfg.l2),
      dram_(cfg.dram) {}

Cycle MemorySystem::access(Cycle now, const MemRequest& req) {
    // control registers are not cached
    if (req.addr == mmio::kDoorbell || req.addr == mmio::kFence) return 1;

    // each side has its own L1
    Cache& l1 = (req.source == Requester::gpu) ? gpu_l1_ : cpu_l1_;

    // L1
    Cycle t = l1.config().hit_latency;
    if (l1.access(req.addr, req.is_write)) return t;

    // L1 miss -> shared L2
    t += l2_.config().hit_latency;
    if (l2_.access(req.addr, req.is_write)) return t;

    // L2 miss -> DRAM fetches one full L2 line, starting after the L1 and L2 lookups
    const Cycle done = dram_.access(now + t, l2_.config().line_bytes);
    // dram returns a finish time, so turn it back into a latency
    return done - now;
}

}  // namespace socsim
