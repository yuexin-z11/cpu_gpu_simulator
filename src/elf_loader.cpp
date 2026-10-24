#include "socsim/elf_loader.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>

namespace socsim {

LoadedProgram load_elf(const std::string& path, FlatMemory& mem) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open program: " + path);

    LoadedProgram prog;
    // TODO:
    //  1. Read the ELF header; check magic, ELFCLASS32, little-endian,
    //     e_machine == EM_RISCV (243), e_type == ET_EXEC.
    //  2. For each program header with p_type == PT_LOAD, copy p_filesz
    //     bytes to p_vaddr and zero-fill up to p_memsz.
    //  3. prog.entry = e_entry; choose stack_top near the top of memory.
    (void)mem;
    return prog;
}

}  // namespace socsim
