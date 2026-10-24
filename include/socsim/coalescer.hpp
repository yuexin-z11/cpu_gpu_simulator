#pragma once
// Groups the active lanes' addresses of one warp memory instruction into
// distinct cache-line requests.

#include <cstdint>
#include <vector>

#include "socsim/types.hpp"

namespace socsim {

// Returns one request per distinct cache line, in ascending address order
// so the result does not depend on lane order.
std::vector<MemRequest> coalesce(const std::vector<MemRequest>& lane_requests,
                                 std::uint32_t line_bytes);

}  // namespace socsim
