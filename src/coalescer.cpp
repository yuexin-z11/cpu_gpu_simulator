#include "socsim/coalescer.hpp"

namespace socsim {

std::vector<MemRequest> coalesce(const std::vector<MemRequest>& lane_requests,
                                 std::uint32_t line_bytes) {
    // TODO: map each address to its line (addr & ~(line_bytes - 1)), keep
    // the distinct lines in ascending order, one MemRequest per line.
    (void)line_bytes;
    return lane_requests;
}

}  // namespace socsim
