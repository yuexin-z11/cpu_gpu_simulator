#include "socsim/decoder.hpp"

namespace socsim {

DecodedInst decode(Word raw) {
    DecodedInst d;
    d.raw = raw;

    // bits 6..0
    const Word opcode = raw & 0x7F;
    // bits 14..12
    const Word funct3 = (raw >> 12) & 0x7;
    // bits 31..25
    const Word funct7 = (raw >> 25) & 0x7F;
    // bits 11..7
    d.rd = static_cast<std::uint8_t>((raw >> 7) & 0x1F);
    // bits 19..15
    d.rs1 = static_cast<std::uint8_t>((raw >> 15) & 0x1F);
    // bits 24..20
    d.rs2 = static_cast<std::uint8_t>((raw >> 20) & 0x1F);

    // i-type ALU
    if (opcode == 0x13) {
        d.imm = static_cast<std::int32_t>(raw) >> 20;
        switch (funct3) {
        // addi: add immediate
        case 0: d.op = Op::addi; break;
        // slli: shift left logical immediate
        case 1:
            d.op = Op::slli;
            // shamt = bits 24..20
            d.imm = d.rs2;
            break;
        // slti: set less than immediate (signed)
        case 2: d.op = Op::slti; break;
        // sltiu: set less than immediate unsigned
        case 3: d.op = Op::sltiu; break;
        // xori: xor immediate
        case 4: d.op = Op::xori; break;
        // srli: shift right logical immediate (funct7 = 0x00)
        // srai: shift right arithmetic immediate (funct7 = 0x20)
        case 5:
            d.op  = (funct7 == 0x20) ? Op::srai : Op::srli;
            d.imm = d.rs2;
            break;
        // ori: or immediate
        case 6: d.op = Op::ori; break;
        // andi: and immediate
        case 7: d.op = Op::andi; break;
        default: break;
        }
    }

    // load instructions
    if (opcode == 0x03) {
        d.imm = static_cast<std::int32_t>(raw) >> 20;
        switch (funct3) {
        // lb: load byte
        case 0: d.op = Op::lb; break;
        // lh: load halfword
        case 1: d.op = Op::lh; break;
        // lw: load word
        case 2: d.op = Op::lw; break;
        // lbu: load byte unsigned
        case 4: d.op = Op::lbu; break;
        // lhu: load halfword unsigned
        case 5: d.op = Op::lhu; break;
        // 3, 6, 7 are not valid loads
        default: break;
        }
    }

    // store instructions
    if (opcode == 0x23) {
        d.imm = ((static_cast<std::int32_t>(raw) >> 20) & ~0x1F)
                | static_cast<std::int32_t>((raw >> 7) & 0x1F);
        switch (funct3) {
        // sb: store byte
        case 0: d.op = Op::sb; break;
        // sh: store halfword
        case 1: d.op = Op::sh; break;
        // sw: store word
        case 2: d.op = Op::sw; break;
        default: break;
        }
    }

    // register-register ALU
    if (opcode == 0x33 && funct7 != 0x01) {
        switch (funct3) {
        // add: add (funct7 = 0x00), sub: subtract (funct7 = 0x20)
        case 0: d.op = (funct7 == 0x20) ? Op::sub : Op::add; break;
        // sll: shift left logical
        case 1: d.op = Op::sll; break;
        // slt: set less than (signed)
        case 2: d.op = Op::slt; break;
        // sltu: set less than unsigned
        case 3: d.op = Op::sltu; break;
        // xor: exclusive or
        case 4: d.op = Op::xor_; break;
        // srl: shift right logical (funct7 = 0x00), sra: shift right arithmetic (funct7 = 0x20)
        case 5: d.op = (funct7 == 0x20) ? Op::sra : Op::srl; break;
        // or: or
        case 6: d.op = Op::or_; break;
        // and: and
        case 7: d.op = Op::and_; break;
        default: break;
        }
    }

    // multiply and divide (M extension)
    if (opcode == 0x33 && funct7 == 0x01) {
        switch (funct3) {
        // mul: multiply (low 32 bits)
        case 0: d.op = Op::mul; break;
        // mulh: multiply high (signed x signed)
        case 1: d.op = Op::mulh; break;
        // mulhsu: multiply high (signed x unsigned)
        case 2: d.op = Op::mulhsu; break;
        // mulhu: multiply high (unsigned x unsigned)
        case 3: d.op = Op::mulhu; break;
        // div: divide (signed)
        case 4: d.op = Op::div; break;
        // divu: divide unsigned
        case 5: d.op = Op::divu; break;
        // rem: remainder (signed)
        case 6: d.op = Op::rem; break;
        // remu: remainder unsigned
        case 7: d.op = Op::remu; break;
        default: break;
        }
    }

    // lui: load upper immediate
    if (opcode == 0x37) {
        d.op  = Op::lui;
        d.imm = static_cast<std::int32_t>(raw & 0xFFFFF000);
    }

    // auipc: add upper immediate to pc
    if (opcode == 0x17) {
        d.op  = Op::auipc;
        d.imm = static_cast<std::int32_t>(raw & 0xFFFFF000);
    }

    // branch instructions
    if (opcode == 0x63) {
        // imm[12] and sign: bit 31 -> bit 12 and up
        // imm[10:5]: bits 30..25 -> bits 10..5
        // imm[4:1]: bits 11..8 -> bits 4..1
        // imm[11]: bit 7 -> bit 11
        d.imm = ((static_cast<std::int32_t>(raw) >> 19) & ~0xFFF)
                | static_cast<std::int32_t>((raw >> 20) & 0x7E0)
                | static_cast<std::int32_t>((raw >> 7) & 0x1E)
                | static_cast<std::int32_t>((raw << 4) & 0x800);
        switch (funct3) {
        // beq: branch if equal
        case 0: d.op = Op::beq; break;
        // bne: branch if not equal
        case 1: d.op = Op::bne; break;
        // blt: branch if less than (signed)
        case 4: d.op = Op::blt; break;
        // bge: branch if greater or equal (signed)
        case 5: d.op = Op::bge; break;
        // bltu: branch if less than unsigned
        case 6: d.op = Op::bltu; break;
        // bgeu: branch if greater or equal unsigned
        case 7: d.op = Op::bgeu; break;
        // 2 and 3 are not valid branches
        default: break;
        }
    }

    // jal: jump and link
    if (opcode == 0x6F) {
        d.op = Op::jal;
        // imm[20] and sign: bit 31 -> bit 20 and up
        // imm[19:12]: already in place
        // imm[11]: bit 20 -> bit 11
        // imm[10:1]: bits 30..21 -> bits 10..1
        d.imm = ((static_cast<std::int32_t>(raw) >> 11) & ~0xFFFFF)
                | static_cast<std::int32_t>(raw & 0xFF000)
                | static_cast<std::int32_t>((raw >> 9) & 0x800)
                | static_cast<std::int32_t>((raw >> 20) & 0x7FE);
    }

    // jalr: jump and link register
    if (opcode == 0x67 && funct3 == 0) {
        d.op  = Op::jalr;
        d.imm = static_cast<std::int32_t>(raw) >> 20;
    }

    // system instructions
    if (opcode == 0x73) {
        // csr number = bits 31..20, not sign-extended
        d.csr = static_cast<std::uint16_t>(raw >> 20);
        switch (funct3) {
        // ecall: environment call (csr = 0), ebreak: breakpoint (csr = 1)
        case 0: d.op = (d.csr == 0) ? Op::ecall : Op::ebreak; break;
        // csrrw: csr read and write
        case 1: d.op = Op::csrrw; break;
        // csrrs: csr read and set (csrr when rs1 = x0)
        case 2: d.op = Op::csrrs; break;
        // csrrc: csr read and clear
        case 3: d.op = Op::csrrc; break;
        // csrrwi: csr read and write immediate
        case 5: d.op = Op::csrrwi; break;
        // csrrsi: csr read and set immediate
        case 6: d.op = Op::csrrsi; break;
        // csrrci: csr read and clear immediate
        case 7: d.op = Op::csrrci; break;
        default: break;
        }
    }

    // fence: memory ordering (no effect in this simulator)
    if (opcode == 0x0F) {
        d.op = Op::fence;
    }

    return d;
}

OpClass op_class(Op op) {
    switch (op) {
    case Op::mul:
    case Op::mulh:
    case Op::mulhsu:
    case Op::mulhu: return OpClass::mul;
    case Op::div:
    case Op::divu:
    case Op::rem:
    case Op::remu: return OpClass::div;
    case Op::jal:
    case Op::jalr:
    case Op::beq:
    case Op::bne:
    case Op::blt:
    case Op::bge:
    case Op::bltu:
    case Op::bgeu: return OpClass::branch;
    case Op::lb:
    case Op::lh:
    case Op::lw:
    case Op::lbu:
    case Op::lhu: return OpClass::load;
    case Op::sb:
    case Op::sh:
    case Op::sw: return OpClass::store;
    case Op::fence:
    case Op::ecall:
    case Op::ebreak:
    case Op::csrrw:
    case Op::csrrs:
    case Op::csrrc:
    case Op::csrrwi:
    case Op::csrrsi:
    case Op::csrrci: return OpClass::system;
    default: return OpClass::alu;
    }
}

const char* op_name(Op op) {
    // TODO: return the assembly mnemonic for debugging and the event log
    (void)op;
    return "?";
}

}  // namespace socsim
