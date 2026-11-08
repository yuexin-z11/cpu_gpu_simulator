#include "socsim/executor.hpp"

#include <stdexcept>

namespace socsim {

ExecResult execute(const DecodedInst& inst, RegFile& regs, FlatMemory& mem,
                   const CsrReader& read_csr) {
    ExecResult r;
    r.next_pc = regs.pc + 4;

    // source register values
    const Word a = regs.x[inst.rs1];
    const Word b = regs.x[inst.rs2];
    // immediate as an unsigned word, so adds wrap like hardware
    const Word imm = static_cast<Word>(inst.imm);

    // shift amount = low 5 bits of rs2
    const Word sh = b & 0x1F;

    // signed views of rs1 and rs2
    const std::int32_t sa = static_cast<std::int32_t>(a);
    const std::int32_t sb = static_cast<std::int32_t>(b);

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
    case Op::srai: regs.x[inst.rd] = static_cast<Word>(static_cast<std::int32_t>(a) >> imm); break;
    // sub: rd = rs1 - rs2
    case Op::sub: regs.x[inst.rd] = a - b; break;
    // sll: shift left logical
    case Op::sll: regs.x[inst.rd] = a << sh; break;
    // slt: set less than (signed)
    case Op::slt:
        regs.x[inst.rd] = (static_cast<std::int32_t>(a) < static_cast<std::int32_t>(b)) ? 1 : 0;
        break;
    // sltu: set less than unsigned
    case Op::sltu: regs.x[inst.rd] = (a < b) ? 1 : 0; break;
    // xor: exclusive or
    case Op::xor_: regs.x[inst.rd] = a ^ b; break;
    // srl: shift right logical (fill with 0)
    case Op::srl: regs.x[inst.rd] = a >> sh; break;
    // sra: shift right arithmetic (copy sign bit)
    case Op::sra: regs.x[inst.rd] = static_cast<Word>(static_cast<std::int32_t>(a) >> sh); break;
    // or: or
    case Op::or_: regs.x[inst.rd] = a | b; break;
    // and: and
    case Op::and_: regs.x[inst.rd] = a & b; break;
    // lui: load upper immediate
    case Op::lui: regs.x[inst.rd] = imm; break;
    // auipc: add upper immediate to pc
    case Op::auipc: regs.x[inst.rd] = regs.pc + imm; break;
    // beq: branch if equal
    case Op::beq:
        if (a == b) r.next_pc = regs.pc + imm;
        break;
    // bne: branch if not equal
    case Op::bne:
        if (sa != sb) r.next_pc = regs.pc + imm;
        break;
    // blt: branch if less than (signed)
    case Op::blt:
        if (sa < sb) r.next_pc = regs.pc + imm;
        break;
    // bge: branch if greater or equal (signed)
    case Op::bge:
        if (sa >= sb) r.next_pc = regs.pc + imm;
        break;
    // bltu: branch if less than unsigned
    case Op::bltu:
        if (a < b) r.next_pc = regs.pc + imm;
        break;
    // bgeu: branch if greater or equal unsigned
    case Op::bgeu:
        if (a >= b) r.next_pc = regs.pc + imm;
        break;
    // jal: jump and link (rd = return address)
    case Op::jal:
        regs.x[inst.rd] = regs.pc + 4;
        r.next_pc       = regs.pc + imm;
        break;
    // jalr: jump and link register (bit 0 of target cleared)
    case Op::jalr:
        regs.x[inst.rd] = regs.pc + 4;
        r.next_pc       = (a + imm) & ~1u;
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
