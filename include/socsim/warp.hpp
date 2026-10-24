#pragma once
// A warp: `warp_size` lanes, each with its own registers and pc.
// Divergence rule: execute at the lowest pc among unfinished lanes;
// only lanes at that pc are active.

#include <cstdint>
#include <vector>

#include "socsim/executor.hpp"
#include "socsim/types.hpp"

namespace socsim {

class Warp {
public:
    Warp(std::uint32_t warp_id, std::uint32_t warp_size);

    // Sets every lane's pc to `entry`, a0 to `args_ptr`, ra to the
    // sentinel return address, and gives each lane its own stack.
    void launch(Addr entry, Addr args_ptr, Addr stack_base, std::uint32_t stack_bytes);

    bool done() const;
    Addr next_pc() const;                    // lowest pc among unfinished lanes
    std::vector<std::uint32_t> active_lanes() const;  // lanes at next_pc()
    std::uint32_t active_mask() const;       // bit i set = lane i active

    RegFile& lane(std::uint32_t i) { return lanes_[i]; }
    const RegFile& lane(std::uint32_t i) const { return lanes_[i]; }
    void finish_lane(std::uint32_t i) { finished_[i] = true; }

    std::uint32_t id() const { return id_; }
    std::uint32_t size() const { return static_cast<std::uint32_t>(lanes_.size()); }

    Cycle ready_at = 0;  // warp cannot issue before this cycle

    // A lane that returns to this address has finished the kernel.
    static constexpr Addr kExitSentinel = 0xFFFF'FFF0;

private:
    std::uint32_t id_;
    std::vector<RegFile> lanes_;
    std::vector<bool> finished_;
};

}  // namespace socsim
