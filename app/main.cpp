// socsim command-line program: run, sweep, bench.

#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "socsim/config.hpp"
#include "socsim/simulator.hpp"
#include "socsim/sweep_runner.hpp"

namespace {

void usage() {
    std::cerr <<
        "usage:\n"
        "  socsim run   --config <chip.json> --program <prog.elf>\n"
        "  socsim sweep --sweep <sweep.json> --threads <n> --out <results.csv>\n"
        "  socsim bench --sweep <sweep.json> --threads <n,n,...> --out <speedup.csv>\n";
}

// Parses "--key value" pairs after the subcommand.
std::map<std::string, std::string> parse_flags(int argc, char** argv) {
    std::map<std::string, std::string> flags;
    for (int i = 2; i < argc; ++i) {
        std::string key = argv[i];
        if (key.rfind("--", 0) != 0 || i + 1 >= argc)
            throw std::invalid_argument("bad argument: " + key);
        flags[key.substr(2)] = argv[++i];
    }
    return flags;
}

std::string require(const std::map<std::string, std::string>& flags, const std::string& key) {
    auto it = flags.find(key);
    if (it == flags.end()) throw std::invalid_argument("missing --" + key);
    return it->second;
}

std::vector<unsigned> parse_thread_list(const std::string& s) {
    std::vector<unsigned> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ','))
        out.push_back(static_cast<unsigned>(std::stoul(item)));
    return out;
}

int cmd_run(const std::map<std::string, std::string>& flags) {
    const socsim::ChipConfig cfg = socsim::load_config(require(flags, "config"));
    socsim::Simulator sim(cfg);
    sim.load_program(require(flags, "program"));
    const socsim::RunResult r = sim.run();
    std::cout << socsim::format_summary(r, cfg.gpu.warp_size);
    return r.passed ? 0 : 1;
}

int cmd_sweep(const std::map<std::string, std::string>& flags) {
    const socsim::SweepSpec spec = socsim::load_sweep(require(flags, "sweep"));
    const unsigned threads = static_cast<unsigned>(std::stoul(require(flags, "threads")));
    const auto results = socsim::run_sweep(spec, threads);
    socsim::write_sweep_csv(require(flags, "out"), spec, results);
    return 0;
}

int cmd_bench(const std::map<std::string, std::string>& flags) {
    const socsim::SweepSpec spec = socsim::load_sweep(require(flags, "sweep"));
    const auto rows = socsim::bench_sweep(spec, parse_thread_list(require(flags, "threads")));
    socsim::write_bench_csv(require(flags, "out"), rows);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 2;
    }
    const std::string cmd = argv[1];
    try {
        const auto flags = parse_flags(argc, argv);
        if (cmd == "run") return cmd_run(flags);
        if (cmd == "sweep") return cmd_sweep(flags);
        if (cmd == "bench") return cmd_bench(flags);
        usage();
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
