#include "socsim/cache.hpp"

#include <utility>

namespace socsim {

double CacheStats::hit_rate() const {
    const std::uint64_t total = hits + misses;
    return total == 0 ? 0.0 : static_cast<double>(hits) / static_cast<double>(total);
}

Cache::Cache(std::string name, const CacheConfig& cfg)
    : name_(std::move(name)), cfg_(cfg) {
    // TODO: num_sets_ = size_bytes / (line_bytes * ways); size lines_.
}

Addr Cache::line_addr(Addr addr) const {
    // TODO: clear the offset bits.
    return addr;
}

bool Cache::access(Addr addr, bool is_write) {
    // TODO: find the set, compare tags, update LRU, fill on miss,
    // and count hits/misses. Writes allocate (write-back timing is not
    // modelled).
    (void)addr;
    (void)is_write;
    return false;
}

}  // namespace socsim
