#include "socsim/warp.hpp"

namespace socsim {

Warp::Warp(std::uint32_t warp_id, std::uint32_t warp_size)
    : id_(warp_id), lanes_(warp_size), finished_(warp_size, true) {}

void Warp::launch(Addr entry, Addr args_ptr, Addr stack_base, std::uint32_t stack_bytes) {
    // TODO: for each lane: pc = entry, a0 (x10) = args_ptr,
    // ra (x1) = kExitSentinel, sp (x2) = its own stack top; mark unfinished.
    (void)entry;
    (void)args_ptr;
    (void)stack_base;
    (void)stack_bytes;
}

bool Warp::done() const {
    for (bool f : finished_)
        if (!f) return false;
    return true;
}

Addr Warp::next_pc() const {
    // TODO: lowest pc among unfinished lanes.
    return 0;
}

std::vector<std::uint32_t> Warp::active_lanes() const {
    // TODO: unfinished lanes whose pc == next_pc().
    return {};
}

std::uint32_t Warp::active_mask() const {
    std::uint32_t mask = 0;
    for (std::uint32_t i : active_lanes()) mask |= 1u << i;
    return mask;
}

}  // namespace socsim
