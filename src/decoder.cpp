#include "socsim/decoder.hpp"

namespace socsim {

DecodedInst decode(Word raw) {
    DecodedInst d;
    d.raw = raw;
    // TODO: switch on opcode (raw & 0x7f), then funct3 / funct7; extract
    // rd, rs1, rs2 and the I/S/B/U/J immediates (sign-extended).
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
