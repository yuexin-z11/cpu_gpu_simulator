#include "check.hpp"
#include "socsim/decoder.hpp"

using namespace socsim;

TEST(decode_addi) {
    // addi x1, x0, 5  ->  0x00500093
    // TODO: enable once decode() is implemented.
    // const DecodedInst d = decode(0x00500093);
    // EXPECT(d.op == Op::Addi);
    // EXPECT_EQ(d.rd, 1);
    // EXPECT_EQ(d.imm, 5);
}

TEST(op_class_groups) {
    EXPECT(op_class(Op::Mul) == OpClass::Mul);
    EXPECT(op_class(Op::Lw) == OpClass::Load);
    EXPECT(op_class(Op::Beq) == OpClass::Branch);
}

// TODO: one test per instruction format (R, I, S, B, U, J), sign-extended
// immediates, M-extension edge cases, and csrr.
