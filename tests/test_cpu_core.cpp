#include "check.hpp"
#include "socsim/cpu_core.hpp"
#include "socsim/event_log.hpp"

using namespace socsim;

// a 5-instruction program: 6 * 7, then exit with a0 as the exit code
TEST(cpu_runs_program_and_exits) {
    MemorySystem mem(ChipConfig{});
    EventLog log;
    CpuCore cpu(CpuConfig{}, mem, log);

    // addi a0, zero, 6 / addi a1, zero, 7 / mul a0, a0, a1 / addi a7, zero, 93 / ecall
    const Word program[] = {0x00600513, 0x00700593, 0x02B50533, 0x05D00893, 0x00000073};
    // write the program at address 0
    for (Addr i = 0; i < 5; ++i) mem.data().write32(i * 4, program[i]);

    // start at 0, stack at the top of a 64 KB region
    cpu.reset(0, 0x10000);

    // call step once per cycle until the program exits (with a safety stop)
    Cycle now = 0;
    while (cpu.step(now) && now < 1000) ++now;

    EXPECT_EQ(cpu.halted(), true);
    EXPECT_EQ(cpu.exit_code(), 42u);
    EXPECT_EQ(cpu.stats().instructions, 5u);
    EXPECT_EQ(cpu.stats().cycles, 7u);
}
