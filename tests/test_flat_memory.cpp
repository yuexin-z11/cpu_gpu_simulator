#include <cstdint>
#include <stdexcept>

#include "check.hpp"
#include "socsim/flat_memory.hpp"

using namespace socsim;

TEST(flat_memory_block_write) {
    FlatMemory mem(4);
    const std::uint8_t data[] = {0x12, 0x34};
    mem.write_block(2, data, 2);
    EXPECT_EQ(mem.read8(2), 0x12);
    EXPECT_EQ(mem.read8(3), 0x34);

    bool rejected = false;
    try {
        mem.write_block(3, data, 2);
    } catch (const std::out_of_range&) {
        rejected = true;
    }
    EXPECT(rejected);
    EXPECT_EQ(mem.read8(3), 0x34);
}

TEST(flat_memory_word_round_trip) {
    FlatMemory mem(16);
    mem.write32(4, 0x11223344);

    EXPECT_EQ(mem.read32(4), 0x11223344u);  // reads back the same value
    EXPECT_EQ(mem.read8(4), 0x44);          // low byte at the lowest address
    EXPECT_EQ(mem.read8(7), 0x11);          // high byte at the highest address
    EXPECT_EQ(mem.read16(4), 0x3344);       // lower half of the word
}
