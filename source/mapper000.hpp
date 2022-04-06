//
// Created by Noah Schonhorn on 4/3/22.
//

#ifndef CYGNES_MAPPER000_HPP
#define CYGNES_MAPPER000_HPP

#include "mapper.hpp"

class mapper000 : public mapper
{
  public:
    mapper000(int prg_banks, int chr_banks);
    auto cpu_read(uint16_t addr, int& mapped_addr) -> bool override;
    auto cpu_write(uint16_t addr, int& mapped_addr) -> bool override;
    auto ppu_read(uint16_t addr, int& mapped_addr) -> bool override;
    auto ppu_write(uint16_t addr, int& mapped_addr) -> bool override;
};

#endif  // CYGNES_MAPPER000_HPP
