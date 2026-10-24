#include "socsim/stats.hpp"

#include <sstream>

namespace socsim {

double GpuStats::divergence_efficiency(std::uint32_t warp_size) const {
    if (warp_instructions == 0 || warp_size == 0) return 0.0;
    return static_cast<double>(active_lane_sum) /
           (static_cast<double>(warp_instructions) * warp_size);
}

double GpuStats::lines_per_mem_instruction() const {
    if (warp_mem_instructions == 0) return 0.0;
    return static_cast<double>(cache_line_requests) /
           static_cast<double>(warp_mem_instructions);
}

std::vector<std::string> csv_header() {
    // TODO: extend with every column you want in the sweep CSV.
    return {"passed", "total_cycles", "cpu_instructions", "gpu_warp_instructions",
            "divergence_efficiency", "lines_per_mem_instruction",
            "cpu_l1_hit_rate", "gpu_l1_hit_rate", "l2_hit_rate", "l2_misses"};
}

std::vector<std::string> csv_row(const RunResult& r, std::uint32_t warp_size) {
    // TODO: keep the same order as csv_header(); use fixed precision so
    // output is byte-identical across runs.
    (void)r;
    (void)warp_size;
    return {};
}

std::string format_summary(const RunResult& r, std::uint32_t warp_size) {
    std::ostringstream out;
    out << (r.passed ? "PASS" : "FAIL") << "  (" << r.exit_reason << ")\n"
        << "total cycles: " << r.total_cycles << '\n';
    // TODO: CPU/GPU stats, hit rates, divergence efficiency, lines/request.
    (void)warp_size;
    return out.str();
}

}  // namespace socsim
