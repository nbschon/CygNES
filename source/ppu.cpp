//
// Created by Noah Schonhorn on 4/2/22.
//

#include <cstdint>

#include "ppu.hpp"

#include "utils.hpp"

ppu::ppu()
{
    init(m_window, m_renderer, screen_width * 2, screen_height * 2);
    m_render_target =
        std::shared_ptr<SDL_Texture>(SDL_CreateTexture(&*m_renderer,
                                                       SDL_PIXELFORMAT_RGBA8888,
                                                       SDL_TEXTUREACCESS_TARGET,
                                                       screen_width,
                                                       screen_height),
                                     SDL_DestroyTexture);

    for (auto& byte : m_vram)
    {
        byte = 0x00;
    }

    for (auto& byte : m_pal_ram)
    {
        byte = 0x00;
    }

    m_scanline = 0;
    m_pixel = 0;
}

auto ppu::fake_render() -> void
{
    SDL_SetRenderTarget(&*m_renderer, &*m_render_target);
    SDL_SetRenderDrawColor(&*m_renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(&*m_renderer);

    uint8_t tile1;
    uint8_t tile2;

    for (int y = 0; y < 16; ++y)
    {
        for (int x = 0; x < 16; ++x)
        {
            for (int h = 0; h < 8; ++h)
            {
                tile1 = bus_read((x * 16) + (y * 256) + h);
                tile2 = bus_read((x * 16) + (y * 256) + 8 + h);

                for (int w = 0; w < 8; ++w)
                {
                    uint8_t pixel = ((tile1 & 0x1) << 1) + (tile2 & 0x1);

                    if (pixel == 1)
                    {
                        SDL_SetRenderDrawColor(
                            &*m_renderer, 0xFF, 0x00, 0x00, 0xFF);
                    }
                    else if (pixel == 2)
                    {
                        SDL_SetRenderDrawColor(
                            &*m_renderer, 0x00, 0xFF, 0x00, 0xFF);
                    }
                    else if (pixel == 3)
                    {
                        SDL_SetRenderDrawColor(
                            &*m_renderer, 0x00, 0x00, 0xFF, 0xFF);
                    }
                    else if (pixel == 0)
                    {
                        SDL_SetRenderDrawColor(
                            &*m_renderer, 0x00, 0x00, 0x00, 0xFF);
                    }

                    SDL_RenderDrawPoint(
                        &*m_renderer, (x * 8) + 8 - w, (y * 8) + h);
                    tile1 >>= 1;
                    tile2 >>= 1;
                }
            }

            for (int h = 0; h < 8; ++h)
            {
                tile1 = bus_read((x * 16) + (y * 256) + h + 0x1000);
                tile2 = bus_read((x * 16) + (y * 256) + 8 + h + 0x1000);

                for (int w = 0; w < 8; ++w)
                {
                    uint8_t pixel = ((tile1 & 0x1) << 1) + (tile2 & 0x1);

                    if (pixel == 1)
                    {
                        SDL_SetRenderDrawColor(
                            &*m_renderer, 0xFF, 0x00, 0x00, 0xFF);
                    }
                    else if (pixel == 2)
                    {
                        SDL_SetRenderDrawColor(
                            &*m_renderer, 0x00, 0xFF, 0x00, 0xFF);
                    }
                    else if (pixel == 3)
                    {
                        SDL_SetRenderDrawColor(
                            &*m_renderer, 0x00, 0x00, 0xFF, 0xFF);
                    }
                    else if (pixel == 0)
                    {
                        SDL_SetRenderDrawColor(
                            &*m_renderer, 0x00, 0x00, 0x00, 0xFF);
                    }

                    SDL_RenderDrawPoint(
                        &*m_renderer, (x * 8) + 8 - w + 128, (y * 8) + h);
                    tile1 >>= 1;
                    tile2 >>= 1;
                }
            }
        }
    }

    SDL_SetRenderTarget(&*m_renderer, nullptr);
    SDL_Rect srcRect {0, 0, 256, 240};
    SDL_RenderCopy(&*m_renderer, &*m_render_target, &srcRect, nullptr);
    //    SDL_RenderPresent(&*m_renderer);
}

auto ppu::connect_cartridge(std::shared_ptr<cartridge>& cart) -> void
{
    m_cart = cart;
}

auto ppu::reset() -> void
{
    m_latch = false;
    m_vram_addr.addr = 0;
    m_temp_addr.addr = 0;
    m_ctrl.ctrl = 0;
    m_mask.mask = 0;
    m_status.status = 0;
}

auto ppu::reg_read(uint16_t addr) -> uint8_t
{
    uint8_t byte = 0x00;

    switch (addr)
    {
        case 0x02:
            byte = m_status.status;
            m_status.vblank = 0;
            m_latch = false;
            break;
        case 0x04:
            // TODO: OAM stuff
            break;
        case 0x07:
            byte = m_read_buffer;
            m_read_buffer = bus_read(m_vram_addr.addr);
            if (m_vram_addr.addr >= 0x3F00)  // in palette part of RAM
            {
                byte = m_read_buffer;
            }

            if (m_ctrl.vram_inc == 1)
            {
                m_vram_addr.addr += 32;
            }
            else
            {
                m_vram_addr.addr++;
            }
            break;
        default:
            printf("NOTE: Illegal PPU read attempt at addr $%04X\n", addr);
            break;
    }

    return byte;
}

auto ppu::reg_write(uint16_t addr, uint8_t byte) -> void
{
    switch (addr)
    {
        case 0x00:
            m_ctrl.ctrl = byte;
            m_temp_addr.nametable_x = m_ctrl.nametable_x;
            m_temp_addr.nametable_y = m_ctrl.nametable_y;
            break;
        case 0x01:
            m_mask.mask = byte;
            break;
        case 0x03:
            // TODO: OAM
            break;
        case 0x04:
            // TODO: OAM
            break;
        case 0x05:
            if (!m_latch)
            {
                m_fine_x = (byte & 0x07);
                m_temp_addr.coarse_x = (byte >> 3);
                m_latch = !m_latch;
            }
            else
            {
                m_temp_addr.fine_y = (byte & 0x07);
                m_temp_addr.coarse_y = (byte >> 3);
                m_latch = !m_latch;
            }
            break;
        case 0x06:
            if (!m_latch)
            {
                m_temp_addr.high_byte = (byte & 0x3F);
                m_latch = !m_latch;
            }
            else
            {
                m_temp_addr.low_byte = byte;
                m_vram_addr.addr = m_temp_addr.addr;
                m_latch = !m_latch;
            }
            break;
        case 0x07:
            bus_write(m_vram_addr.addr, byte);
            if (m_ctrl.vram_inc == 1)
            {
                m_vram_addr.addr += 32;
            }
            else
            {
                m_vram_addr.addr++;
            }
            break;
        default:
            printf("NOTE: Illegal PPU write attempt at addr %04X\n", addr);
            break;
    }
}

auto ppu::bus_read(uint16_t addr) -> uint8_t
{
    uint8_t byte = 0x00;

    addr &= 0x3FFF;

    switch (addr)
    {
        case 0x0000 ... 0x1FFF:
            m_cart->ppu_read(addr, byte);
            break;
        case 0x2000 ... 0x3EFF:
            if (m_cart->get_mirroring())
            {
                byte = m_vram.at(addr & 0x800);
            }
            else
            {
                byte = m_vram.at(((addr / 2) & 0x400) + (addr % 0x400));
            }
            break;
        case 0x3F00 ... 0x3FFF:
            byte = m_pal_ram.at(addr & 0x1F);
            break;
    }

    return byte;
}

auto ppu::bus_write(uint16_t addr, uint8_t byte) -> void
{
    switch (addr)
    {
        case 0x0000 ... 0x1FFF:
            m_cart->ppu_write(addr, byte);
            printf("NOTE: Writing from PPU to cartridge at addr %04X", addr);
            break;
        case 0x2000 ... 0x3EFF:
            if (m_cart->get_mirroring())
            {
                m_vram.at(addr & 0x800) = byte;
            }
            else
            {
                m_vram.at(((addr / 2) & 0x400) + (addr & 0x3FF)) = byte;
            }
            break;
        case 0x3F00 ... 0x3FFF:
            m_pal_ram.at(addr & 0x1F) = byte;
            break;
    }
}

auto ppu::clock() -> void
{
    if (m_scanline == 261 or (m_scanline >= 0 and m_scanline <= 239))
    {
        if (m_scanline == 0 and m_pixel == 0)
        {
            m_pixel++;
        }

        if (m_scanline == 261 and m_pixel == 1)
        {
            m_status.vblank = 0;
        }

        if ((m_pixel >= 2 and m_pixel <= 257) or (m_pixel >= 321 and m_pixel <= 337))
        {
            shift();

            switch ((m_pixel - 1) % 8)
            {
                case 0:
                    reload();
                    m_nt_byte = bus_read(0x2000 | (m_vram_addr.addr & 0x0FFF));
                    break;
                case 2:
                    m_attr_byte = bus_read(0x23C0 | (m_vram_addr.addr & 0x0C00)
                                           | ((m_vram_addr.addr >> 4) & 0x38)
                                           | ((m_vram_addr.addr >> 2) & 0x07));
                    if (m_vram_addr.coarse_y & 0x2)
                    {
                        m_attr_byte >>= 4;
                    }
                    if (m_vram_addr.coarse_x & 0x2)
                    {
                        m_attr_byte >>= 2;
                    }

                    // don't need upper bits for attribute data
                    m_attr_byte &= 0x3;
                    break;
                case 4:
                    m_pattern_low =
                        bus_read((m_ctrl.bg_tbl << 12)
                                 + (static_cast<uint16_t>(m_nt_byte) << 4)
                                 + m_vram_addr.fine_y);
                    break;
                case 6:
                    m_pattern_high =
                        bus_read((m_ctrl.bg_tbl << 12)
                                 + (static_cast<uint16_t>(m_nt_byte) << 4)
                                 + m_vram_addr.fine_y + 8);
                    break;
                case 7:
                    inc_x();
                    break;
            }
        }

        if (m_pixel == 256)
        {
            inc_y();
        }

        if (m_pixel == 257)
        {
            reload();
            copy_x();
        }

        if (m_pixel == 338 or m_pixel == 340)
        {
            m_nt_byte = bus_read(0x2000 | (m_vram_addr.addr & 0x0FFF));
        }

        if (m_scanline == 261 and m_pixel >= 280 and m_pixel <= 304)
        {
            copy_y();
        }
    }

    if (m_scanline >= 241 and m_scanline <= 260)
    {
        if (m_scanline == 241 and m_pixel == 1)
        {
            m_status.vblank = 1;

            if (m_ctrl.do_nmi == 1)
            {
                m_do_interr = true;
            }
        }
    }

    uint8_t bg_pix = 0;
    uint8_t bg_pal = 0;

    if (m_mask.show_bg)
    {
        uint16_t mask = 0x8000 >> m_fine_x;

        uint8_t pix_low = (m_p_shift_low & mask) > 0;
        uint8_t pix_high = (m_p_shift_high & mask) > 0;

        bg_pix = (pix_high << 1) | pix_low;

        uint8_t pal_low = (m_a_shift_low & mask) > 0;
        uint8_t pal_high = (m_a_shift_high & mask) > 0;

        bg_pal = (pal_high << 1) | pal_low;
    }

    SDL_Color colo = get_color(bg_pal, bg_pix);
    SDL_SetRenderDrawColor(&*m_renderer, colo.r, colo.g, colo.b, 0xFF);
    SDL_RenderDrawPoint(&*m_renderer, m_pixel, m_scanline);

    m_pixel++;
    if (m_pixel >= 341)
    {
        m_pixel = 0;

        m_scanline++;
        if (m_scanline >= 261)
        {
            printf("Frame complete\n");
            SDL_Rect src_rect = {0, 0, 256, 240};
            SDL_SetRenderTarget(&*m_renderer, nullptr);
            SDL_RenderCopy(&*m_renderer, &*m_render_target, &src_rect, nullptr);
            SDL_RenderPresent(&*m_renderer);
            SDL_SetRenderTarget(&*m_renderer, &*m_render_target);
            m_scanline = 0;
        }
    }
}

auto ppu::nonmask() -> bool
{
    return false;
}

auto ppu::interr() -> bool
{
    if (m_do_interr)
    {
        m_do_interr = false;
        return true;
    }
    else
    {
        return false;
    }
}

auto ppu::step() -> void
{
    clock();

    m_pixel++;
    if (m_pixel > 340)
    {
        m_pixel = 0;

        m_scanline++;
        if (m_scanline > 261)
        {
            printf("Frame complete\n");
            SDL_Rect src_rect = {0, 0, 256, 250};
            SDL_SetRenderTarget(&*m_renderer, nullptr);
            SDL_RenderCopy(&*m_renderer, &*m_render_target, &src_rect, nullptr);
            SDL_RenderPresent(&*m_renderer);
            SDL_SetRenderTarget(&*m_renderer, &*m_render_target);
            m_scanline = 0;
        }
    }
}

auto ppu::copy_x() -> void
{
    if (m_mask.show_bg or m_mask.show_sprites)
    {
        m_vram_addr.coarse_x = m_temp_addr.coarse_x;
        m_vram_addr.nametable_x = m_temp_addr.nametable_x;
    }
}

auto ppu::copy_y() -> void
{
    if (m_mask.show_bg or m_mask.show_sprites)
    {
        m_vram_addr.fine_y = m_temp_addr.fine_y;
        m_vram_addr.coarse_y = m_temp_addr.coarse_y;
        m_vram_addr.nametable_y = m_temp_addr.coarse_y;
    }
}

auto ppu::inc_x() -> void
{
    // Stolen from NESDev wiki
    // It's public info for a reason!
    if (m_mask.show_bg or m_mask.show_sprites)
    {
        if ((m_vram_addr.addr & 0x1F) == 31)
        {
            m_vram_addr.addr &= ~(0x1F);
            m_vram_addr.addr ^= 0x400;
        }
        else
        {
            m_vram_addr.addr++;
        }
    }
}

auto ppu::inc_y() -> void
{
    // Also stolen from NESDev
    if (m_mask.show_bg or m_mask.show_sprites)
    {
        if ((m_vram_addr.addr & 0x7000) != 0x7000)
        {
            m_vram_addr.addr += 0x1000;
        }
        else
        {
            m_vram_addr.addr &= ~(0x7000);
            uint16_t y = ((m_vram_addr.addr & 0x3E0) >> 5);
            if (y == 29)
            {
                y = 0;
                m_vram_addr.addr ^= 0x0800;
            }
            else if (y == 31)
            {
                y = 0;
            }
            else
            {
                y++;
            }

            m_vram_addr.addr = ((m_vram_addr.addr & ~(0x3E0)) | (y << 5));
        }
    }
}

auto ppu::reload() -> void
{
    m_p_shift_low = (m_p_shift_low & 0xFF00) | m_pattern_low;
    m_p_shift_high = (m_p_shift_high & 0xFF00) | m_pattern_high;

    // Borrowed from javid9x's NES emulator
    // Fills the entire shift register with whatever the latch would've been
    // latched to, so you can shift them alongside the pattern table shifters
    m_a_shift_low =
        (m_a_shift_low & 0xFF00) | ((m_attr_byte & 1) ? 0xFF : 0x00);
    m_a_shift_high =
        (m_a_shift_high & 0xFF00) | ((m_attr_byte & 2) ? 0xFF : 0x00);
}

auto ppu::shift() -> void
{
    if (m_mask.show_bg)
    {
        m_p_shift_low <<= 1;
        m_p_shift_high <<= 1;
        m_a_shift_low <<= 1;
        m_a_shift_high <<= 1;
    }
}

auto ppu::draw() -> void
{
    uint8_t bg_pix = 0;
    uint8_t bg_pal = 0;

    if (m_mask.show_bg)
    {
        uint16_t mask = 0x8000 >> m_fine_x;

        uint8_t low_pix = (m_p_shift_low & mask) > 0;
        uint8_t high_pix = (m_p_shift_high & mask) > 0;

        bg_pix = (high_pix << 1) | low_pix;

        uint8_t low_pal = (m_a_shift_low & mask) > 0;
        uint8_t high_pal = (m_a_shift_high & mask) > 0;

        bg_pal = (high_pal << 1) | low_pal;

        SDL_Color col = get_color(bg_pal, bg_pix);
//        SDL_SetRenderTarget(&*m_renderer, &*m_render_target);
        SDL_SetRenderDrawColor(&*m_renderer, col.r, col.g, col.b, 0xFF);
        SDL_RenderDrawPoint(&*m_renderer, m_pixel, m_scanline);
    }
}

auto ppu::get_color(uint8_t pal, uint8_t pix) -> SDL_Color
{
    return m_colors.at(bus_read(0x3F00 + (pal << 2) + pix) & 0x3F);
}
