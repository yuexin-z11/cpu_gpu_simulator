#pragma once
// Chip configuration, loaded from JSON (see configs/default.json).

#include <cstdint>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace socsim {

struct CacheConfig {
    std::uint32_t size_bytes = 4096;
    std::uint32_t line_bytes = 64;
    std::uint32_t ways = 4;
    std::uint32_t hit_latency = 1;  // cycles
};

struct DramConfig {
    std::uint32_t latency = 100;         // cycles
    std::uint32_t bytes_per_cycle = 16;  // bandwidth limit
};

struct CpuConfig {
    std::uint32_t alu_cycles = 1;
    std::uint32_t mul_cycles = 3;
    std::uint32_t div_cycles = 20;
    std::uint32_t branch_cycles = 1;
};

struct GpuConfig {
    std::uint32_t num_warps = 4;
    std::uint32_t warp_size = 16;
};

struct ChipConfig {
    CpuConfig cpu;
    GpuConfig gpu;
    CacheConfig cpu_l1;
    CacheConfig gpu_l1;
    CacheConfig l2;
    DramConfig dram;
    std::uint64_t max_cycles = 100'000'000;  // safety stop
};

ChipConfig config_from_json(const nlohmann::json& j);
nlohmann::json config_to_json(const ChipConfig& c);
ChipConfig load_config(const std::string& path);

// Throws std::invalid_argument if a value is unusable (e.g. size not a
// multiple of line_bytes * ways).
void validate(const ChipConfig& c);

}  // namespace socsim
