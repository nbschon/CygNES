#include "lib.hpp"

library::library(std::string path)
    : name("CygNES")
{
    cpu CPU = cpu();
    ppu PPU = ppu();
    std::shared_ptr<cartridge> cart = std::make_shared<cartridge>();

    if (cart->open_rom_file(path))
    {
        CPU.connect_cartridge(cart);
        CPU.reset();

        PPU.connect_cartridge(cart);
        PPU.reset();

        for (;;)
        {
            CPU.clock();
        }
    }
}
