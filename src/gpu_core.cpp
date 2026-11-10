#include "socsim/gpu_core.hpp"

#include "socsim/event_log.hpp"

namespace socsim {

namespace {
// GPU lane stacks sit above the CPU stack (link.ld: __stack_top = 0x00F00000)
constexpr Addr kGpuStackBase = 0x00F0'0000;
// 1 KB per lane
constexpr std::uint32_t kLaneStackBytes = 0x400;
}  // namespace

GpuCore::GpuCore(const GpuConfig& cfg, MemorySystem& mem, EventLog& log)
    : cfg_(cfg), mem_(mem), log_(log) {}

void GpuCore::launch(Cycle now) {
    // read the launch packet the CPU wrote (3 words, in struct order)
    const FlatMemory& m = mem_.data();
    LaunchPacket pkt;
    pkt.kernel_entry = m.read32(mmio::kLaunchPacket + 0);
    pkt.args_ptr     = m.read32(mmio::kLaunchPacket + 4);
    pkt.num_warps    = m.read32(mmio::kLaunchPacket + 8);

    // one Warp per requested warp, each with warp_size lanes
    warps_.clear();
    for (std::uint32_t w = 0; w < pkt.num_warps; ++w) {
        warps_.emplace_back(w, cfg_.warp_size);
        // each warp gets its own block of lane stacks
        const Addr base = kGpuStackBase + w * cfg_.warp_size * kLaneStackBytes;
        warps_.back().launch(pkt.kernel_entry, pkt.args_ptr, base, kLaneStackBytes);
    }

    rr_next_      = 0;
    launch_cycle_ = now;
    stats_        = GpuStats{};
    // busy only if there is something to run
    busy_ = !warps_.empty();
}

std::uint32_t GpuCore::select_warp(Cycle now) {
    // TODO: starting at rr_next_, return the first warp that is not done
    // and has ready_at <= now
    (void)now;
    return static_cast<std::uint32_t>(warps_.size());
}

void GpuCore::issue(Warp& w, Cycle now) {
    // TODO: decode the instruction at w.next_pc(), execute it for each
    // active lane (CSR reader returns lane/warp/block ids), mark lanes that
    // jump to Warp::kExitSentinel as finished, coalesce memory requests,
    // charge latency to w.ready_at, update stats_ and log_
    (void)w;
    (void)now;
}

void GpuCore::step(Cycle now) {
    if (!busy_) return;
    const std::uint32_t i = select_warp(now);
    if (i < warps_.size()) issue(warps_[i], now);
    // TODO: when every warp is done, write mmio::kFenceDone to
    // mmio::kFence, set stats_.cycles, busy_ = false
}

}  // namespace socsim
