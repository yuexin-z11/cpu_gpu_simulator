#include "socsim/gpu_core.hpp"

#include <stdexcept>

#include "socsim/coalescer.hpp"
#include "socsim/decoder.hpp"
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
    const std::uint32_t n = static_cast<std::uint32_t>(warps_.size());
    for (std::uint32_t k = 0; k < n; ++k) {
        // start at rr_next_ and wrap around the end
        const std::uint32_t i = (rr_next_ + k) % n;
        const Warp& w         = warps_[i];
        // skip finished warps and warps still waiting on memory
        if (w.done() || w.ready_at > now) continue;
        // next search starts after this warp
        rr_next_ = (i + 1) % n;
        return i;
    }
    // no warp can issue this cycle
    return n;
}

void GpuCore::issue(Warp& w, Cycle now) {
    // every active lane runs the same instruction
    const Addr pc                           = w.next_pc();
    const std::vector<std::uint32_t> active = w.active_lanes();
    const DecodedInst inst                  = decode(mem_.data().read32(pc));

    // memory requests from this instruction, one per lane, before coalescing
    std::vector<MemRequest> lane_reqs;

    for (std::uint32_t lane : active) {
        RegFile& r = w.lane(lane);

        // each lane sees its own identity through the read-only CSRs
        auto csrs = [&](std::uint16_t c) -> std::optional<Word> {
            if (c == csr::kLaneId) return lane;
            if (c == csr::kWarpId) return w.id();
            if (c == csr::kBlockSize) return cfg_.warp_size;
            return std::nullopt;
        };

        const ExecResult res = execute(inst, r, mem_.data(), csrs);
        // the GPU has no operating system or memory ordering instructions
        if (res.is_ecall || res.is_ebreak || res.is_fence) {
            throw std::runtime_error("ecall/ebreak/fence not supported on the GPU");
        }
        if (res.mem) {
            MemRequest q = *res.mem;
            q.source     = Requester::gpu;
            lane_reqs.push_back(q);
        }

        // move this lane on; returning to the sentinel ends the lane
        r.pc = res.next_pc;
        if (r.pc == Warp::kExitSentinel) w.finish_lane(lane);
    }

    // part B: timing and stats go here
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
