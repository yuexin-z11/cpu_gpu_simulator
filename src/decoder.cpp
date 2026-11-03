#include "socsim/decoder.hpp"

namespace socsim {

DecodedInst decode(Word raw) {
    DecodedInst d;
    d.raw = raw;

    const Word opcode = raw & 0x7F;                          // bits 6..0
    const Word funct3 = (raw >> 12) & 0x7;                   // bits 14..12
    const Word funct7 = (raw >> 25) & 0x7F;                  // bits 31..25
    d.rd  = static_cast<std::uint8_t>((raw >> 7) & 0x1F);    // bits 11..7
    d.rs1 = static_cast<std::uint8_t>((raw >> 15) & 0x1F);   // bits 19..15
    d.rs2 = static_cast<std::uint8_t>((raw >> 20) & 0x1F);   // bits 24..20

    // I-type ALU
    if (opcode == 0x13) {
        d.imm = static_cast<std::int32_t>(raw) >> 20;
        switch (funct3) {
        // addi: add immediate
        case 0: d.op = Op::Addi;  break;
        // slli: shift left logical immediate
        case 1:
            d.op  = Op::Slli;
            // shamt = bits 24..20
            d.imm = d.rs2;
            break;
        // slti: set less than immediate (signed)
        case 2: d.op = Op::Slti; break;
        // sltiu: set less than immediate unsigned
        case 3: d.op = Op::Sltiu; break;
        // xori: xor immediate
        case 4: d.op = Op::Xori;  break;
        // srli: shift right logical immediate (funct7 = 0x00)
        // srai: shift right arithmetic immediate (funct7 = 0x20)
        case 5:
            d.op  = (funct7 == 0x20) ? Op::Srai : Op::Srli;
            d.imm = d.rs2;
            break;
        // ori: or immediate
        case 6: d.op = Op::Ori;   break;
        // andi: and immediate
        case 7: d.op = Op::Andi;  break;
        default: break;
        }
    }

    // load instructions 
    if (opcode == 0x03) {
        d.imm = static_cast<std::int32_t>(raw) >> 20;
        switch (funct3) {
        // lb: load byte
        case 0: d.op = Op::Lb;  break;
        // lh: load halfword
        case 1: d.op = Op::Lh; break;
        // lw: load word
        case 2: d.op = Op::Lw;  break;
        // lbu: load byte unsigned
        case 4: d.op = Op::Lbu; break;
        // lhu: load halfword unsigned
        case 5: d.op = Op::Lhu; break;
        default: break;   // 3, 6, 7 are not valid loads
        }
    }

    return d;
}

OpClass op_class(Op op) {
    switch (op) {
    case Op::Mul: case Op::Mulh: case Op::Mulhsu: case Op::Mulhu:
        return OpClass::Mul;
    case Op::Div: case Op::Divu: case Op::Rem: case Op::Remu:
        return OpClass::Div;
    case Op::Jal: case Op::Jalr:
    case Op::Beq: case Op::Bne: case Op::Blt: case Op::Bge: case Op::Bltu: case Op::Bgeu:
        return OpClass::Branch;
    case Op::Lb: case Op::Lh: case Op::Lw: case Op::Lbu: case Op::Lhu:
        return OpClass::Load;
    case Op::Sb: case Op::Sh: case Op::Sw:
        return OpClass::Store;
    case Op::Fence: case Op::Ecall: case Op::Ebreak:
    case Op::Csrrw: case Op::Csrrs: case Op::Csrrc:
    case Op::Csrrwi: case Op::Csrrsi: case Op::Csrrci:
        return OpClass::System;
    default:
        return OpClass::Alu;
    }
}

const char* op_name(Op op) {
    // TODO: return the assembly mnemonic for debugging and the event log.
    (void)op;
    return "?";
}

}  // namespace socsim
