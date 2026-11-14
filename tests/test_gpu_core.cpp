#include "check.hpp"
#include "socsim/event_log.hpp"
#include "socsim/gpu_core.hpp"

using namespace socsim;

// kernel: out[gid] = gid, where gid = warp_id * block_size + lane_id and out = a0
TEST(gpu_runs_kernel_and_writes_fence) {
    MemorySystem mem(ChipConfig{});
    EventLog log;
    // 2 warps of 4 lanes = 8 threads
    GpuConfig cfg;
    cfg.num_warps = 2;
    cfg.warp_size = 4;
    GpuCore gpu(cfg, mem, log);

    // csrr t0, lane_id / csrr t1, warp_id / csrr t2, block_size
    // mul t1, t1, t2 / add t0, t0, t1 / slli t3, t0, 2 / add t3, t3, a0
    // sw t0, 0(t3) / ret
    const Word kernel[] = {0xCC0022F3, 0xCC102373, 0xCC2023F3, 0x02730333, 0x006282B3,
                           0x00229E13, 0x00AE0E33, 0x005E2023, 0x00008067};
    // write the kernel at 0x100
    for (Addr i = 0; i < 9; ++i) mem.data().write32(0x100 + i * 4, kernel[i]);

    // launch packet: entry, args (the output array), number of warps
    mem.data().write32(mmio::kLaunchPacket + 0, 0x100);
    mem.data().write32(mmio::kLaunchPacket + 4, 0x3000);
    mem.data().write32(mmio::kLaunchPacket + 8, 2);

    // run until the kernel finishes (with a safety stop)
    gpu.launch(0);
    for (Cycle now = 0; gpu.busy() && now < 10000; ++now) gpu.step(now);

    // the kernel finished and told the CPU
    EXPECT_EQ(gpu.busy(), false);
    EXPECT_EQ(mem.data().read32(mmio::kFence), mmio::kFenceDone);

    // every thread wrote its own global index
    EXPECT_EQ(mem.data().read32(0x3000), 0u);
    EXPECT_EQ(mem.data().read32(0x3004), 1u);
    // warp 1, lane 0
    EXPECT_EQ(mem.data().read32(0x3010), 4u);
    // warp 1, lane 3 (the last thread)
    EXPECT_EQ(mem.data().read32(0x301C), 7u);

    // 9 instructions per warp, no divergence
    EXPECT_EQ(gpu.stats().warp_instructions, 18u);
    EXPECT_EQ(gpu.stats().active_lane_sum, 72u);  // 8 threads * 9 instructions
    // one sw per warp, and each warp's 4 words fit in one 64 B line
    EXPECT_EQ(gpu.stats().warp_mem_instructions, 2u);
    EXPECT_EQ(gpu.stats().cache_line_requests, 2u);
}
