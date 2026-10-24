#include "check.hpp"
#include "socsim/cache.hpp"
#include "socsim/dram.hpp"

using namespace socsim;

TEST(cache_hit_rate_empty) {
    CacheStats s;
    EXPECT_EQ(s.hit_rate(), 0.0);
}

// TODO:
//  - first access misses, second access to the same line hits
//  - LRU eviction when ways + 1 lines map to one set
//  - DRAM bandwidth: back-to-back requests queue behind each other
