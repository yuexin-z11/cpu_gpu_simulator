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

// loads: size, sign extension, and little-endian byte order
TEST(execute_loads) {
    RegFile regs;
    FlatMemory mem(64);
    mem.write32(0x10, 0x12348AF0);

    // addi x1, x0, 16  (base address 0x10)
    execute(decode(0x01000093), regs, mem, no_csrs);

    // lb x2, 0(x1)  (byte 0xF0, top bit is 1)
    execute(decode(0x00008103), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[2], 0xFFFFFFF0u);

    // lbu x3, 0(x1)
    execute(decode(0x0000C183), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[3], 0x000000F0u);

    // lh x4, 0(x1)  (halfword = bytes 0x11 and 0x10)
    execute(decode(0x00009203), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[4], 0xFFFF8AF0u);

    // lhu x5, 0(x1)
    execute(decode(0x0000D283), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[5], 0x00008AF0u);

    // lb x7, 3(x1)  (byte at 0x13, top bit is 0)
    execute(decode(0x00308383), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[7], 0x00000012u);

    // lw x6, 0(x1)
    ExecResult r = execute(decode(0x0000A303), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[6], 0x12348AF0u);
    // the timing request
    EXPECT_EQ(r.mem->addr, 0x10u);
    EXPECT_EQ(r.mem->size, 4u);
    EXPECT_EQ(r.mem->is_write, false);
}

// stores: only the low bytes of rs2 are written, little-endian
TEST(execute_stores) {
    RegFile regs;
    FlatMemory mem(64);

    // addi x1, x0, 32  (base address 0x20)
    execute(decode(0x02000093), regs, mem, no_csrs);
    // addi x2, x0, -2
    execute(decode(0xFFE00113), regs, mem, no_csrs);
    // addi x3, x0, 0x123
    execute(decode(0x12300193), regs, mem, no_csrs);

    // sw x2, 0(x1)
    execute(decode(0x0020A023), regs, mem, no_csrs);
    EXPECT_EQ(mem.read32(0x20), 0xFFFFFFFEu);

    // sh x2, 8(x1)  (only the low 16 bits of x2)
    execute(decode(0x00209423), regs, mem, no_csrs);
    EXPECT_EQ(mem.read32(0x28), 0x0000FFFEu);

    // sb x3, 4(x1)  (only the low 8 bits of x3)
    ExecResult r = execute(decode(0x00308223), regs, mem, no_csrs);
    EXPECT_EQ(mem.read32(0x24), 0x00000023u);
    // the byte after it is untouched
    EXPECT_EQ(mem.read8(0x25), 0u);
    // the timing request
    EXPECT_EQ(r.mem->addr, 0x24u);
    EXPECT_EQ(r.mem->size, 1u);
    EXPECT_EQ(r.mem->is_write, true);
}

// multiply: low half, and high half for signed/unsigned mixes
TEST(execute_multiply) {
    RegFile regs;
    FlatMemory mem(64);

    // addi x1, x0, -2  (0xFFFFFFFE)
    execute(decode(0xFFE00093), regs, mem, no_csrs);
    // addi x2, x0, -1  (0xFFFFFFFF)
    execute(decode(0xFFF00113), regs, mem, no_csrs);

    // mul x3, x1, x2  (-2 * -1 = 2, low 32 bits)
    execute(decode(0x022081B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[3], 0x00000002u);

    // mulh x4, x1, x2  (signed * signed, high 32 bits)
    execute(decode(0x02209233), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[4], 0x00000000u);

    // mulhsu x5, x1, x2  (signed -2 * unsigned 0xFFFFFFFF, high 32 bits)
    execute(decode(0x0220A2B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[5], 0xFFFFFFFEu);

    // mulhu x6, x1, x2  (unsigned 0xFFFFFFFE * 0xFFFFFFFF, high 32 bits)
    execute(decode(0x0220B333), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[6], 0xFFFFFFFDu);
}

// divide: rounds toward zero, div by zero, and signed overflow
TEST(execute_divide) {
    RegFile regs;
    FlatMemory mem(64);

    // addi x1, x0, -7  (0xFFFFFFF9)
    execute(decode(0xFF900093), regs, mem, no_csrs);
    // addi x2, x0, 2
    execute(decode(0x00200113), regs, mem, no_csrs);

    // div x3, x1, x2  (-7 / 2 rounds toward zero)
    execute(decode(0x0220C1B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[3], 0xFFFFFFFDu);

    // rem x4, x1, x2  (sign follows rs1)
    execute(decode(0x0220E233), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[4], 0xFFFFFFFFu);

    // divu x5, x1, x2  (0xFFFFFFF9 / 2 as unsigned)
    execute(decode(0x0220D2B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[5], 0x7FFFFFFCu);

    // remu x6, x1, x2
    execute(decode(0x0220F333), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[6], 0x00000001u);

    // div x7, x1, x0  (divide by zero)
    execute(decode(0x0200C3B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[7], 0xFFFFFFFFu);

    // rem x8, x1, x0  (divide by zero)
    execute(decode(0x0200E433), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[8], 0xFFFFFFF9u);

    // lui x9, 0x80000  (INT32_MIN)
    execute(decode(0x800004B7), regs, mem, no_csrs);
    // addi x10, x0, -1
    execute(decode(0xFFF00513), regs, mem, no_csrs);

    // div x11, x9, x10  (overflow)
    execute(decode(0x02A4C5B3), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[11], 0x80000000u);

    // rem x12, x9, x10  (overflow)
    execute(decode(0x02A4E633), regs, mem, no_csrs);
    EXPECT_EQ(regs.x[12], 0x00000000u);
}

// system: ecall/ebreak/fence set flags, csrr reads a CSR, illegal ops throw
TEST(execute_system) {
    RegFile regs;
    FlatMemory mem(64);

    // csr reader for a GPU lane: only lane id 7 is allowed
    auto lane_csrs = [](std::uint16_t c) -> std::optional<Word> {
        if (c == csr::kLaneId) return 7u;
        return std::nullopt;
    };

    // ecall
    ExecResult r = execute(decode(0x00000073), regs, mem, no_csrs);
    EXPECT_EQ(r.is_ecall, true);
    EXPECT_EQ(r.is_ebreak, false);

    // ebreak
    r = execute(decode(0x00100073), regs, mem, no_csrs);
    EXPECT_EQ(r.is_ebreak, true);

    // fence
    r = execute(decode(0x0FF0000F), regs, mem, no_csrs);
    EXPECT_EQ(r.is_fence, true);

    // csrr x5, 0xCC0  (lane id)
    execute(decode(0xCC0022F3), regs, mem, lane_csrs);
    EXPECT_EQ(regs.x[5], 7u);

    // the same csrr on a CPU (no CSRs) is illegal
    EXPECT_THROW(execute(decode(0xCC0022F3), regs, mem, no_csrs));

    // csrrw x5, 0xCC0, x1  (writes a CSR, not supported)
    EXPECT_THROW(execute(decode(0xCC0092F3), regs, mem, lane_csrs));

    // all zeros is not a valid instruction
    EXPECT_THROW(execute(decode(0x00000000), regs, mem, no_csrs));
}