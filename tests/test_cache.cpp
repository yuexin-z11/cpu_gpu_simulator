#include "check.hpp"
#include "socsim/cache.hpp"
#include "socsim/dram.hpp"

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

// TODO:
// - DRAM bandwidth: back-to-back requests queue behind each other