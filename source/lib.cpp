#include "lib.hpp"

#include <utility>

library::library(std::string path)
    : name("CygNES")
{
    SDL_Event e;

    cpu CPU = cpu();
    std::shared_ptr<cartridge> cart = std::make_shared<cartridge>();
    std::shared_ptr<controller> controller_a = std::make_shared<controller>(e);

    if (cart->open_rom_file(std::move(path)))
    {
        CPU.connect_cartridge(cart);
        CPU.connect_controller(controller_a);
        CPU.reset();

        bool quit = false;

        while (!quit)
        {
            while (SDL_PollEvent(&e) != 0)
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
                        case SDLK_r:
                            CPU.reset();
                            break;
                    }
                }
                controller_a->get_input();
            }

            CPU.step();
        }
    }
}
