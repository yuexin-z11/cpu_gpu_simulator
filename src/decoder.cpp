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

    // store instructions
    if (opcode == 0x23) {
        d.imm = ((static_cast<std::int32_t>(raw) >> 20) & ~0x1F) | static_cast<std::int32_t>((raw >> 7) & 0x1F);
        switch (funct3) {
        // sb: store byte
        case 0: d.op = Op::Sb; break;
        // sh: store halfword
        case 1: d.op = Op::Sh; break;
        // sw: store word
        case 2: d.op = Op::Sw; break;
        default: break;
        }
    }

    // register-register ALU
    if (opcode == 0x33 && funct7 != 0x01) {
        switch (funct3) {
        // add: add (funct7 = 0x00), sub: subtract (funct7 = 0x20)
        case 0: d.op = (funct7 == 0x20) ? Op::Sub : Op::Add; break;
        // sll: shift left logical
        case 1: d.op = Op::Sll;  break;
        // slt: set less than (signed)
        case 2: d.op = Op::Slt; break;
        // sltu: set less than unsigned
        case 3: d.op = Op::Sltu; break;
        // xor: exclusive or
        case 4: d.op = Op::Xor;  break;
        // srl: shift right logical (funct7 = 0x00), sra: shift right arithmetic (funct7 = 0x20)
        case 5: d.op = (funct7 == 0x20) ? Op::Sra : Op::Srl; break;
        // or: or
        case 6: d.op = Op::Or;   break;
        // and: and
        case 7: d.op = Op::And; break;
        default: break;
        }
    }

    // multiply and divide (M extension)
    if (opcode == 0x33 && funct7 == 0x01) {
        switch (funct3) {
        // mul: multiply (low 32 bits)
        case 0: d.op = Op::Mul;    break;
        // mulh: multiply high (signed x signed)
        case 1: d.op = Op::Mulh; break;
        // mulhsu: multiply high (signed x unsigned)
        case 2: d.op = Op::Mulhsu; break;
        // mulhu: multiply high (unsigned x unsigned)
        case 3: d.op = Op::Mulhu;  break;
        // div: divide (signed)
        case 4: d.op = Op::Div;  break;
        // divu: divide unsigned
        case 5: d.op = Op::Divu; break;
        // rem: remainder (signed)
        case 6: d.op = Op::Rem;    break;
        // remu: remainder unsigned
        case 7: d.op = Op::Remu; break;
        default: break;
        }
    }

    // lui: load upper immediate
    if (opcode == 0x37) {
        d.op  = Op::Lui;
        d.imm = static_cast<std::int32_t>(raw & 0xFFFFF000);
    }

    // auipc: add upper immediate to pc
    if (opcode == 0x17) {
        d.op  = Op::Auipc;
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
              | static_cast<std::int32_t>((raw << 11) & 0x800);
        switch (funct3) {
        // beq: branch if equal
        case 0: d.op = Op::Beq;  break;
        // bne: branch if not equal
        case 1: d.op = Op::Bne; break;
        // blt: branch if less than (signed)
        case 4: d.op = Op::Blt; break;
        // bge: branch if greater or equal (signed)
        case 5: d.op = Op::Bge;  break;
        // bltu: branch if less than unsigned
        case 6: d.op = Op::Bltu; break;
        // bgeu: branch if greater or equal unsigned
        case 7: d.op = Op::Bgeu; break;
        default: break;   // 2 and 3 are not valid branches
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
