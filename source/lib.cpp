#include "lib.hpp"

library::library(std::string path)
    : name("CygNES")
{
    cpu CPU = cpu();
    std::shared_ptr<cartridge> cart = std::make_shared<cartridge>();

    if (cart->open_rom_file(path))
    {
        CPU.connect_cartridge(cart);
        CPU.reset();

        for (;;)
        {
            CPU.clock();
        }
    }
}
