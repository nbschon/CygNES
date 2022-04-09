#include "cartridge.hpp"
#include "mapper000.hpp"

#include <fstream>

cartridge::cartridge()
{

}

auto cartridge::open_rom_file(std::string rom_path) -> bool
{
    bool load_success = true;

    std::ifstream rom(rom_path, std::ifstream::binary);

    if (!rom)
    {
        load_success = false;
    }
    else
    {
        char header[16];
        rom.read(header, 16);

        if (header[0] != 'N'
            and header[1] != 'E'
            and header[2] != 'S'
            and header[3] != '\n')
        {
            load_success = false;
        }

        if (load_success)
        {
            /*
                I don't think whether or not the file is 1.0 or 2.0 matters
            */
            // if (!header[7] & 0b000 and (header[7] & 0b1000) == 0b1000)
            // {
            //     printf("iNES 2.0 File\n\n");
            // }
            // else
            // {
            //     printf("Standard iNES file\n\n");
            // }

            int prg_rom_lsb = static_cast<uint16_t>(header[4]);
            int chr_rom_lsb = static_cast<uint16_t>(header[5]);
            int prg_rom_msb = static_cast<uint16_t>(header[9] & 0x0F);
            int chr_rom_msb = static_cast<uint16_t>(header[9] & 0xF0);

            int prg_rom_size = ((prg_rom_msb << 8) | prg_rom_lsb) * prg_rom_bank_size;
            int chr_rom_size = ((chr_rom_msb << 8) | chr_rom_lsb) * chr_rom_bank_size;

            printf("PRG-ROM Size (bytes): \t%d\n", prg_rom_size);
            printf("CHR-ROM Size (bytes): \t%d\n", chr_rom_size);

            m_vertically_mirrored = (header[6] & 0b0001) == 0b0001; 
            bool has_battery = (header[6] & 0b0010) == 0b0010;
            bool has_trainer = (header[6] & 0b0100) == 0b0100;
            bool four_screen = (header[6] & 0b1000) == 0b1000;

            int mapper_number = (header[6] >> 4);

            printf("Vertically mapped: \t\t%s\n", m_vertically_mirrored ? "true" : "false");
            printf("Battery backup: \t\t%s\n", has_battery ? "true" : "false");
            printf("Trainer in ROM: \t\t%s\n", has_trainer ? "true" : "false");
            printf("4 Screen Mode: \t\t%s\n", four_screen ? "true" : "false");

            /*
                Gonna ignore these for now
            */
            // uint16_t prg_ram_shift_count = (header[10] & 0x0F);
            // uint16_t prg_eeprom_shift_count = (header[10] & 0xF0);
            // uint16_t chr_ram_shift_count = (header[10] & 0x0F);
            // uint16_t chr_nvram_shift_count = (header[10] & 0xF0);

            if (has_trainer)
            {
                rom.seekg(512, std::ios_base::cur);
            }

            m_prg_rom.resize(prg_rom_size);
            rom.read(reinterpret_cast<char*>(m_prg_rom.data()), m_prg_rom.size());
            m_chr_rom.resize(chr_rom_size);
            rom.read(reinterpret_cast<char*>(m_chr_rom.data()), m_chr_rom.size());

            switch (mapper_number)
            {
                case 0:
                    m_mapper = std::make_shared<mapper000>(prg_rom_lsb, chr_rom_lsb);
                    break;
                default:
                    fprintf(stderr, "could not create mapper! iNES mapper #: %3d\n", mapper_number);
                    fprintf(stderr, "mapper is probably unimplemented.\n");
                    break;
            }
        }
    }

    return load_success;
}

auto cartridge::cpu_read(uint16_t addr, uint8_t& byte) -> bool
{
    bool success = false;
    int mapped_addr = 0;

    if (m_mapper->cpu_read(addr, mapped_addr))
    {
        byte = m_prg_rom[mapped_addr];
        success = true;
    }

    return success;
}

auto cartridge::cpu_write(uint16_t addr, uint8_t byte) -> bool
{
    bool success = false;
    int mapped_addr = 0;

    if (m_mapper->cpu_read(addr, mapped_addr))
    {
        m_prg_rom[mapped_addr] = byte;
        success = true;
    }

    return success;
}

auto cartridge::ppu_read(uint16_t addr, uint8_t& byte) -> bool
{
    bool success = false;
    int mapped_addr = 0;

    if (m_mapper->ppu_read(addr, mapped_addr))
    {
        byte = m_chr_rom[mapped_addr];
        success = true;
    }

    return success;
}

auto cartridge::ppu_write(uint16_t addr, uint8_t byte) -> bool
{
    bool success = false;
    int mapped_addr = 0;

    if (m_mapper->ppu_write(addr, mapped_addr))
    {
        m_chr_rom[mapped_addr] = byte;
        success = true;
    }

    return success;
}
auto cartridge::get_mirroring() const -> bool
{
    return m_vertically_mirrored;
}
