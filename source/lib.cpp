#include "lib.hpp"

library::library(std::string path)
    : name("CygNES")
{
    cpu CPU = cpu();
//    ppu PPU = ppu();
    std::shared_ptr<cartridge> cart = std::make_shared<cartridge>();

    if (cart->open_rom_file(path))
    {
        CPU.connect_cartridge(cart);
        CPU.reset();

        SDL_Event e;
        bool quit = false;

        while (!quit)
        {
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                else if (e.type == SDL_KEYDOWN)
                {
                    switch (e.key.keysym.sym)
                    {
                        case SDLK_q:
                        case SDLK_ESCAPE:
                            quit = true;
                            break;
                    }
                }
            }

            CPU.step();
//            PPU.fake_render();
        }
    }
}
