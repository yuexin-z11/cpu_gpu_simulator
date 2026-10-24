#pragma once
// Basic types shared by every SoCSim component.

#include <cstdint>

namespace socsim {

using Addr  = std::uint32_t;  // RV32 byte address
using Word  = std::uint32_t;
using Cycle = std::uint64_t;

// Which processor issued a memory request.
enum class Requester : std::uint8_t { Cpu, Gpu };

// One request as seen by the timing model (caches and DRAM).
// Data is read and written through FlatMemory separately.
struct MemRequest {
    Addr addr = 0;
    std::uint32_t size = 4;  // bytes
    bool is_write = false;
    Requester source = Requester::Cpu;
};

// Fixed memory map. Values must match programs/src/common/socsim.h.
namespace mmio {
constexpr Addr kLaunchPacket = 0x0001'0000;  // LaunchPacket lives here
constexpr Addr kDoorbell     = 0x0001'0100;  // CPU store starts the GPU
constexpr Addr kFence        = 0x0001'0104;  // GPU writes kFenceDone when finished
constexpr Word kFenceDone    = 1;
}  // namespace mmio

// Read-only CSRs that a GPU lane reads with `csrr`.
namespace csr {
constexpr std::uint16_t kLaneId    = 0xCC0;
constexpr std::uint16_t kWarpId    = 0xCC1;
constexpr std::uint16_t kBlockSize = 0xCC2;
}  // namespace csr

// Layout of the launch packet the host writes at mmio::kLaunchPacket.
struct LaunchPacket {
    Addr kernel_entry = 0;
    Addr args_ptr = 0;
    std::uint32_t num_warps = 0;
};

}  // namespace socsim
