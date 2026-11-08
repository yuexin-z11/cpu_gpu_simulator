#include "socsim/executor.hpp"

#include <stdexcept>

namespace socsim {

ExecResult execute(const DecodedInst& inst, RegFile& regs, FlatMemory& mem, const CsrReader& read_csr) {
    ExecResult r;
    r.next_pc = regs.pc + 4;

    // source register values
    const Word a = regs.x[inst.rs1];
    const Word b = regs.x[inst.rs2];
    // immediate as an unsigned word, so adds wrap like hardware
    const Word imm = static_cast<Word>(inst.imm);

    switch (inst.op) {
    // addi: rd = rs1 + imm
    case Op::addi: regs.x[inst.rd] = a + imm; break;
    // add: rd = rs1 + rs2
    case Op::add: regs.x[inst.rd] = a + b; break;
    // slti: set less than immediate (signed)
    case Op::slti: regs.x[inst.rd] = (static_cast<std::int32_t>(a) < inst.imm) ? 1 : 0; break;
    // sltiu: set less than immediate unsigned
    case Op::sltiu: regs.x[inst.rd] = (a < imm) ? 1 : 0; break;
    // xori: xor immediate
    case Op::xori: regs.x[inst.rd] = a ^ imm; break;
    // ori: or immediate
    case Op::ori: regs.x[inst.rd] = a | imm; break;
    // andi: and immediate
    case Op::andi: regs.x[inst.rd] = a & imm; break;
    // slli: shift left logical immediate
    case Op::slli: regs.x[inst.rd] = a << imm; break;
    // srli: shift right logical immediate (fill with 0)
    case Op::srli: regs.x[inst.rd] = a >> imm; break;
    // srai: shift right arithmetic immediate (copy sign bit)
    case Op::srai:
        regs.x[inst.rd] = static_cast<Word>(static_cast<std::int32_t>(a) >> imm);
        break;
    default: break;
    }

    // x0 is always 0, so undo any write to it
    regs.x[0] = 0;

    (void)mem;
    (void)read_csr;
    return r;
}

}  // namespace socsim
