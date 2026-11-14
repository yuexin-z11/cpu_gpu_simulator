#include <string>
#include <vector>

#include "check.hpp"
#include "socsim/simulator.hpp"

using namespace socsim;

// host program at 0x0: write the launch packet, ring the doorbell,
// wait for the fence, then exit with a0 = out[7] - 7 (0 only if the GPU was right)
static const std::vector<Word> kHost = {
    // lui t0, 0x10  (t0 = 0x10000, the launch packet)
    0x000102B7,
    // addi t1, x0, 0x100 / sw t1, 0(t0)  (kernel entry)
    0x10000313, 0x0062A023,
    // lui t1, 0x3 / sw t1, 4(t0)  (args = output array at 0x3000)
    0x00003337, 0x0062A223,
    // addi t1, x0, 2 / sw t1, 8(t0)  (2 warps)
    0x00200313, 0x0062A423,
    // addi t1, x0, 1 / sw t1, 0x100(t0)  (ring the doorbell)
    0x00100313, 0x1062A023,
    // poll: lw t2, 0x104(t0) / beq t2, x0, poll  (wait for the fence)
    0x1042A383, 0xFE038EE3,
    // lui t3, 0x3 / lw a0, 0x1C(t3) / addi a0, a0, -7  (a0 = out[7] - 7)
    0x00003E37, 0x01CE2503, 0xFF950513,
    // addi a7, x0, 93 / ecall  (exit)
    0x05D00893, 0x00000073,
};

// kernel at 0x100: out[gid] = gid, where gid = warp_id * block_size + lane_id
static const std::vector<Word> kKernel = {
    // csrr t0, lane_id / csrr t1, warp_id / csrr t2, block_size
    0xCC0022F3, 0xCC102373, 0xCC2023F3,
    // mul t1, t1, t2 / add t0, t0, t1  (gid)
    0x02730333, 0x006282B3,
    // slli t3, t0, 2 / add t3, t3, a0 / sw t0, 0(t3)  (out[gid] = gid)
    0x00229E13, 0x00AE0E33, 0x005E2023,
    // ret
    0x00008067,
};

// the CPU launches a kernel, waits on the fence, and checks the result
TEST(simulator_cpu_launches_gpu_kernel) {
    // default chip: 16 lanes per warp
    Simulator sim(ChipConfig{});
    sim.load_words(0x100, kKernel, 0);
    sim.load_words(0x000, kHost, 0);

    const RunResult r = sim.run();

    // the host exited normally with a0 == 0
    EXPECT_EQ(r.exit_reason, std::string("exit"));
    EXPECT_EQ(r.exit_code, 0u);
    EXPECT_EQ(r.passed, true);

    // 2 warps x 16 lanes = 32 threads wrote out[0..31]
    // out[7] at 0x3000 + 4 * 7
    EXPECT_EQ(sim.memory().data().read32(0x301C), 7u);
    // the last thread (warp 1, lane 15)
    EXPECT_EQ(sim.memory().data().read32(0x307C), 31u);

    // 9 instructions per warp, no divergence
    EXPECT_EQ(r.gpu.warp_instructions, 18u);
    // each warp stores 16 x 4 B = 64 B, exactly one cache line
    EXPECT_EQ(r.gpu.cache_line_requests, 2u);
}
