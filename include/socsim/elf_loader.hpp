#pragma once
// Loads a static RV32IM ELF file into FlatMemory.

#include <string>

#include "socsim/flat_memory.hpp"
#include "socsim/types.hpp"

namespace socsim {

struct LoadedProgram {
    Addr entry = 0;       // CPU start address (_start)
    Addr stack_top = 0;   // initial CPU sp
};

// Copies every PT_LOAD segment into `mem` and zero-fills .bss.
// Throws std::runtime_error if the file is not a 32-bit RISC-V executable.
LoadedProgram load_elf(const std::string& path, FlatMemory& mem);

}  // namespace socsim
