#pragma once
// Results of one simulation run.

#include <cstdint>
#include <string>
#include <vector>

#include "socsim/cache.hpp"
#include "socsim/dram.hpp"

namespace socsim {

struct CpuStats {
    std::uint64_t instructions = 0;
    std::uint64_t cycles = 0;
    std::uint64_t mem_stall_cycles = 0;
    std::uint64_t fence_poll_cycles = 0;
};

struct GpuStats {
    std::uint64_t warp_instructions = 0;   // issued warp instructions
    std::uint64_t active_lane_sum = 0;     // sum of active lanes over issues
    std::uint64_t warp_mem_instructions = 0;
    std::uint64_t cache_line_requests = 0; // after coalescing
    std::uint64_t cycles = 0;              // launch to completion

    // active lanes / warp size, averaged over issued instructions
    double divergence_efficiency(std::uint32_t warp_size) const;
    // cache lines per warp memory instruction
    double lines_per_mem_instruction() const;
};

struct RunResult {
    bool passed = false;
    std::string exit_reason;  // e.g. "ecall exit", "max_cycles"
    std::uint32_t exit_code = 0;
    std::uint64_t total_cycles = 0;
    CpuStats cpu;
    GpuStats gpu;
    CacheStats cpu_l1;
    CacheStats gpu_l1;
    CacheStats l2;
    DramStats dram;
};

// CSV helpers so the sweep runner and single runs share one column order.
std::vector<std::string> csv_header();
std::vector<std::string> csv_row(const RunResult& r, std::uint32_t warp_size);

// Human-readable summary for `socsim run`.
std::string format_summary(const RunResult& r, std::uint32_t warp_size);

}  // namespace socsim
