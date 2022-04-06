//
// Created by Noah Schonhorn on 4/1/22.
//

#ifndef CYGNES_MAPPER_HPP
#define CYGNES_MAPPER_HPP

#include <cstdint>

class mapper
{
  protected:
    int prg_banks, chr_banks;

  public:
    mapper(int prg_banks, int chr_banks);
    virtual auto cpu_read(uint16_t addr, int& mapped_addr) -> bool = 0;
    virtual auto cpu_write(uint16_t addr, int& mapped_addr) -> bool = 0;
    virtual auto ppu_read(uint16_t addr, int& mapped_addr) -> bool = 0;
    virtual auto ppu_write(uint16_t addr, int& mapped_addr) -> bool = 0;
};

#endif  // CYGNES_MAPPER_HPP
