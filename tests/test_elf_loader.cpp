#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "check.hpp"
#include "socsim/elf_loader.hpp"

using namespace socsim;

// little-endian helpers for building a file by hand
static void put16(std::vector<std::uint8_t>& v, std::size_t at, std::uint16_t x) {
    v[at]     = static_cast<std::uint8_t>(x);
    v[at + 1] = static_cast<std::uint8_t>(x >> 8);
}
static void put32(std::vector<std::uint8_t>& v, std::size_t at, std::uint32_t x) {
    for (std::size_t i = 0; i < 4; ++i) v[at + i] = static_cast<std::uint8_t>(x >> (8 * i));
}

// writes `bytes` to a temp file and returns its path
static std::string write_temp(const std::string& name, const std::vector<std::uint8_t>& bytes) {
    const std::string path = (std::filesystem::temp_directory_path() / name).string();
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()),
              static_cast<std::streamsize>(bytes.size()));
    return path;
}

// smallest valid RV32 executable: one PT_LOAD segment at 0x1000
// with 8 bytes from the file and 8 more bytes of .bss
static std::vector<std::uint8_t> tiny_elf() {
    // file header (52) + one program header (32) + segment data (8)
    std::vector<std::uint8_t> f(52 + 32 + 8, 0);

    // e_ident: magic, 32-bit, little-endian, version 1
    f[0] = 0x7F;
    f[1] = 'E';
    f[2] = 'L';
    f[3] = 'F';
    f[4] = 1;
    f[5] = 1;
    f[6] = 1;
    // e_type = executable, e_machine = RISC-V, e_version = 1
    put16(f, 16, 2);
    put16(f, 18, 243);
    put32(f, 20, 1);
    // e_entry, e_phoff (program headers right after this header)
    put32(f, 24, 0x1000);
    put32(f, 28, 52);
    // e_ehsize, e_phentsize, e_phnum
    put16(f, 40, 52);
    put16(f, 42, 32);
    put16(f, 44, 1);

    // program header: PT_LOAD, file offset 84, loaded at 0x1000
    put32(f, 52, 1);
    put32(f, 56, 84);
    put32(f, 60, 0x1000);
    put32(f, 64, 0x1000);
    // p_filesz = 8, p_memsz = 16 (the last 8 bytes are .bss)
    put32(f, 68, 8);
    put32(f, 72, 16);
    // p_flags = read + execute, p_align = 4
    put32(f, 76, 5);
    put32(f, 80, 4);

    // segment data: addi x1, x0, 5 / ecall
    put32(f, 84, 0x00500093);
    put32(f, 88, 0x00000073);
    return f;
}

// file bytes land at vaddr, .bss is zeroed, entry and stack come back
TEST(elf_loads_segment_and_bss) {
    FlatMemory mem(1u << 20);
    // garbage where .bss will go, so the test can see it get zeroed
    mem.write32(0x100C, 0xDEADBEEF);

    const LoadedProgram p = load_elf(write_temp("socsim_tiny.elf", tiny_elf()), mem);
    EXPECT_EQ(p.entry, 0x1000u);
    EXPECT_EQ(p.stack_top, 0x00F00000u);

    // the 8 file bytes, copied to vaddr
    EXPECT_EQ(mem.read32(0x1000), 0x00500093u);
    EXPECT_EQ(mem.read32(0x1004), 0x00000073u);
    // .bss, zero-filled up to memsz
    EXPECT_EQ(mem.read32(0x100C), 0x00000000u);
}

// missing files and files that are not RV32 executables throw
TEST(elf_rejects_bad_files) {
    FlatMemory mem(1u << 20);

    // file does not exist
    EXPECT_THROW(load_elf("/nonexistent/socsim.elf", mem));

    // wrong magic number
    std::vector<std::uint8_t> f = tiny_elf();
    f[1] = 'X';
    EXPECT_THROW(load_elf(write_temp("socsim_bad_magic.elf", f), mem));

    // an x86-64 machine number (62) instead of RISC-V
    f = tiny_elf();
    put16(f, 18, 62);
    EXPECT_THROW(load_elf(write_temp("socsim_x86.elf", f), mem));
}
