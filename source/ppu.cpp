//
// Created by Noah Schonhorn on 4/2/22.
//

#include <cstdint>

#include "ppu.hpp"

#include "utils.hpp"

ppu::ppu() : m_scanline(0), m_pixel(0)
{
    init(m_window, m_renderer, screen_width * 2, screen_height * 2);
    m_render_target =
        std::shared_ptr<SDL_Texture>(SDL_CreateTexture(&*m_renderer,
                                                       SDL_PIXELFORMAT_ARGB8888,
                                                       SDL_TEXTUREACCESS_STREAMING,
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

    for (auto& byte : m_oam)
    {
        byte = 0x00;
    }
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

    SDL_SetRenderTarget(&*m_renderer, &*m_render_target);
    SDL_SetRenderDrawColor(&*m_renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_Rect fill_rect = {0, 0, screen_width, screen_height};
    SDL_RenderFillRect(&*m_renderer, &fill_rect);
    SDL_SetRenderTarget(&*m_renderer, nullptr);
    SDL_RenderCopy(&*m_renderer, &*m_render_target, &fill_rect, nullptr);
    SDL_RenderPresent(&*m_renderer);
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
            byte = m_oam.at(m_oam_addr);
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
            m_oam_addr = byte;
            break;
        case 0x04:
            m_oam.at(m_oam_addr) = byte;
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
            addr &= 0x0FFF;
            if (m_cart->get_mirroring())
            {
                switch (addr & 0x0FFF)
                {
                    case 0x0000 ... 0x03FF:
                    case 0x0800 ... 0x0BFF:
                        byte = m_vram.at(addr & 0x03FF);
                        break;
                    case 0x0400 ... 0x07FF:
                    case 0x0C00 ... 0x0FFF:
                        byte = m_vram.at((addr & 0x07FF));
                        break;
                    default:
                        printf("NOTE: Unable to read VRAM at location $%04X\n", addr);
                        break;
                }
            }
            else
            {
                switch (addr & 0x0FFF)
                {
                    case 0x0000 ... 0x03FF:
                    case 0x0400 ... 0x07FF:
                        byte = m_vram.at(addr & 0x03FF);
                        break;
                    case 0x0800 ... 0x0BFF:
                    case 0x0C00 ... 0x0FFF:
                        byte = m_vram.at((addr & 0x07FF));
                        break;
                    default:
                        printf("NOTE: Unable to read VRAM at location $%04X\n", addr);
                        break;
                }
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
            printf("NOTE: Writing from PPU to cartridge at addr %04X\n", addr);
            break;
        case 0x2000 ... 0x3EFF:
            addr &= 0x0FFF;
            if (m_cart->get_mirroring())
            {
                switch (addr & 0x0FFF)
                {
                    case 0x0000 ... 0x03FF:
                    case 0x0800 ... 0x0BFF:
                        m_vram.at(addr & 0x03FF) = byte;
                        break;
                    case 0x0400 ... 0x07FF:
                    case 0x0C00 ... 0x0FFF:
                        m_vram.at((addr & 0x07FF)) = byte;
                        break;
                    default:
                        printf("NOTE: Unable to write %02X to VRAM at location $%04X\n", byte, addr);
                        break;
                }
            }
            else
            {
                switch (addr & 0x0FFF)
                {
                    case 0x0000 ... 0x03FF:
                    case 0x0400 ... 0x07FF:
                        m_vram.at(addr & 0x03FF) = byte;
                        break;
                    case 0x0800 ... 0x0BFF:
                    case 0x0C00 ... 0x0FFF:
                        m_vram.at((addr & 0x07FF)) = byte;
                        break;
                    default:
                        printf("NOTE: Unable to write %02X to VRAM at location $%04X\n", byte, addr);
                        break;
                }
            }

            break;
        case 0x3F00 ... 0x3FFF:
            m_pal_ram.at(addr & 0x1F) = byte;
            break;
    }
}

auto ppu::clock(line_type type) -> void
{
    if (type == visible or type == pre)
    {
        if (m_scanline == 0 and m_pixel == 0)
        {
            m_pixel++;
        }

        if (type == pre and m_pixel == 1)
        {
            m_status.vblank = 0;
        }

        switch (m_pixel)
        {
            case 2 ... 257:
            case 321 ... 337:
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
                        if ((m_vram_addr.coarse_y & 0x2) == 0x2)
                        {
                            m_attr_byte >>= 4;
                        }
                        if ((m_vram_addr.coarse_x & 0x2) == 0x2)
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

                if (m_pixel == 256)
                {
                    inc_y();
                }

                if (m_pixel == 257)
                {
                    reload();
                    copy_x();
                }
                break;
            case 338:
            case 340:
                m_nt_byte = bus_read(0x2000 | (m_vram_addr.addr & 0x0FFF));
                break;
            case 280 ... 304:
                if (type == pre)
                {
                    copy_y();
                }
                break;
        }
    }

    if (type == nmi and m_pixel == 1)
    {
        m_status.vblank = 1;

        if (m_ctrl.do_nmi == 1)
        {
            m_do_interr = true;
        }
    }

    uint8_t bg_pix = 0;
    uint8_t bg_pal = 0;

    if (m_mask.show_bg)
    {
        uint16_t mask = 0x8000 >> m_fine_x;

        uint8_t pix_low = ((m_p_shift_low & mask) > 0 ? 1 : 0);
        uint8_t pix_high = ((m_p_shift_high & mask) > 0 ? 1 : 0);

        bg_pix = (pix_high << 1) | pix_low;

        uint8_t pal_low = ((m_a_shift_low & mask) > 0 ? 1 : 0);
        uint8_t pal_high = ((m_a_shift_high & mask) > 0 ? 1 : 0);

        bg_pal = (pal_high << 1) | pal_low;
    }

    SDL_Color col = get_color(bg_pal, bg_pix);
    Uint32 final_pixel = (0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b);
    m_frame_buffer[(m_scanline * screen_width) + m_pixel] = final_pixel;
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
    switch (m_scanline)
    {
        case 0 ... 239:
            clock(visible);
            break;
        case 240:
            clock(post);
            break;
        case 241:
            clock(nmi);
            break;
        case 261:
            clock(pre);
            break;
        default:
            clock(idle);
            break;
    }

    m_pixel++;
    if (m_pixel > 340)
    {
        m_pixel = 0;

        m_scanline++;
        if (m_scanline > 261)
        {
            printf("Frame complete\n");
            SDL_Rect src_rect = {0, 0, screen_width, screen_height};
            SDL_UpdateTexture(&*m_render_target, nullptr, m_frame_buffer, m_pitch);
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

auto ppu::get_color(uint8_t pal, uint8_t pix) -> SDL_Color
{
    return m_colors.at(bus_read(0x3F00 + (pal << 2) + pix) & 0x3F);
}

auto ppu::oam_write(uint8_t index, uint8_t byte) -> void
{
    m_oam.at(index) = byte;
}
