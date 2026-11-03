#include "check.hpp"
#include "socsim/decoder.hpp"

using namespace socsim;

TEST(decode_addi) {
    // addi x1, x0, 5  ->  0x00500093
    const DecodedInst d = decode(0x00500093);
    EXPECT(d.op == Op::Addi);
    EXPECT_EQ(d.rd, 1);
    EXPECT_EQ(d.imm, 5);
}

TEST(decode_addi_negative) {
    // addi x1, x0, -1  ->  0xFFF00093
    const DecodedInst d = decode(0xFFF00093);
    EXPECT(d.op == Op::Addi);
    EXPECT_EQ(d.imm, -1);
}

TEST(decode_srai) {
    // srai x1, x2, 3  ->  0x40315093
    const DecodedInst d = decode(0x40315093);
    EXPECT(d.op == Op::Srai);
    EXPECT_EQ(d.rd, 1);
    EXPECT_EQ(d.rs1, 2);
    EXPECT_EQ(d.imm, 3);
}

TEST(decode_sw_negative) {
    // sw x5, -4(x2)  ->  0xFE512E23
    const DecodedInst d = decode(0xFE512E23);
    EXPECT(d.op == Op::Sw);
    EXPECT_EQ(d.rs1, 2);   // base address register
    EXPECT_EQ(d.rs2, 5);   // register holding the value to store
    EXPECT_EQ(d.imm, -4);
}

TEST(op_class_groups) {
    EXPECT(op_class(Op::Mul) == OpClass::Mul);
    EXPECT(op_class(Op::Lw) == OpClass::Load);
    EXPECT(op_class(Op::Beq) == OpClass::Branch);
}

// TODO: one test per instruction format (R, I, S, B, U, J), sign-extended
// immediates, M-extension edge cases, and csrr.
