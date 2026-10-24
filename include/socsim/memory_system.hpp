#pragma once
// Routes requests through L1 -> shared L2 -> DRAM and returns latency.

#include "socsim/cache.hpp"
#include "socsim/config.hpp"
#include "socsim/dram.hpp"
#include "socsim/flat_memory.hpp"
#include "socsim/types.hpp"

namespace socsim {

class MemorySystem {
public:
    explicit MemorySystem(const ChipConfig& cfg);

    // Timing only: returns the number of cycles the request takes when
    // issued at `now`. The caller reads/writes data through data().
    Cycle access(Cycle now, const MemRequest& req);

    FlatMemory& data() { return data_; }
    const FlatMemory& data() const { return data_; }

    const Cache& cpu_l1() const { return cpu_l1_; }
    const Cache& gpu_l1() const { return gpu_l1_; }
    const Cache& l2() const { return l2_; }
    const Dram& dram() const { return dram_; }

private:
    FlatMemory data_;
    Cache cpu_l1_;
    Cache gpu_l1_;
    Cache l2_;
    Dram dram_;
};

}  // namespace socsim
