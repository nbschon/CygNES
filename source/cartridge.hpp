#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "mapper.hpp"

class cartridge
{
    bool m_vertically_mirrored;

    const int prg_rom_bank_size = 0x4000;
    const int chr_rom_bank_size = 0x2000;

    std::vector<uint8_t> m_prg_rom;
    std::vector<uint8_t> m_chr_rom;

     std::shared_ptr<mapper> m_mapper;
    
public:
    cartridge();

    auto get_mirroring() const -> bool;

    auto open_rom_file(std::string rom_path) -> bool;
    auto cpu_read(uint16_t addr, uint8_t &byte) -> bool;
    auto cpu_write(uint16_t addr, uint8_t byte) -> bool;
    auto ppu_read(uint16_t addr, uint8_t &byte) -> bool;
    auto ppu_write(uint16_t addr, uint8_t byte) -> bool;
};