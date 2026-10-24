#pragma once
// DRAM timing: fixed latency plus a bytes-per-cycle bandwidth limit.

#include <cstdint>

#include "socsim/config.hpp"
#include "socsim/types.hpp"

namespace socsim {

struct DramStats {
    std::uint64_t requests = 0;
    std::uint64_t bytes = 0;
};

class Dram {
public:
    explicit Dram(const DramConfig& cfg);

    // Returns the cycle at which a `bytes`-sized transfer issued at `now`
    // completes, accounting for transfers already occupying the bus.
    Cycle access(Cycle now, std::uint32_t bytes);

    const DramStats& stats() const { return stats_; }

private:
    DramConfig cfg_;
    Cycle bus_free_at_ = 0;
    DramStats stats_;
};

}  // namespace socsim
