#include "socsim/simulator.hpp"

#include "socsim/elf_loader.hpp"

namespace socsim {

Simulator::Simulator(const ChipConfig& cfg)
    : cfg_(cfg), mem_(cfg), cpu_(cfg.cpu, mem_, log_), gpu_(cfg.gpu, mem_, log_) {}

void Simulator::load_program(const std::string& elf_path) {
    const LoadedProgram prog = load_elf(elf_path, mem_.data());
    cpu_.reset(prog.entry, prog.stack_top);
    now_ = 0;
}

void Simulator::load_words(Addr base, const std::vector<Word>& words, Addr entry) {
    for (std::size_t i = 0; i < words.size(); ++i) {
        // word i sits 4 bytes after word i - 1
        mem_.data().write32(base + static_cast<Addr>(i * 4), words[i]);
    }
    // same stack top as the ELF loader (link.ld __stack_top)
    cpu_.reset(entry, 0x00F0'0000);
    // start the clock over
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
    // copy every component's stats into the result
    r.cpu    = cpu_.stats();
    r.gpu    = gpu_.stats();
    r.cpu_l1 = mem_.cpu_l1().stats();
    r.gpu_l1 = mem_.gpu_l1().stats();
    r.l2     = mem_.l2().stats();
    r.dram   = mem_.dram().stats();

    r.total_cycles = now_;
    r.exit_code    = cpu_.exit_code();
    r.passed       = cpu_.halted() && cpu_.exit_code() == 0;
    r.exit_reason  = cpu_.halted() ? "exit" : "max_cycles";
    return r;
}

}  // namespace socsim
