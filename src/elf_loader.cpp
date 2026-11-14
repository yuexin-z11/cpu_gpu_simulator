#include "socsim/elf_loader.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace socsim {

namespace {
// ELF32 file header: the first 52 bytes of the file
struct Elf32Header {
    std::uint8_t ident[16];
    std::uint16_t type;
    std::uint16_t machine;
    std::uint32_t version;
    std::uint32_t entry;
    std::uint32_t phoff;
    std::uint32_t shoff;
    std::uint32_t flags;
    std::uint16_t ehsize;
    std::uint16_t phentsize;
    std::uint16_t phnum;
    std::uint16_t shentsize;
    std::uint16_t shnum;
    std::uint16_t shstrndx;
};
static_assert(sizeof(Elf32Header) == 52, "ELF32 header is 52 bytes");

// one program header: where a segment lives in the file and in memory
struct Elf32ProgramHeader {
    std::uint32_t type;
    std::uint32_t offset;
    std::uint32_t vaddr;
    std::uint32_t paddr;
    std::uint32_t filesz;
    std::uint32_t memsz;
    std::uint32_t flags;
    std::uint32_t align;
};
static_assert(sizeof(Elf32ProgramHeader) == 32, "ELF32 program header is 32 bytes");

// values from the ELF standard
constexpr std::uint8_t kElfClass32   = 1;
constexpr std::uint8_t kElfLittle    = 1;
constexpr std::uint16_t kTypeExec    = 2;
constexpr std::uint16_t kMachineRiscv = 243;
constexpr std::uint32_t kPtLoad      = 1;
// matches __stack_top in programs/src/common/link.ld
constexpr Addr kStackTop = 0x00F0'0000;
}  // namespace

LoadedProgram load_elf(const std::string& path, FlatMemory& mem) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open program: " + path);

    // read the 52-byte file header straight into the struct
    // (fine on little-endian hosts, which is every machine we run on)
    Elf32Header h{};
    if (!in.read(reinterpret_cast<char*>(&h), sizeof(h))) {
        throw std::runtime_error("file too short for an ELF header: " + path);
    }

    // magic number: 0x7F 'E' 'L' 'F'
    if (h.ident[0] != 0x7F || h.ident[1] != 'E' || h.ident[2] != 'L' || h.ident[3] != 'F') {
        throw std::runtime_error("not an ELF file: " + path);
    }
    // 32-bit, little-endian, RISC-V, executable
    if (h.ident[4] != kElfClass32 || h.ident[5] != kElfLittle || h.machine != kMachineRiscv
        || h.type != kTypeExec) {
        throw std::runtime_error("not a 32-bit little-endian RISC-V executable: " + path);
    }

    LoadedProgram prog;

    // copy every loadable segment into memory
    for (std::uint16_t i = 0; i < h.phnum; ++i) {
        // program header i sits at phoff + i * phentsize
        Elf32ProgramHeader ph{};
        in.seekg(h.phoff + static_cast<std::uint32_t>(i) * h.phentsize);
        if (!in.read(reinterpret_cast<char*>(&ph), sizeof(ph))) {
            throw std::runtime_error("truncated program header: " + path);
        }
        // only PT_LOAD segments go into memory
        if (ph.type != kPtLoad) continue;
        if (ph.filesz > ph.memsz) throw std::runtime_error("bad segment sizes: " + path);

        // file bytes: filesz bytes at offset go to vaddr
        std::vector<std::uint8_t> bytes(ph.filesz);
        in.seekg(ph.offset);
        if (!in.read(reinterpret_cast<char*>(bytes.data()),
                     static_cast<std::streamsize>(bytes.size()))) {
            throw std::runtime_error("truncated segment: " + path);
        }
        mem.write_block(ph.vaddr, bytes.data(), ph.filesz);

        // .bss: the rest of the segment, up to memsz, starts as zero
        for (std::uint32_t b = ph.filesz; b < ph.memsz; ++b) mem.write8(ph.vaddr + b, 0);
    }

    // the CPU starts at the ELF entry point (_start)
    prog.entry     = h.entry;
    prog.stack_top = kStackTop;
    return prog;
}

}  // namespace socsim