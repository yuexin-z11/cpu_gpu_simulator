#include "socsim/dram.hpp"

#include <algorithm>

namespace socsim {

Dram::Dram(const DramConfig& cfg) : cfg_(cfg) {}

Cycle Dram::access(Cycle now, std::uint32_t bytes) {
    // wait for the bus if an earlier transfer is still using it
    const Cycle start = std::max(now, bus_free_at_);
    // cycles on the bus, rounded up (64 B at 16 B/cycle = 4, 65 B = 5)
    const Cycle transfer = (bytes + cfg_.bytes_per_cycle - 1) / cfg_.bytes_per_cycle;
    // the bus is busy only while data moves
    bus_free_at_ = start + transfer;

    ++stats_.requests;
    stats_.bytes += bytes;
    return start + cfg_.latency + transfer;
}

}  // namespace socsim
