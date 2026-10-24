#include "socsim/gpu_core.hpp"

#include "socsim/event_log.hpp"

namespace socsim {

GpuCore::GpuCore(const GpuConfig& cfg, MemorySystem& mem, EventLog& log)
    : cfg_(cfg), mem_(mem), log_(log) {}

void GpuCore::launch(Cycle now) {
    // TODO: read the LaunchPacket at mmio::kLaunchPacket, create
    // num_warps Warps of cfg_.warp_size lanes, and call Warp::launch.
    launch_cycle_ = now;
    stats_ = GpuStats{};
    busy_ = false;  // set to true once warps are created
}

std::uint32_t GpuCore::select_warp(Cycle now) {
    // TODO: starting at rr_next_, return the first warp that is not done
    // and has ready_at <= now.
    (void)now;
    return static_cast<std::uint32_t>(warps_.size());
}

void GpuCore::issue(Warp& w, Cycle now) {
    // TODO: decode the instruction at w.next_pc(), execute it for each
    // active lane (CSR reader returns lane/warp/block ids), mark lanes that
    // jump to Warp::kExitSentinel as finished, coalesce memory requests,
    // charge latency to w.ready_at, update stats_ and log_.
    (void)w;
    (void)now;
}

void GpuCore::step(Cycle now) {
    if (!busy_) return;
    const std::uint32_t i = select_warp(now);
    if (i < warps_.size()) issue(warps_[i], now);
    // TODO: when every warp is done, write mmio::kFenceDone to
    // mmio::kFence, set stats_.cycles, busy_ = false.
}

}  // namespace socsim
