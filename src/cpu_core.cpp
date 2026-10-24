#include "socsim/cpu_core.hpp"

#include "socsim/event_log.hpp"

namespace socsim {

CpuCore::CpuCore(const CpuConfig& cfg, MemorySystem& mem, EventLog& log)
    : cfg_(cfg), mem_(mem), log_(log) {}

void CpuCore::reset(Addr entry, Addr stack_top) {
    regs_ = RegFile{};
    regs_.pc = entry;
    regs_.x[2] = stack_top;  // sp
    busy_until_ = 0;
    halted_ = false;
    doorbell_ = false;
    exit_code_ = 0;
    stats_ = CpuStats{};
}

Cycle CpuCore::cost(const DecodedInst& inst) const {
    // TODO: map op_class(inst.op) to the cycle counts in cfg_.
    (void)inst;
    return cfg_.alu_cycles;
}

void CpuCore::handle_ecall() {
    // TODO: a7 == 93 (exit) -> halted_ = true, exit_code_ = a0.
}

bool CpuCore::step(Cycle now) {
    if (halted_) return false;
    if (now < busy_until_) return true;
    // TODO: fetch at regs_.pc, decode, execute, add cost + memory latency
    // to busy_until_, detect stores to mmio::kDoorbell, handle ecall,
    // update stats_ and pc.
    halted_ = true;  // remove once execution is implemented
    return false;
}

}  // namespace socsim
