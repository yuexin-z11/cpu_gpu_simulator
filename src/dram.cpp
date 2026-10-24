#include "socsim/dram.hpp"

namespace socsim {

Dram::Dram(const DramConfig& cfg) : cfg_(cfg) {}

Cycle Dram::access(Cycle now, std::uint32_t bytes) {
    // TODO: start = max(now, bus_free_at_); transfer takes
    // ceil(bytes / bytes_per_cycle) cycles; done = start + latency + transfer.
    // Update bus_free_at_ and stats_.
    (void)bytes;
    return now;
}

}  // namespace socsim
