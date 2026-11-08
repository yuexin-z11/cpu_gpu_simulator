#pragma once
// executes one decoded instruction on one register file (a CPU or a GPU lane)

#include <array>
#include <cstdint>
#include <functional>
#include <optional>

#include "socsim/decoder.hpp"
#include "socsim/flat_memory.hpp"
#include "socsim/types.hpp"

namespace socsim {

struct RegFile {
    // x[0] is always 0
    std::array<Word, 32> x{};
    Addr pc = 0;
};

// what the instruction did, so the core can charge timing and react
struct ExecResult {
    Addr next_pc = 0;
    // set for loads and stores
    std::optional<MemRequest> mem;
    bool is_ecall  = false;
    bool is_ebreak = false;
    bool is_fence  = false;
};

// returns the value of a CSR, or std::nullopt if the core does not allow it
using CsrReader = std::function<std::optional<Word>(std::uint16_t csr)>;

// updates the registers in `regs` and performs any load/store on `mem`
// it does not change regs.pc; the caller applies ExecResult::next_pc
// throws on illegal instructions
ExecResult execute(const DecodedInst& inst, RegFile& regs, FlatMemory& mem,
                   const CsrReader& read_csr);

}  // namespace socsim
