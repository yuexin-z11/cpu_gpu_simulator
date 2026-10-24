#pragma once
// In-order RV32IM CPU: one instruction at a time, fixed cost per class.

#include "socsim/config.hpp"
#include "socsim/executor.hpp"
#include "socsim/memory_system.hpp"
#include "socsim/stats.hpp"
#include "socsim/types.hpp"

namespace socsim {

class EventLog;

class CpuCore {
public:
    CpuCore(const CpuConfig& cfg, MemorySystem& mem, EventLog& log);

    void reset(Addr entry, Addr stack_top);

    // Runs one instruction if the core is not stalled. Returns false once
    // the program has exited (ecall exit).
    bool step(Cycle now);

    bool halted() const { return halted_; }
    std::uint32_t exit_code() const { return exit_code_; }

    // Set when the host stores to mmio::kDoorbell; cleared by the Simulator.
    bool doorbell_rung() const { return doorbell_; }
    void clear_doorbell() { doorbell_ = false; }

    const CpuStats& stats() const { return stats_; }

private:
    Cycle cost(const DecodedInst& inst) const;
    void handle_ecall();

    CpuConfig cfg_;
    MemorySystem& mem_;
    EventLog& log_;
    RegFile regs_;
    Cycle busy_until_ = 0;
    bool halted_ = false;
    bool doorbell_ = false;
    std::uint32_t exit_code_ = 0;
    CpuStats stats_;
};

}  // namespace socsim
