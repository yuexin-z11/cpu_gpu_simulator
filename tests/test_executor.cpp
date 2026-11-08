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
    EXPECT_EQ(regs.x[4], 0xFFFFFFF4u);

    // srli x5, x1, 1
    execute(decode(0x0010D293), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[5], 0x7FFFFFFCu);
}
