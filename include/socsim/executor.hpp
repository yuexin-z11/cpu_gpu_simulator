#pragma once
// Executes one decoded instruction on one register file (a CPU or a GPU lane).

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

#include "socsim/decoder.hpp"
#include "socsim/flat_memory.hpp"
#include "socsim/types.hpp"

namespace socsim {

struct RegFile {
    std::array<Word, 32> x{};  // x[0] is always 0
    Addr pc = 0;
};

// What the instruction did, so the core can charge timing and react.
struct ExecResult {
    Addr next_pc = 0;
    std::optional<MemRequest> mem;  // set for loads and stores
    bool is_ecall = false;
    bool is_ebreak = false;
    bool is_fence = false;
};

// Returns the value of a CSR, or std::nullopt if the core does not allow it.
using CsrReader = std::function<std::optional<Word>(std::uint16_t csr)>;

// Updates the registers in `regs` and performs any load/store on `mem`.
// It does not change regs.pc; the caller applies ExecResult::next_pc.
// Throws on illegal instructions.
ExecResult execute(const DecodedInst& inst, RegFile& regs, FlatMemory& mem,
                   const CsrReader& read_csr);

}  // namespace socsim
