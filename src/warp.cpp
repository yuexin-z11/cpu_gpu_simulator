#include "socsim/warp.hpp"

namespace socsim {

Warp::Warp(std::uint32_t warp_id, std::uint32_t warp_size)
    : id_(warp_id), lanes_(warp_size), finished_(warp_size, true) {}

void Warp::launch(Addr entry, Addr args_ptr, Addr stack_base, std::uint32_t stack_bytes) {
    for (std::uint32_t i = 0; i < size(); ++i) {
        // fresh registers for every lane
        RegFile& r = lanes_[i];
        r          = RegFile{};
        r.pc       = entry;
        // a0 = kernel arguments
        r.x[10] = args_ptr;
        // ra = sentinel, so `ret` from the kernel marks the lane finished
        r.x[1] = kExitSentinel;
        // sp = top of this lane's own stack (stacks grow down)
        r.x[2] = stack_base + (i + 1) * stack_bytes;
        finished_[i] = false;
    }
    ready_at = 0;
}

bool Warp::done() const {
    for (bool f : finished_)
        if (!f) return false;
    return true;
}

Addr Warp::next_pc() const {
    // start from the largest address, then take the minimum
    Addr best = ~Addr{0};
    for (std::uint32_t i = 0; i < size(); ++i) {
        // finished lanes do not count
        if (finished_[i]) continue;
        if (lanes_[i].pc < best) best = lanes_[i].pc;
    }
    return best;
}

std::vector<std::uint32_t> Warp::active_lanes() const {
    std::vector<std::uint32_t> out;
    const Addr pc = next_pc();
    for (std::uint32_t i = 0; i < size(); ++i) {
        // active = unfinished and at the chosen pc
        if (!finished_[i] && lanes_[i].pc == pc) out.push_back(i);
    }
    return out;
}

std::uint32_t Warp::active_mask() const {
    std::uint32_t mask = 0;
    for (std::uint32_t i : active_lanes())
        mask |= 1u << i;
    return mask;
}

}  // namespace socsim
