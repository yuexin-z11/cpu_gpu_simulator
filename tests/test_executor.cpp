#include <optional>

#include "check.hpp"
#include "socsim/decoder.hpp"
#include "socsim/executor.hpp"

using namespace socsim;

// csr reader that allows no csrs (cpu-style)
static std::optional<Word> no_csrs(std::uint16_t) {
    return std::nullopt;
}

// addi and add write rd, and x0 stays 0
TEST(execute_addi_add) {
    RegFile regs;
    FlatMemory mem(64);

    // addi x1, x0, 5
    execute(decode(0x00500093), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[1], 5u);

    // addi x2, x0, -1
    execute(decode(0xFFF00113), regs, mem, no_csrs);
    // -1 as an unsigned word
    EXPECT_EQ(regs.x[2], 0xFFFFFFFFu);

    // add x3, x1, x2  (5 + -1)
    execute(decode(0x002081B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[3], 4u);

    // addi x0, x0, 7  (write to x0 must be discarded)
    execute(decode(0x00700013), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[0], 0);
}

// signed vs unsigned: slti/sltiu and srai/srli differ for negative values
TEST(execute_signed_unsigned) {
    RegFile regs;
    FlatMemory mem(64);

    // addi x1, x0, -8
    execute(decode(0xFF800093), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[1], 0xFFFFFFF8u);

    // slti x2, x1, 5
    execute(decode(0x0050A113), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[2], 1u);

    // sltiu x3, x1, 5
    execute(decode(0x0050B193), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[3], 0u);

    // srai x4, x1, 1
    execute(decode(0x4010D213), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[4], 0xFFFFFFFCu);

    // srli x5, x1, 1
    execute(decode(0x0010D293), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[5], 0x7FFFFFFCu);
}

// register ops: sub, shift amount uses low 5 bits, slt vs sltu
TEST(execute_register_ops) {
    RegFile regs;
    FlatMemory mem(64);

    // addi x1, x0, 3
    execute(decode(0x00300093), regs, mem, no_csrs);
    // addi x2, x0, 35
    execute(decode(0x02300113), regs, mem, no_csrs);

    // sub x3, x1, x2
    execute(decode(0x402081B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[3], 0xFFFFFFE0u);

    // sll x4, x1, x2  (shift by 35 & 31)
    execute(decode(0x00209233), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[4], 24u);

    // slt x5, x3, x1
    execute(decode(0x0011A2B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[5], 1u);

    // sltu x6, x3, x1
    execute(decode(0x0011B333), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[6], 0u);
}

// branches: taken -> pc + offset, not taken -> pc + 4
TEST(execute_branches) {
    RegFile regs;
    FlatMemory mem(64);

    // addi x1, x0, -8
    execute(decode(0xFF800093), regs, mem, no_csrs);
    // addi x2, x0, 5
    execute(decode(0x00500113), regs, mem, no_csrs);

    // every branch below sits at address 0x100
    regs.pc = 0x100;

    // blt x1, x2, 8  
    // taken
    ExecResult r = execute(decode(0x0020C463), regs, mem, no_csrs);
    EXPECT_EQ(r.next_pc, 0x108u);

    // bltu x1, x2, 8 
    // not taken
    r = execute(decode(0x0020E463), regs, mem, no_csrs);
    EXPECT_EQ(r.next_pc, 0x104u);

    // bge x1, x1, 8
    // taken
    r = execute(decode(0x0010D463), regs, mem, no_csrs);
    EXPECT_EQ(r.next_pc, 0x108u);

    // bne x1, x2, -8
    // taken, backward branch
    r = execute(decode(0xFE209CE3), regs, mem, no_csrs);
    EXPECT_EQ(r.next_pc, 0xF8u);
}

// jumps: jal saves pc + 4, ret returns, jalr clears bit 0 and handles rd == rs1
TEST(execute_jumps) {
    RegFile regs;
    FlatMemory mem(64);

    // jal x1, 2052 at 0x100
    regs.pc = 0x100;
    ExecResult r = execute(decode(0x005000EF), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[1], 0x104u);
    EXPECT_EQ(r.next_pc, 0x904u);

    // ret (jalr x0, 0(x1)) at 0x904
    regs.pc = 0x904;
    r = execute(decode(0x00008067), regs, mem, no_csrs);
    EXPECT_EQ(r.next_pc, 0x104u);
    EXPECT_EQ(regs.x[0], 0u);

    // jalr x1, 1(x1) at 0x200  (rd == rs1, odd target)
    regs.pc = 0x200;
    r = execute(decode(0x001080E7), regs, mem, no_csrs);
    EXPECT_EQ(r.next_pc, 0x104u);
    EXPECT_EQ(regs.x[1], 0x204u);
}
