#include "socsim/executor.hpp"

#include <stdexcept>

namespace socsim {

ExecResult execute(const DecodedInst& inst, RegFile& regs, FlatMemory& mem,
                   const CsrReader& read_csr) {
    ExecResult r;
    r.next_pc = regs.pc + 4;
    // TODO: one case per Op. Remember:
    //  - writes to x0 are discarded
    //  - branches/jumps set r.next_pc
    //  - loads/stores access `mem` and fill r.mem for timing
    //  - DIV/REM by zero and overflow follow the RISC-V spec (no trap)
    //  - csrr calls read_csr; throw if it returns std::nullopt
    (void)inst;
    (void)mem;
    (void)read_csr;
    return r;
}

}  // namespace socsim
