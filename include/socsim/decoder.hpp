#pragma once
// RV32IM instruction decoding, shared by the CPU and GPU

#include <cstdint>

#include "socsim/types.hpp"

namespace socsim {

// clang-format off
enum class Op : std::uint8_t {
    invalid,
    // RV32I
    lui, auipc, jal, jalr,
    beq, bne, blt, bge, bltu, bgeu,
    lb, lh, lw, lbu, lhu, sb, sh, sw,
    addi, slti, sltiu, xori, ori, andi, slli, srli, srai,
    add, sub, sll, slt, sltu, xor_, srl, sra, or_, and_,
    fence, ecall, ebreak,
    // Zicsr (only csrrs with rs1 = x0, i.e. `csrr`, is meaningful here)
    csrrw, csrrs, csrrc, csrrwi, csrrsi, csrrci,
    // M extension
    mul, mulh, mulhsu, mulhu, div, divu, rem, remu,
};
// clang-format on

// used to pick the cycle cost of an instruction
enum class OpClass : std::uint8_t { alu, mul, div, branch, load, store, system };

struct DecodedInst {
    Op op             = Op::invalid;
    std::uint8_t rd   = 0;
    std::uint8_t rs1  = 0;
    std::uint8_t rs2  = 0;
    std::int32_t imm  = 0;
    std::uint16_t csr = 0;
    Word raw          = 0;
};

DecodedInst decode(Word raw);
OpClass op_class(Op op);
const char* op_name(Op op);

}  // namespace socsim
