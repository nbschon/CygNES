//
// Created by Noah Schonhorn on 4/2/22.
//

#ifndef CYGNES_PPU_HPP
#define CYGNES_PPU_HPP

#include <memory>

#include "SDL.h"

class ppu
{
    std::shared_ptr<SDL_Renderer> renderer;
    std::shared_ptr<SDL_Window> window;
    std::shared_ptr<SDL_Texture> render_target;

    union
    {
        struct
        {
            uint8_t nametable : 2;
            uint8_t vram_inc : 1;
            uint8_t sprite_tbl : 1;
            uint8_t bg_tbl : 1;
            uint8_t sprite_size : 1;
            uint8_t slave_select : 1;
            uint8_t do_nmi : 1;
        };

        uint8_t ctrl;
    } ctrl;

    union
    {
        struct
        {
            uint8_t grayscale : 1;
            uint8_t bg_left : 1;
            uint8_t sprite_left : 1;
            uint8_t show_bg : 1;
            uint8_t show_sprites : 1;
            uint8_t emph_red : 1;
            uint8_t emph_green : 1;
            uint8_t emph_blue : 1;
        };

        uint8_t mask;
    } mask;

    union
    {
        struct
        {
            uint8_t unused : 5;
            uint8_t overflow : 1;
            uint8_t zero_hit : 1;
            uint8_t vblank : 1;
        };

        uint8_t status;
    } status;

    union
    {
        struct
        {
            uint16_t coarse_x : 5;
            uint16_t coarse_y : 5;
            uint16_t nametable : 2;
            uint16_t fine_y : 3;
        };

        uint16_t addr;
    } addr_reg;

};

#endif  // CYGNES_PPU_HPP
