#include "socsim/simulator.hpp"

#include "socsim/elf_loader.hpp"

namespace socsim {

Simulator::Simulator(const ChipConfig& cfg)
    : cfg_(cfg),
      mem_(cfg),
      cpu_(cfg.cpu, mem_, log_),
      gpu_(cfg.gpu, mem_, log_) {}

void Simulator::load_program(const std::string& elf_path) {
    const LoadedProgram prog = load_elf(elf_path, mem_.data());
    cpu_.reset(prog.entry, prog.stack_top);
    now_ = 0;
}

RunResult Simulator::run() {
    RunResult r;
    while (now_ < cfg_.max_cycles) {
        if (!cpu_.step(now_)) break;
        if (cpu_.doorbell_rung()) {
            cpu_.clear_doorbell();
            gpu_.launch(now_);
        }
        gpu_.step(now_);
        ++now_;
    }
    // TODO: copy stats from cpu_, gpu_, and mem_ (caches and DRAM).
    r.total_cycles = now_;
    r.exit_code = cpu_.exit_code();
    r.passed = cpu_.halted() && cpu_.exit_code() == 0;
    r.exit_reason = cpu_.halted() ? "exit" : "max_cycles";
    return r;
}

}  // namespace socsim
