//
// Created by Noah Schonhorn on 4/2/22.
//

#ifndef CYGNES_PPU_HPP
#define CYGNES_PPU_HPP

#include <memory>
#include <array>
#include <cstdint>

#include "SDL.h"
#include "cartridge.hpp"

class ppu
{
    static const int screen_width = 256;
    static const int screen_height = 240;

    //
    std::shared_ptr<SDL_Renderer> m_renderer;
    std::shared_ptr<SDL_Window> m_window;
    std::shared_ptr<SDL_Texture> m_render_target;
    Uint32 *m_frame_buffer = new uint32_t[screen_width * screen_height];
    int m_pitch = screen_width * sizeof(Uint32);

    std::shared_ptr<cartridge> m_cart = nullptr;

    bool m_do_nmi;
    bool m_do_interr;

    bool m_latch = false;

    /*
     * $2000
     */
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

    /*
     * $2001
     */
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

    /*
     * $2002
     */
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

    // uint8_t m_oam_addr;
    // uint8_t m_oam_data;
    uint8_t m_scroll;

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

    // Data destinations for reading background-related bytes
    uint8_t m_nt_byte;
    uint8_t m_attr_byte;
    uint8_t m_pattern_low;
    uint8_t m_pattern_high;

    // Pattern table shift register (low)
    uint16_t m_p_shift_low;
    // Pattern table shift register (high)
    uint16_t m_p_shift_high;
    // Attribute table shift register (low)
    uint16_t m_a_shift_low;
    // Attribute table shift register (high)
    uint16_t m_a_shift_high;

    // Helper methods to consolidate PPU operations
    auto copy_x() -> void;
    auto copy_y() -> void;
    auto inc_x()  -> void;
    auto inc_y()  -> void;
    auto reload() -> void;
    auto shift()  -> void;

    // General VRAM & Palette
    static const int vram_size = 0x800;
    static const int pal_ram_size = 0x20;
    static const int colors_size = 0x40;

    std::array<uint8_t, vram_size> m_vram;
    std::array<uint8_t, pal_ram_size> m_pal_ram;
    static constexpr std::array<SDL_Color, colors_size> m_colors = {{
        // Palette colors from here:
        // https://www.nesdev.org/wiki/PPU_palettes
        // Row 1:
        {84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136},
        {68, 0, 100}, {92, 0, 48}, {84, 4, 0}, {60, 24, 0},
        {32, 42, 0}, {8, 58, 0}, {0, 64, 0}, {0, 60, 0},
        {0, 50, 60}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},

        // Row 2:
        {152, 150, 152}, {8, 76, 196}, {48, 50, 236}, {92, 30, 228},
        {136, 20, 176}, {160, 20, 100}, {152, 34, 32}, {120, 60, 0},
        {84, 90, 0}, {40, 114, 0}, {8, 124, 0}, {0, 118, 40},
        {0, 102, 120}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},

        // Row 3:
        {236, 238, 236}, {76, 154, 236}, {120, 124, 236}, {176, 98, 236},
        {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32},
        {160, 170, 0}, {116, 196, 0}, {76, 208, 32}, {56, 204, 108},
        {56, 180, 204}, {60, 60, 60}, {0, 0, 0}, {0, 0, 0},

        // Row 4:
        {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236},
        {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
        {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180},
        {160, 214, 228}, {160, 162, 160}, {0, 0, 0}, {0, 0, 0}
    }};
    auto get_color(uint8_t pal, uint8_t pix) -> SDL_Color;

    // OAM-related
    static const int oam_size = 0x100;
    std::array<uint8_t, oam_size> m_oam;
    uint8_t m_oam_addr;

    // Coordinates for currently-drawn pixel
    int m_scanline;
    int m_pixel;

    // Declarations for each possible "cycle types" of the PPU
    enum line_type
    {
        visible,
        post,
        nmi,
        pre,
        idle
    };

    auto clock(line_type type) -> void;

  public:

    ppu();
    auto connect_cartridge(std::shared_ptr<cartridge>& cart) -> void;
    auto reset() -> void;
    auto step() -> void;

    auto reg_read(uint16_t addr) -> uint8_t;
    auto reg_write(uint16_t addr, uint8_t byte) -> void;

    auto bus_read(uint16_t addr) -> uint8_t;
    auto bus_write(uint16_t addr, uint8_t byte) -> void;

    auto oam_write(uint8_t index, uint8_t byte) -> void;

    auto nonmask() -> bool;
    auto interr() -> bool;
};

#endif  // CYGNES_PPU_HPP
