#pragma once
// loads a static RV32IM ELF file into FlatMemory

#include <string>

#include "socsim/flat_memory.hpp"
#include "socsim/types.hpp"

namespace socsim {

struct LoadedProgram {
    // CPU start address (_start)
    Addr entry = 0;
    // initial CPU sp
    Addr stack_top = 0;
};

// copies every PT_LOAD segment into `mem` and zero-fills .bss
// throws std::runtime_error if the file is not a 32-bit RISC-V executable
LoadedProgram load_elf(const std::string& path, FlatMemory& mem);

}  // namespace socsim
