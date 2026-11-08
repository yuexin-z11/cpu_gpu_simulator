#pragma once
// set-associative cache that tracks tags only, to decide hit or miss

#include <cstdint>
#include <string>
#include <vector>

#include "socsim/config.hpp"
#include "socsim/types.hpp"

namespace socsim {

struct CacheStats {
    std::uint64_t hits   = 0;
    std::uint64_t misses = 0;
    double hit_rate() const;
};

class Cache {
public:
    Cache(std::string name, const CacheConfig& cfg);

    // looks up the line holding `addr`. updates LRU state and fills the
    // line on a miss. returns true on a hit
    bool access(Addr addr, bool is_write);

    Addr line_addr(Addr addr) const;
    const CacheConfig& config() const { return cfg_; }
    const CacheStats& stats() const { return stats_; }
    const std::string& name() const { return name_; }

private:
    struct Line {
        bool valid = false;
        Addr tag   = 0;
        // for LRU
        std::uint64_t last_use = 0;
    };

    std::string name_;
    CacheConfig cfg_;
    std::uint32_t num_sets_ = 0;
    // num_sets_ * cfg_.ways
    std::vector<Line> lines_;
    std::uint64_t use_counter_ = 0;
    CacheStats stats_;
};

}  // namespace socsim
