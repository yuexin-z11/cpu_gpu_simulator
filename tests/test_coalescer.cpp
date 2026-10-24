#include "check.hpp"
#include "socsim/coalescer.hpp"

using namespace socsim;

TEST(coalesce_contiguous_one_line) {
    // 16 lanes x 4 bytes inside one 64-byte line -> 1 request.
    // TODO: enable once coalesce() is implemented.
    // std::vector<MemRequest> reqs;
    // for (Addr i = 0; i < 16; ++i) reqs.push_back({0x1000 + 4 * i, 4, false, Requester::Gpu});
    // EXPECT_EQ(coalesce(reqs, 64).size(), 1u);
}

// TODO: strided access (one line per lane), unaligned, duplicate addresses.
