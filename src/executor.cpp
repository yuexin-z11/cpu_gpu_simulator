#include "socsim/executor.hpp"

#include <limits>
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

    // load/store address = rs1 + imm
    const Addr addr = a + imm;

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
    // lb: load byte (sign-extended)
    case Op::lb:
        regs.x[inst.rd] = static_cast<Word>(static_cast<std::int8_t>(mem.read8(addr)));
        r.mem = MemRequest{addr, 1, false, Requester::cpu};
        break;
    // lh: load halfword (sign-extended)
    case Op::lh:
        regs.x[inst.rd] = static_cast<Word>(static_cast<std::int16_t>(mem.read16(addr)));
        r.mem = MemRequest{addr, 2, false, Requester::cpu};
        break;
    // lw: load word
    case Op::lw:
        regs.x[inst.rd] = mem.read32(addr);
        r.mem = MemRequest{addr, 4, false, Requester::cpu};
        break;
    // lbu: load byte unsigned (zero-extended)
    case Op::lbu:
        regs.x[inst.rd] = static_cast<Word>(mem.read8(addr));
        r.mem = MemRequest{addr, 1, false, Requester::cpu};
        break;
    // lhu: load halfword unsigned (zero-extended)
    case Op::lhu:
        regs.x[inst.rd] = static_cast<Word>(mem.read16(addr));
        r.mem = MemRequest{addr, 2, false, Requester::cpu};
        break;
    // sb: store byte (low 8 bits of rs2)
    case Op::sb:
        mem.write8(addr, static_cast<std::uint8_t>(b));
        r.mem = MemRequest{addr, 1, true, Requester::cpu};
        break;
    // sh: store halfword (low 16 bits of rs2)
    case Op::sh:
        mem.write16(addr, static_cast<std::uint16_t>(b));
        r.mem = MemRequest{addr, 2, true, Requester::cpu};
        break;
    // sw: store word
    case Op::sw:
        mem.write32(addr, b);
        r.mem = MemRequest{addr, 4, true, Requester::cpu};
        break;
    // mul: low 32 bits of rs1 * rs2
    case Op::mul: regs.x[inst.rd] = a * b; break;
    // mulh: high 32 bits, signed * signed
    case Op::mulh:
        regs.x[inst.rd] = static_cast<Word>(
            (static_cast<std::int64_t>(sa) * static_cast<std::int64_t>(sb)) >> 32);
        break;
    // mulhsu: high 32 bits, signed rs1 * unsigned rs2
    case Op::mulhsu:
        regs.x[inst.rd] = static_cast<Word>(
            (static_cast<std::int64_t>(sa) * static_cast<std::int64_t>(b)) >> 32);
        break;
    // mulhu: high 32 bits, unsigned * unsigned
    case Op::mulhu:
        regs.x[inst.rd] = static_cast<Word>(
            (static_cast<std::uint64_t>(a) * static_cast<std::uint64_t>(b)) >> 32);
        break;
    // div: signed divide (div by 0 -> -1, overflow -> INT32_MIN)
    case Op::div:
        if (b == 0) {
            regs.x[inst.rd] = 0xFFFFFFFFu;
        } else if (sa == std::numeric_limits<std::int32_t>::min() && sb == -1) {
            regs.x[inst.rd] = static_cast<Word>(sa);
        } else {
            regs.x[inst.rd] = static_cast<Word>(sa / sb);
        }
        break;
    // divu: unsigned divide
    case Op::divu:
        if (b == 0) {
            regs.x[inst.rd] = 0xFFFFFFFFu;
        } else {
            regs.x[inst.rd] = a / b;
        }
        break;
    // rem: signed remainder
    case Op::rem:
        if (b == 0) {
            regs.x[inst.rd] = a;
        } else if (sa == std::numeric_limits<std::int32_t>::min() && sb == -1) {
            regs.x[inst.rd] = 0;
        } else {
            regs.x[inst.rd] = static_cast<Word>(sa % sb);
        }
        break;
    // remu: unsigned remainder
    case Op::remu:
        if (b == 0) {
            regs.x[inst.rd] = a;
        } else {
            regs.x[inst.rd] = a % b;
        }
        break;
    // fence: memory ordering (no effect here, just flagged)
    case Op::fence: r.is_fence = true; break;
    // ecall: environment call (the core decides what it means)
    case Op::ecall: r.is_ecall = true; break;
    // ebreak: breakpoint
    case Op::ebreak: r.is_ebreak = true; break;
    // csrrs with rs1 = x0 is csrr: read a CSR into rd
    case Op::csrrs: {
        // our CSRs are read-only, so setting bits is illegal
        if (inst.rs1 != 0) throw std::runtime_error("csr write not supported");
        const std::optional<Word> v = read_csr(inst.csr);
        // this core does not have that CSR
        if (!v) throw std::runtime_error("unknown csr");
        regs.x[inst.rd] = *v;
        break;
    }
    // every other CSR op writes a CSR, which is not supported
    case Op::csrrw:
    case Op::csrrc:
    case Op::csrrwi:
    case Op::csrrsi:
    case Op::csrrci: throw std::runtime_error("csr write not supported");
    // anything not handled above is an illegal instruction
    default: throw std::runtime_error("illegal instruction");
    }

    // x0 is always 0, so undo any write to it
    regs.x[0] = 0;
    
    return r;
}

}  // namespace socsim
