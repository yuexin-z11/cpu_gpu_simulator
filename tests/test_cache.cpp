#include "check.hpp"
#include "socsim/cache.hpp"
#include "socsim/dram.hpp"
#include "socsim/memory_system.hpp"

using namespace socsim;

TEST(cache_hit_rate_empty) {
    CacheStats s;
    EXPECT_EQ(s.hit_rate(), 0.0);
}

// tiny cache: 256 B, 64 B lines, 2 ways -> 2 sets
static CacheConfig tiny_cache() {
    CacheConfig cfg;
    cfg.size_bytes = 256;
    cfg.line_bytes = 64;
    cfg.ways       = 2;
    return cfg;
}

// line_addr clears the offset bits
TEST(cache_line_addr) {
    Cache c("l1", tiny_cache());
    EXPECT_EQ(c.line_addr(0x1234), 0x1200u);
    EXPECT_EQ(c.line_addr(0x0040), 0x0040u);
}

// first access misses, the same line then hits
TEST(cache_miss_then_hit) {
    Cache c("l1", tiny_cache());

    // cold miss
    EXPECT_EQ(c.access(0x000, false), false);
    // same line, different byte
    EXPECT_EQ(c.access(0x03C, false), true);
    // next line (set 1), cold miss
    EXPECT_EQ(c.access(0x040, false), false);

    EXPECT_EQ(c.stats().hits, 1u);
    EXPECT_EQ(c.stats().misses, 2u);
}

// 3 lines in a 2-way set: the least recently used one is evicted
TEST(cache_lru_eviction) {
    Cache c("l1", tiny_cache());

    // A and B fill both ways of set 0
    EXPECT_EQ(c.access(0x000, false), false);
    EXPECT_EQ(c.access(0x080, false), false);

    EXPECT_EQ(c.access(0x000, false), true);
    // C needs set 0, so one of A/B is evicted
    EXPECT_EQ(c.access(0x100, false), false);

    EXPECT_EQ(c.access(0x000, false), true);

    EXPECT_EQ(c.access(0x080, false), false);
}

// dram: latency overlaps, but transfers queue behind each other on the bus
TEST(dram_bandwidth_queueing) {
    Dram d(DramConfig{});

    // 64 B at cycle 0: bus 0-4, done 0 + 100 + 4
    EXPECT_EQ(d.access(0, 64), 104u);
    // another 64 B at cycle 0: waits for the bus until cycle 4
    EXPECT_EQ(d.access(0, 64), 108u);
    // 64 B at cycle 200: the bus has been idle, so no waiting
    EXPECT_EQ(d.access(200, 64), 304u);
    // 65 B at cycle 200: waits for the bus, and 65 B rounds up to 5 cycles
    EXPECT_EQ(d.access(200, 65), 309u);

    EXPECT_EQ(d.stats().requests, 4u);
    EXPECT_EQ(d.stats().bytes, 257u);
}

// memory system: L1 -> L2 -> DRAM latency, separate L1s, shared L2, MMIO bypass
TEST(memory_system_latency) {
    MemorySystem m(ChipConfig{});
    const MemRequest cpu_rd{0x2000, 4, false, Requester::cpu};
    const MemRequest gpu_rd{0x2000, 4, false, Requester::gpu};

    // cpu, cold: L1 miss + L2 miss + DRAM (1 + 1 + 100 + 4)
    EXPECT_EQ(m.access(1000, cpu_rd), 106u);
    // cpu again: L1 hit
    EXPECT_EQ(m.access(2000, cpu_rd), 1u);
    // gpu, same address: its own L1 misses, but the shared L2 has it
    EXPECT_EQ(m.access(3000, gpu_rd), 2u);
    // doorbell store skips the caches
    EXPECT_EQ(m.access(4000, MemRequest{mmio::kDoorbell, 4, true, Requester::cpu}), 1u);

    // counters
    EXPECT_EQ(m.cpu_l1().stats().hits, 1u);
    EXPECT_EQ(m.cpu_l1().stats().misses, 1u);
    EXPECT_EQ(m.gpu_l1().stats().misses, 1u);
    EXPECT_EQ(m.l2().stats().hits, 1u);
    EXPECT_EQ(m.l2().stats().misses, 1u);
    EXPECT_EQ(m.dram().stats().requests, 1u);
}