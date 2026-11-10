#include "check.hpp"
#include "socsim/warp.hpp"

using namespace socsim;

// launch gives every lane the same pc and args, but its own stack
TEST(warp_launch) {
    Warp w(0, 4);
    // entry 0x100, args at 0x2000, stacks from 0x8000, 1 KB each
    w.launch(0x100, 0x2000, 0x8000, 0x400);

    EXPECT_EQ(w.lane(2).pc, 0x100u);
    EXPECT_EQ(w.lane(2).x[10], 0x2000u);
    EXPECT_EQ(w.lane(2).x[1], Warp::kExitSentinel);
    // lane 2's stack top
    EXPECT_EQ(w.lane(2).x[2], 0x8C00u);
    EXPECT_EQ(w.done(), false);
}

// divergence: lowest pc runs first, then the warp reconverges
TEST(warp_divergence) {
    Warp w(0, 4);
    w.launch(0x100, 0x2000, 0x8000, 0x400);

    // all lanes together
    EXPECT_EQ(w.next_pc(), 0x100u);
    EXPECT_EQ(w.active_mask(), 0xFu);

    // lanes 1 and 3 take a branch to 0x120
    w.lane(1).pc = 0x120;
    w.lane(3).pc = 0x120;
    EXPECT_EQ(w.next_pc(), 0x100u);
    EXPECT_EQ(w.active_mask(), 0x5u);

    // lanes 0 and 2 finish the kernel
    w.finish_lane(0);
    w.finish_lane(2);
    EXPECT_EQ(w.next_pc(), 0x120u);
    EXPECT_EQ(w.active_mask(), 0xAu);

    // the rest finish too
    w.finish_lane(1);
    w.finish_lane(3);
    EXPECT_EQ(w.done(), true);
}
