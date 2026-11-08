#include "check.hpp"
#include "socsim/decoder.hpp"

using namespace socsim;

// i-type: positive immediate, checks opcode 0x13 and the rd field
TEST(decode_addi) {
    // addi x1, x0, 5  ->  0x00500093
    const DecodedInst d = decode(0x00500093);
    EXPECT(d.op == Op::addi);
    EXPECT_EQ(d.rd, 1);
    EXPECT_EQ(d.imm, 5);
}

// i-type: negative immediate, checks sign extension (would be 4095 if wrong)
TEST(decode_addi_negative) {
    // addi x1, x0, -1  ->  0xFFF00093
    const DecodedInst d = decode(0xFFF00093);
    EXPECT(d.op == Op::addi);
    EXPECT_EQ(d.imm, -1);
}

// shift: funct7 = 0x20 picks srai over srli, imm holds only shamt
TEST(decode_srai) {
    // srai x1, x2, 3  ->  0x40315093
    const DecodedInst d = decode(0x40315093);
    EXPECT(d.op == Op::srai);
    EXPECT_EQ(d.rd, 1);
    EXPECT_EQ(d.rs1, 2);
    // would be 0x403 if funct7 leaked into imm
    EXPECT_EQ(d.imm, 3);
}

// s-type: split immediate with a negative offset
TEST(decode_sw_negative) {
    // sw x5, -4(x2)  ->  0xFE512E23
    const DecodedInst d = decode(0xFE512E23);
    EXPECT(d.op == Op::sw);
    // base address register
    EXPECT_EQ(d.rs1, 2);
    // register holding the value to store
    EXPECT_EQ(d.rs2, 5);
    EXPECT_EQ(d.imm, -4);
}

// op_class groups ops for cpu cycle costs
TEST(op_class_groups) {
    EXPECT(op_class(Op::mul) == OpClass::mul);
    EXPECT(op_class(Op::lw) == OpClass::load);
    EXPECT(op_class(Op::beq) == OpClass::branch);
}

// b-type: forward branch with imm[11] = 0
// catches a wrong imm[11] shift (would add 2048)
TEST(decode_bne_forward) {
    // bne x1, x0, 8  ->  0x00009463
    const DecodedInst d = decode(0x00009463);
    EXPECT(d.op == Op::bne);
    EXPECT_EQ(d.rs1, 1);
    EXPECT_EQ(d.rs2, 0);
    EXPECT_EQ(d.imm, 8);
}

// b-type: backward branch (loop), checks the sign and imm[11]
TEST(decode_beq_backward) {
    // beq x1, x2, -8  ->  0xFE208CE3
    const DecodedInst d = decode(0xFE208CE3);
    EXPECT(d.op == Op::beq);
    EXPECT_EQ(d.rs1, 1);
    EXPECT_EQ(d.rs2, 2);
    EXPECT_EQ(d.imm, -8);
}

// j-type: jal offset that uses imm[11] and imm[10:1]
// catches wrong shifts for both pieces
TEST(decode_jal) {
    // jal x1, 2052  ->  0x005000EF
    const DecodedInst d = decode(0x005000EF);
    EXPECT(d.op == Op::jal);
    // jal writes the return address, so it has an rd
    EXPECT_EQ(d.rd, 1);
    EXPECT_EQ(d.imm, 2052);
}

// system: csrr reads the gpu lane id
// catches sign extension of the csr number (would be 0xFCC0)
TEST(decode_csrr_lane_id) {
    // csrr x5, 0xCC0  ->  0xCC0022F3
    const DecodedInst d = decode(0xCC0022F3);
    EXPECT(d.op == Op::csrrs);
    EXPECT_EQ(d.rd, 5);
    // csrr is csrrs with rs1 = x0
    EXPECT_EQ(d.rs1, 0);
    EXPECT_EQ(d.csr, 0xCC0);
}