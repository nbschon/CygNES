//
// Created by Noah Schonhorn on 4/3/22.
//

#include "mapper000.hpp"
mapper000::mapper000(int prg_banks, int chr_banks)
    : mapper(prg_banks, chr_banks)
{

}

auto mapper000::cpu_read(uint16_t addr, int& mapped_addr) -> bool
{
    bool success = false;

    if (addr >= 0x8000 and addr <= 0xFFFF)
    {
        if (prg_banks == 1)
        {
            mapped_addr = (addr & 0x3FFF);
        }
        else
        {
            mapped_addr = (addr & 0x7FFF);
        }

        success = true;
    }

    return success;
}

auto mapper000::cpu_write(uint16_t addr, int& mapped_addr) -> bool
{
    bool success = false;

    return success;
}

auto mapper000::ppu_read(uint16_t addr, int& mapped_addr) -> bool
{
    bool success = false;

    if (addr >= 0x0000 and addr <= 0x1FFF)
    {
        mapped_addr = addr;
        success = true;
    }

    return success;
}

auto mapper000::ppu_write(uint16_t addr, int& mapped_addr) -> bool
{
    bool success = false;

    return success;
}
