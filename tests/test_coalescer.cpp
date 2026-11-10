#include "check.hpp"
#include "socsim/coalescer.hpp"

using namespace socsim;

// 16 gpu lanes reading addr = base + stride * lane
static std::vector<MemRequest> lanes(Addr base, Addr stride) {
    std::vector<MemRequest> reqs;
    for (Addr i = 0; i < 16; ++i) reqs.push_back({base + stride * i, 4, false, Requester::gpu});
    return reqs;
}

// 16 lanes x 4 bytes inside one 64-byte line -> 1 request
TEST(coalesce_contiguous_one_line) {
    const std::vector<MemRequest> out = coalesce(lanes(0x1000, 4), 64);
    EXPECT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0].addr, 0x1000u);
    EXPECT_EQ(out[0].size, 64u);
    EXPECT_EQ(out[0].source, Requester::gpu);
}

// stride of a full line: every lane gets its own line
TEST(coalesce_strided) {
    const std::vector<MemRequest> out = coalesce(lanes(0x1000, 64), 64);
    EXPECT_EQ(out.size(), 16u);
    EXPECT_EQ(out[0].addr, 0x1000u);
    // the last lane's line
    EXPECT_EQ(out.back().addr, 0x13C0u);
}

// stride 8: 16 lanes span 128 bytes -> 2 lines
TEST(coalesce_half_line_stride) {
    EXPECT_EQ(coalesce(lanes(0x1000, 8), 64).size(), 2u);
}

// duplicates merge, and the output is sorted no matter the lane order
TEST(coalesce_duplicates_sorted) {
    const std::vector<MemRequest> reqs = {{0x2044, 4, false, Requester::gpu},
                                          {0x2000, 4, false, Requester::gpu},
                                          {0x2040, 4, false, Requester::gpu},
                                          {0x2000, 4, false, Requester::gpu}};
    const std::vector<MemRequest> out = coalesce(reqs, 64);
    EXPECT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0].addr, 0x2000u);
    EXPECT_EQ(out[1].addr, 0x2040u);
}

// no active lanes -> no requests
TEST(coalesce_empty) {
    EXPECT_EQ(coalesce({}, 64).size(), 0u);
}