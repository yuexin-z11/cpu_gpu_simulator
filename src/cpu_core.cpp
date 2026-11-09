#include "socsim/cpu_core.hpp"

#include "socsim/event_log.hpp"

namespace socsim {

CpuCore::CpuCore(const CpuConfig& cfg, MemorySystem& mem, EventLog& log)
    : cfg_(cfg), mem_(mem), log_(log) {}

void CpuCore::reset(Addr entry, Addr stack_top) {
    regs_    = RegFile{};
    regs_.pc = entry;
    // sp
    regs_.x[2]  = stack_top;
    busy_until_ = 0;
    halted_     = false;
    doorbell_   = false;
    exit_code_  = 0;
    stats_      = CpuStats{};
}

Cycle CpuCore::cost(const DecodedInst& inst) const {
    switch (op_class(inst.op)) {
    // multiply and divide take longer than simple alu ops
    case OpClass::mul: return cfg_.mul_cycles;
    case OpClass::div: return cfg_.div_cycles;
    // branches and jumps
    case OpClass::branch: return cfg_.branch_cycles;
    // alu, load/store issue, and system ops take one alu slot
    default: return cfg_.alu_cycles;
    }
}

void CpuCore::handle_ecall() {
    // a7 (x17) holds the syscall number, 93 = exit
    if (regs_.x[17] == 93) {
        halted_ = true;
        // a0 (x10) holds the exit code
        exit_code_ = regs_.x[10];
    }
}

bool CpuCore::step(Cycle now) {
    if (halted_) return false;
    // still busy with the previous instruction
    if (now < busy_until_) return true;

    // fetch and decode the instruction at pc
    const Word raw         = mem_.data().read32(regs_.pc);
    const DecodedInst inst = decode(raw);

    // the CPU has no CSRs (lane id etc. are GPU-only)
    auto no_csrs = [](std::uint16_t) -> std::optional<Word> { return std::nullopt; };
    const ExecResult r = execute(inst, regs_, mem_.data(), no_csrs);

    // time for this instruction: issue cost plus any memory latency
    Cycle t = cost(inst);
    if (r.mem) {
        // the executor does not know who issued the request
        MemRequest req = *r.mem;
        req.source     = Requester::cpu;
        const Cycle lat = mem_.access(now, req);
        t += lat;
        stats_.mem_stall_cycles += lat;
        // a store to the doorbell starts the GPU
        if (req.is_write && req.addr == mmio::kDoorbell) doorbell_ = true;
    }
    busy_until_ = now + t;

    if (r.is_ecall) handle_ecall();

    // move to the next instruction
    regs_.pc = r.next_pc;
    ++stats_.instructions;
    stats_.cycles = busy_until_;
    return !halted_;
}

}  // namespace socsim
