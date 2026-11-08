#pragma once
// expands a sweep definition into configurations and runs them on
// std::thread workers. each worker owns its Simulator and writes into a
// fixed result slot, so the thread count cannot change any result

#include <cstddef>
#include <string>
#include <vector>

#include "socsim/config.hpp"
#include "socsim/stats.hpp"

namespace socsim {

struct SweepPoint {
    // e.g. "l2.size_bytes=8192,gpu.num_warps=4"
    std::string label;
    ChipConfig config;
};

struct SweepSpec {
    // ELF path
    std::string program;
    std::vector<SweepPoint> points;
};

// reads sweeps/*.json: a base config plus lists of values per parameter;
// produces the cross product in a fixed order
SweepSpec load_sweep(const std::string& path);

std::vector<RunResult> run_sweep(const SweepSpec& spec, unsigned threads);

void write_sweep_csv(const std::string& path, const SweepSpec& spec,
                     const std::vector<RunResult>& results);

struct BenchRow {
    unsigned threads = 0;
    double seconds   = 0.0;
};

// times run_sweep at each thread count; also checks that every run gives
// identical results (determinism). throws if they differ
std::vector<BenchRow> bench_sweep(const SweepSpec& spec,
                                  const std::vector<unsigned>& thread_counts);

void write_bench_csv(const std::string& path, const std::vector<BenchRow>& rows);

}  // namespace socsim
