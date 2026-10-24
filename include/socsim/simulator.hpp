#pragma once
// Owns one chip and advances its clock until the program exits.

#include <string>

#include "socsim/config.hpp"
#include "socsim/cpu_core.hpp"
#include "socsim/event_log.hpp"
#include "socsim/gpu_core.hpp"
#include "socsim/memory_system.hpp"
#include "socsim/stats.hpp"

namespace socsim {

class Simulator {
public:
    explicit Simulator(const ChipConfig& cfg);

    void load_program(const std::string& elf_path);

    // Runs until the CPU exits or cfg.max_cycles is reached.
    RunResult run();

    EventLog& event_log() { return log_; }
    MemorySystem& memory() { return mem_; }
    const ChipConfig& config() const { return cfg_; }

private:
    ChipConfig cfg_;
    MemorySystem mem_;
    EventLog log_;
    CpuCore cpu_;
    GpuCore gpu_;
    Cycle now_ = 0;
};

}  // namespace socsim
