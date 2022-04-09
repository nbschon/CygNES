//
// Created by Noah Schonhorn on 4/2/22.
//

#ifndef CYGNES_PPU_HPP
#define CYGNES_PPU_HPP

#include <memory>
#include <array>

#include "SDL.h"
#include "cartridge.hpp"

class ppu
{
    std::shared_ptr<SDL_Renderer> m_renderer;
    std::shared_ptr<SDL_Window> m_window;
    std::shared_ptr<SDL_Texture> m_render_target;

    std::shared_ptr<cartridge> m_cart = nullptr;

    bool do_nmi;
    bool do_interr;

    bool m_latch = false;

    union
    {
        struct
        {
            uint8_t nametable_x  : 1;
            uint8_t nametable_y  : 1;
            uint8_t vram_inc     : 1;
            uint8_t sprite_tbl   : 1;
            uint8_t bg_tbl       : 1;
            uint8_t sprite_size  : 1;
            uint8_t slave_select : 1;
            uint8_t do_nmi       : 1;
        };

        uint8_t ctrl;
    } m_ctrl;

    union
    {
        struct
        {
            uint8_t grayscale    : 1;
            uint8_t bg_left      : 1;
            uint8_t sprite_left  : 1;
            uint8_t show_bg      : 1;
            uint8_t show_sprites : 1;
            uint8_t emph_red     : 1;
            uint8_t emph_green   : 1;
            uint8_t emph_blue    : 1;
        };

        uint8_t mask;
    } m_mask;

    // uint8_t m_oam_addr;
    // uint8_t m_oam_data;
    uint8_t m_scroll;

    union
    {
        struct
        {
            uint8_t unused   : 5;
            uint8_t overflow : 1;
            uint8_t zero_hit : 1;
            uint8_t vblank   : 1;
        };

        uint8_t status;
    } m_status;

    union addr_reg
    {
        struct
        {
            uint16_t coarse_x    : 5;
            uint16_t coarse_y    : 5;
            uint16_t nametable_x : 1;
            uint16_t nametable_y : 1;
            uint16_t fine_y      : 3;
        };

        struct
        {
            uint16_t low_byte  : 8;
            uint16_t high_byte : 8;
        };

        uint16_t addr;
    };

    addr_reg m_vram_addr, m_temp_addr;

    uint8_t m_fine_x;
    uint8_t m_read_buffer;

    static const int vram_size = 0x800;
    static const int pal_ram_size = 0x20;

    std::array<uint8_t, vram_size> m_vram;
    std::array<uint8_t, pal_ram_size> m_pal_ram;

  public:
    auto connect_cartridge(std::shared_ptr<cartridge>& cart) -> void;
    auto reset() -> void;
    auto clock() -> void;

    auto reg_read(uint16_t addr) -> uint8_t;
    auto reg_write(uint16_t addr, uint8_t byte) -> void;

    auto bus_read(uint16_t addr) -> uint8_t;
    auto bus_write(uint16_t addr, uint8_t byte) -> void;

    auto nmi() -> bool;
    auto interr() -> bool;

};

#endif  // CYGNES_PPU_HPP
