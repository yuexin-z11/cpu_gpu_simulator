#include "socsim/coalescer.hpp"

#include <set>

namespace socsim {

std::vector<MemRequest> coalesce(const std::vector<MemRequest>& lane_requests,
                                 std::uint32_t line_bytes) {
    std::vector<MemRequest> out;
    if (lane_requests.empty()) return out;

    // distinct line addresses (std::set drops duplicates and keeps them sorted)
    std::set<Addr> lines;
    for (const MemRequest& r : lane_requests) {
        // clear the offset bits, same as Cache::line_addr
        lines.insert(r.addr & ~static_cast<Addr>(line_bytes - 1));
    }

    // every lane of one instruction has the same direction and source
    const bool is_write = lane_requests.front().is_write;
    const Requester src = lane_requests.front().source;

    // one request per line, each a full line in size
    for (Addr line : lines) out.push_back(MemRequest{line, line_bytes, is_write, src});
    return out;
}

}  // namespace socsim