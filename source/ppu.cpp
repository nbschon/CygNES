//
// Created by Noah Schonhorn on 4/2/22.
//

#include "ppu.hpp"

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
    uint8_t byte;

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
            if (m_vram_addr.addr >= 0x3F00) // in palette part of RAM
            {
                byte = m_read_buffer;
            }

            if (m_ctrl.vram_inc)
            {
                m_vram_addr.addr += 32;
            }
            else
            {
                m_vram_addr.addr++;
            }
            break;
        default:
            printf("NOTE: Illegal PPU read attempt\n");
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
               m_temp_addr.low_byte =  byte;
               m_vram_addr = m_temp_addr;
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
            printf("NOTE: Illegal PPU write attempt\n");
            break;
    }
}

auto ppu::bus_read(uint16_t addr) -> uint8_t
{
    uint8_t byte;

    switch(addr)
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
}

auto ppu::bus_write(uint16_t addr, uint8_t byte) -> void
{
    switch(addr)
    {
        case 0x0000 ... 0x1FFF:
            m_cart->ppu_read(addr, byte);
            break;
        case 0x2000 ... 0x3EFF:
            if (m_cart->get_mirroring())
            {
                m_vram.at(addr & 0x800) = byte;
            }
            else
            {
                m_vram.at(((addr / 2) & 0x400) + (addr % 0x400)) = byte;
            }
            break;
        case 0x3F00 ... 0x3FFF:
            m_pal_ram.at(addr & 0x1F) = byte;
            break;
    }
}
auto ppu::clock() -> void
{

}

auto ppu::nmi() -> bool
{
    return false;
}

auto ppu::interr() -> bool
{
    return false;
}
