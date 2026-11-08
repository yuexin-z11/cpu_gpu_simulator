#pragma once
// basic types shared by every SoCSim component

#include <cstdint>

namespace socsim {

// RV32 byte address
using Addr  = std::uint32_t;
using Word  = std::uint32_t;
using Cycle = std::uint64_t;

// which processor issued a memory request
enum class Requester : std::uint8_t { cpu, gpu };

// one request as seen by the timing model (caches and DRAM)
// data is read and written through FlatMemory separately
struct MemRequest {
    Addr addr = 0;
    // bytes
    std::uint32_t size = 4;
    bool is_write      = false;
    Requester source   = Requester::cpu;
};

// fixed memory map. values must match programs/src/common/socsim.h
namespace mmio {
// LaunchPacket lives here
constexpr Addr kLaunchPacket = 0x0001'0000;
// CPU store starts the GPU
constexpr Addr kDoorbell = 0x0001'0100;
// GPU writes kFenceDone when finished
constexpr Addr kFence     = 0x0001'0104;
constexpr Word kFenceDone = 1;
}  // namespace mmio

// read-only CSRs that a GPU lane reads with `csrr`
namespace csr {
constexpr std::uint16_t kLaneId    = 0xCC0;
constexpr std::uint16_t kWarpId    = 0xCC1;
constexpr std::uint16_t kBlockSize = 0xCC2;
}  // namespace csr

// layout of the launch packet the host writes at mmio::kLaunchPacket
struct LaunchPacket {
    Addr kernel_entry       = 0;
    Addr args_ptr           = 0;
    std::uint32_t num_warps = 0;
};

}  // namespace socsim
