#pragma once
// a warp: `warp_size` lanes, each with its own registers and pc
// divergence rule: execute at the lowest pc among unfinished lanes;
// only lanes at that pc are active

#include <cstdint>
#include <vector>

#include "socsim/executor.hpp"
#include "socsim/types.hpp"

namespace socsim {

class Warp {
public:
    Warp(std::uint32_t warp_id, std::uint32_t warp_size);

    // sets every lane's pc to `entry`, a0 to `args_ptr`, ra to the
    // sentinel return address, and gives each lane its own stack
    void launch(Addr entry, Addr args_ptr, Addr stack_base, std::uint32_t stack_bytes);

    bool done() const;
    // lowest pc among unfinished lanes
    Addr next_pc() const;
    // lanes at next_pc()
    std::vector<std::uint32_t> active_lanes() const;
    // bit i set = lane i active
    std::uint32_t active_mask() const;

    RegFile& lane(std::uint32_t i) { return lanes_[i]; }
    const RegFile& lane(std::uint32_t i) const { return lanes_[i]; }
    void finish_lane(std::uint32_t i) { finished_[i] = true; }

    std::uint32_t id() const { return id_; }
    std::uint32_t size() const { return static_cast<std::uint32_t>(lanes_.size()); }

    // warp cannot issue before this cycle
    Cycle ready_at = 0;

    // a lane that returns to this address has finished the kernel
    static constexpr Addr kExitSentinel = 0xFFFF'FFF0;

private:
    std::uint32_t id_;
    std::vector<RegFile> lanes_;
    std::vector<bool> finished_;
};

}  // namespace socsim
