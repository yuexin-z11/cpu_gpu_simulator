#include "socsim/memory_system.hpp"

namespace socsim {

MemorySystem::MemorySystem(const ChipConfig& cfg)
    : cpu_l1_("cpu_l1", cfg.cpu_l1),
      gpu_l1_("gpu_l1", cfg.gpu_l1),
      l2_("l2", cfg.l2),
      dram_(cfg.dram) {}

Cycle MemorySystem::access(Cycle now, const MemRequest& req) {
    // TODO: pick the L1 by req.source; on an L1 miss go to L2; on an L2
    // miss go to DRAM for one line. Return the total latency in cycles.
    // MMIO addresses (doorbell, fence) should bypass the caches.
    (void)now;
    (void)req;
    return 1;
}

}  // namespace socsim
