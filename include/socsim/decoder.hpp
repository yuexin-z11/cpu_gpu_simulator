#pragma once
// RV32IM instruction decoding, shared by the CPU and GPU.

#include <cstdint>

#include "socsim/types.hpp"

namespace socsim {

enum class Op : std::uint8_t {
    Invalid,
    // RV32I
    Lui, Auipc, Jal, Jalr,
    Beq, Bne, Blt, Bge, Bltu, Bgeu,
    Lb, Lh, Lw, Lbu, Lhu, Sb, Sh, Sw,
    Addi, Slti, Sltiu, Xori, Ori, Andi, Slli, Srli, Srai,
    Add, Sub, Sll, Slt, Sltu, Xor, Srl, Sra, Or, And,
    Fence, Ecall, Ebreak,
    // Zicsr (only csrrs with rs1 = x0, i.e. `csrr`, is meaningful here)
    Csrrw, Csrrs, Csrrc, Csrrwi, Csrrsi, Csrrci,
    // M extension
    Mul, Mulh, Mulhsu, Mulhu, Div, Divu, Rem, Remu,
};

// Used to pick the cycle cost of an instruction.
enum class OpClass : std::uint8_t { Alu, Mul, Div, Branch, Load, Store, System };

struct DecodedInst {
    Op op = Op::Invalid;
    std::uint8_t rd = 0;
    std::uint8_t rs1 = 0;
    std::uint8_t rs2 = 0;
    std::int32_t imm = 0;
    std::uint16_t csr = 0;
    Word raw = 0;
};

DecodedInst decode(Word raw);
OpClass op_class(Op op);
const char* op_name(Op op);

}  // namespace socsim
