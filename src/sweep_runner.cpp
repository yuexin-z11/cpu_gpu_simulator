#include "socsim/sweep_runner.hpp"

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <thread>

#include <nlohmann/json.hpp>

#include "socsim/simulator.hpp"

namespace socsim {

SweepSpec load_sweep(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open sweep: " + path);
    const nlohmann::json j = nlohmann::json::parse(in);

    SweepSpec spec;
    // TODO: read "program", load "base" (a config path or inline object),
    // and expand "parameters" ({"l2.size_bytes": [..], ...}) into the cross
    // product, in a fixed order, as SweepPoints.
    (void)j;
    return spec;
}

std::vector<RunResult> run_sweep(const SweepSpec& spec, unsigned threads) {
    std::vector<RunResult> results(spec.points.size());
    // TODO: start `threads` std::thread workers; each takes the next point
    // index (e.g. from a std::atomic<size_t>), builds its own Simulator,
    // and writes results[index]. Join all workers.
    (void)threads;
    return results;
}

void write_sweep_csv(const std::string& path, const SweepSpec& spec,
                     const std::vector<RunResult>& results) {
    // TODO: header = "label" + csv_header(); one row per point.
    (void)path;
    (void)spec;
    (void)results;
}

std::vector<BenchRow> bench_sweep(const SweepSpec& spec,
                                  const std::vector<unsigned>& thread_counts) {
    std::vector<BenchRow> rows;
    // TODO: for each thread count, time run_sweep with
    // std::chrono::steady_clock; compare csv rows against the first run and
    // throw if any differ.
    (void)spec;
    (void)thread_counts;
    return rows;
}

void write_bench_csv(const std::string& path, const std::vector<BenchRow>& rows) {
    // TODO: columns threads,seconds,speedup (speedup relative to 1 thread).
    (void)path;
    (void)rows;
}

}  // namespace socsim
