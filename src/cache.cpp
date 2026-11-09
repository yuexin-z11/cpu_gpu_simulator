#include "socsim/cache.hpp"

#include <utility>

namespace socsim {

double CacheStats::hit_rate() const {
    const std::uint64_t total = hits + misses;
    return total == 0 ? 0.0 : static_cast<double>(hits) / static_cast<double>(total);
}

Cache::Cache(std::string name, const CacheConfig& cfg) : name_(std::move(name)), cfg_(cfg) {
    // sets = total size / bytes per set
    num_sets_ = cfg_.size_bytes / (cfg_.line_bytes * cfg_.ways);
    // one Line per way in every set, all start invalid
    lines_.resize(static_cast<std::size_t>(num_sets_) * cfg_.ways);
}

Addr Cache::line_addr(Addr addr) const {
    // clear the offset bits (line_bytes is a power of 2)
    return addr & ~static_cast<Addr>(cfg_.line_bytes - 1);
}

bool Cache::access(Addr addr, bool is_write) {
    // writes allocate like reads (write-back timing is not modelled)
    (void)is_write;
    ++use_counter_;

    // split the address into set index and tag
    const Addr line = addr / cfg_.line_bytes;
    const std::uint32_t set = line % 16;
    const Addr tag = line / 16;

    // first Line of this set
    Line* ways = &lines_[static_cast<std::size_t>(set) * cfg_.ways];

    // hit: a valid way with the same tag
    for (std::uint32_t w = 0; w < cfg_.ways; ++w) {
        if (ways[w].valid && ways[w].tag == tag) {
            ways[w].last_use = use_counter_;
            ++stats_.hits;
            return true;
        }
    }

    // miss: pick an empty way, otherwise the least recently used one
    ++stats_.________;
    std::uint32_t victim = 0;
    for (std::uint32_t w = 0; w < cfg_.ways; ++w) {
        if (!ways[w].valid) {
            victim = w;
            break;
        }
        if (ways[w].last_use < ways[victim].________) victim = w;
    }

    // fill the victim with the new line
    ways[victim].valid    = ________;
    ways[victim].tag      = tag;
    ways[victim].last_use = ________;
    return false;
}

}  // namespace socsim
