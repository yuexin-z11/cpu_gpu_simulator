#pragma once
// Functional memory: holds every byte of program data. No timing.

#include <cstdint>
#include <vector>

#include "socsim/types.hpp"

namespace socsim {

class FlatMemory {
public:
    explicit FlatMemory(std::uint32_t size_bytes = 16u << 20);

    std::uint8_t  read8(Addr a) const;
    std::uint16_t read16(Addr a) const;
    std::uint32_t read32(Addr a) const;
    void write8(Addr a, std::uint8_t v);
    void write16(Addr a, std::uint16_t v);
    void write32(Addr a, std::uint32_t v);

    // Bulk copy used by the ELF loader.
    void write_block(Addr a, const std::uint8_t* data, std::uint32_t n);

    std::uint32_t size() const { return static_cast<std::uint32_t>(bytes_.size()); }

private:
    void check(Addr a, std::uint32_t n) const;  // throws on out-of-range access
    std::vector<std::uint8_t> bytes_;
};

}  // namespace socsim
