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
