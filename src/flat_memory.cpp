#include "socsim/flat_memory.hpp"

#include <stdexcept>
#include <string>

namespace socsim {

FlatMemory::FlatMemory(std::uint32_t size_bytes) : bytes_(size_bytes, 0) {}

// throws if any of the n bytes starting at address a lies outside memory
void FlatMemory::check(Addr a, std::uint32_t n) const {
    // add in 64 bits
    const std::uint64_t end = static_cast<std::uint64_t>(a) + n;

    // Valid addresses are 0 .. size() - 1
    if (end > bytes_.size()) {
        throw std::out_of_range("memory access out of range: address " +
                                std::to_string(a) + ", " + std::to_string(n) +
                                " bytes, memory size " + std::to_string(bytes_.size()));
    }
}

std::uint8_t FlatMemory::read8(Addr a) const {
    check(a, 1);
    return bytes_[a];
}

std::uint16_t FlatMemory::read16(Addr a) const {
    // little-endian
    check(a, 2);
    return static_cast<std::uint16_t>(bytes_[a+1] << 8 | bytes_[a]);
}

std::uint32_t FlatMemory::read32(Addr a) const {
    // little-endian
    check(a, 4);
    return  static_cast<std::uint32_t>(bytes_[a])
         | (static_cast<std::uint32_t>(bytes_[a + 1]) << 8)
         | (static_cast<std::uint32_t>(bytes_[a + 2]) << 16)
         | (static_cast<std::uint32_t>(bytes_[a + 3]) << 24);
}

void FlatMemory::write8(Addr a, std::uint8_t v) {
    check(a, 1);
    bytes_[a] = v;
}

void FlatMemory::write16(Addr a, std::uint16_t v) {
    // little-endian
    check(a, 2);
    bytes_[a] = v & 0xFF;
    bytes_[a+1] = v >> 8;
}

void FlatMemory::write32(Addr a, std::uint32_t v) {
    // little-endian
    check(a, 4);
    bytes_[a] = v & 0xFF;
    bytes_[a+1] = v >> 8;
    bytes_[a+2] = v >> 16;
    bytes_[a+3] = v >> 24;
}

void FlatMemory::write_block(Addr a, const std::uint8_t* data, std::uint32_t n) {
    // TODO
    (void)a;
    (void)data;
    (void)n;
}

}  // namespace socsim
