//
// Created by Noah Schonhorn on 11/21/21.
//

#include <iostream>
#include <iomanip>
#include <chrono>
#include <bitset>
#include <ctime>
#include "cpu.hpp"

cpu::cpu()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream curr_time;
    curr_time << "../logs/cpu-" << std::put_time(&tm, "%m-%d-%Y %H-%M-%S");
    std::string file_name = curr_time.str();
#ifdef WIN32
    file_name.append("-win.txt");
#elif __APPLE__
    file_name.append("-osx.txt");
#elif __linux__
    file_name.append("-linux.txt");
#endif

#ifdef CPU_LOG
    m_log.open(file_name);
#endif

    m_ppu = std::make_shared<ppu>();
}

auto cpu::connect_cartridge(std::shared_ptr<cartridge>& cart) -> void
{
    this->m_cart = cart;
    m_ppu->connect_cartridge(cart);
}

auto cpu::read(uint16_t addr) -> uint8_t
{
    uint8_t byte = 0x00;

    switch (addr)
    {
        case 0x0000 ... 0x1FFF:
            byte = m_ram.at(addr & 0x7FF);
            break;
        case 0x2000 ... 0x3FFF:
            byte = m_ppu->reg_read(addr & 0x07);
            break;
        case 0x8000 ... 0xFFFF:
            m_cart->cpu_read(addr, byte);
            break;
        case 0x4016:
            byte = (m_controller_a_state & 1);
            printf("Controller state: %02X\n", byte);
            m_controller_a_state >>= 1;
            break;
        case 0x4017:
            // Controller port 2 would go here
            break;
        default:
            printf("NOTE: Invalid CPU read attempt at $%04X\n", addr);
            break;
    }

    return byte;
}

auto cpu::write(uint16_t addr, uint8_t byte) -> void
{
    switch (addr)
    {
        case 0x0000 ... 0x1FFF:
            m_ram.at(addr & 0x7FF) = byte;
            break;
        case 0x2000 ... 0x3FFF:
            m_ppu->reg_write(addr & 0x07, byte);
            break;
        case 0x8000 ... 0xFFFF:
            m_cart->cpu_write(addr, byte);
            break;
        case 0x4014:
            printf("NOTE: DMA from page $%02X of RAM into PPU\n", byte);
            m_oam_addr = byte;
            m_oam_index = 0;
            m_try_transfer = true;
            break;
        case 0x4016:
            m_controller_a_state = m_controller_a->get_status();
            break;
        default:
            printf("NOTE: Invalid CPU write attempt at $%04X w/ value %02X\n", addr, byte);
            break;
    }
}


void cpu::set_flag(m_flags flag, bool status)
{
    if (status)
    {
        m_stat_reg |= flag;
    }
    else
    {
        m_stat_reg &= ~flag;
    }
}

uint8_t cpu::get_flag(m_flags flag)
{
    if ((m_stat_reg & flag) == flag)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

auto cpu::fetch_byte_at_addr() -> uint8_t
{
    m_fetched_byte = read(m_addr_abs);

    return m_fetched_byte;
}

/*
 * ADDRESSING MODES
 */
auto cpu::get_absolute() -> void
{
    uint16_t low_byte = read(m_prog_counter);
    m_prog_counter++;
    uint16_t high_byte = read(m_prog_counter);
    m_prog_counter++;

    m_addr_abs = ((high_byte << 8) | low_byte);

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

auto cpu::get_absolute_x() -> void
{
    uint16_t low_byte = read(m_prog_counter);
    m_prog_counter++;
    uint16_t high_byte = read(m_prog_counter);
    m_prog_counter++;

    m_addr_abs = ((high_byte << 8) | low_byte) + m_x_reg;

    // Check to see if page (upper byte of address) changed
    if ((m_addr_abs & 0xFF00) != (high_byte << 8))
    {
        m_changed_page = true;
    }

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

auto cpu::get_absolute_y() -> void
{
    uint16_t low_byte = read(m_prog_counter);
    m_prog_counter++;
    uint16_t high_byte = read(m_prog_counter);
    m_prog_counter++;

    m_addr_abs = ((high_byte << 8) | low_byte) + m_y_reg;

    // Check for page change
    if ((m_addr_abs & 0xFF00) != (high_byte << 8))
    {
        m_changed_page = true;
    }

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

auto cpu::get_accumulator() -> void
{
    m_fetched_byte = m_accumulator;

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_fetched_byte);
#endif
}

auto cpu::get_implied() -> void
{
#ifdef CPU_LOG
    m_bytes << 0;
#endif
}

auto cpu::get_immediate() -> void
{
    m_addr_abs = m_prog_counter++;

#ifdef CPU_LOG
    m_bytes << static_cast<int>(read(m_addr_abs));
#endif
}

auto cpu::get_indirect() -> void
{
    uint16_t low_pointer = read(m_prog_counter);
    m_prog_counter++;
    uint16_t high_pointer = read(m_prog_counter);
    m_prog_counter++;

    uint16_t pointer = ((high_pointer << 8) | low_pointer);

    // Emulate hardware page-wraparound bug
    if (low_pointer == 0xFF)
    {
        // keep top half of address the same
        m_addr_abs = (read(pointer & 0xFF00) << 8) | read(pointer);
    }
    else
    {
        // do it normally otherwise
        m_addr_abs = (read(pointer + 1) << 8) | read(pointer);
    }

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

auto cpu::get_indirect_x() -> void
{
    uint16_t offset = read(m_prog_counter);
    m_prog_counter++;

    uint16_t low_pointer = (read(offset + m_x_reg) % 256);
    uint16_t high_pointer = (read(offset + m_x_reg + 1) % 256);

    m_addr_abs = (high_pointer << 8) | low_pointer;

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

auto cpu::get_indirect_y() -> void
{
    uint16_t offset = read(m_prog_counter);
    m_prog_counter++;

    uint16_t low_pointer = (read(offset & 0xFF));
    uint16_t high_pointer = (read((offset + 1) & 0xFF));

    m_addr_abs = (high_pointer << 8) | low_pointer;
    m_addr_abs += m_y_reg;

    // check for page change
    if ((m_addr_abs & 0xFF00) != (high_pointer << 8))
    {
        m_changed_page = true;
    }

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

auto cpu::get_relative() -> void
{
    m_addr_rel = read(m_prog_counter);
    m_prog_counter++;

    // Check if bit 7 is set - relative addr. mode range is [-128, 127]
    if (m_addr_rel & 0x80)
    {
        // fully set top byte of address
        m_addr_rel |= 0xFF00;
    }

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_rel);
#endif
}

auto cpu::get_zeropage() -> void
{
    m_addr_abs = read(m_prog_counter);
    m_prog_counter++;

    // keep address to page 0 of RAM
    m_addr_abs &= 0x00FF;

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

auto cpu::get_zeropage_x() -> void
{
    m_addr_abs = read(m_prog_counter) + m_x_reg;
    m_prog_counter++;
    m_addr_abs &= 0x00FF;

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

auto cpu::get_zeropage_y() -> void
{
    m_addr_abs = read(m_prog_counter) + m_y_reg;
    m_prog_counter++;
    m_addr_abs &= 0x00FF;

#ifdef CPU_LOG
    m_bytes << static_cast<int>(m_addr_abs);
#endif
}

// INSTRUCTIONS
/*
 * LOAD / STORE
 */
void cpu::LDA(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();
    m_accumulator = m_fetched_byte;

    set_flag(N, m_accumulator & 0x80); //AND'ing with bit 7, aka sign bit
    set_flag(Z, m_accumulator == 0);
}

void cpu::LDX(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();
    m_x_reg = m_fetched_byte;

    set_flag(N, m_x_reg & 0x80);
    set_flag(Z, m_x_reg == 0);
}

void cpu::LDY(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();
    m_y_reg = m_fetched_byte;

    set_flag(N, m_y_reg & 0x80);
    set_flag(Z, m_y_reg == 0);
}

void cpu::STA(addr_mode_ptr mode)
{
    (this->*mode)();

    write(m_addr_abs, m_accumulator);
}

void cpu::STX(addr_mode_ptr mode)
{
    (this->*mode)();

    write(m_addr_abs, m_x_reg);
}

void cpu::STY(addr_mode_ptr mode)
{
    (this->*mode)();

    write(m_addr_abs, m_y_reg);
}

void cpu::ADC(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    uint16_t tempResult = (uint16_t)m_accumulator + (uint16_t)m_fetched_byte + (uint16_t)get_flag(C);

    set_flag(C, tempResult > UINT8_MAX);
    set_flag(Z, (uint8_t)tempResult == 0);
    /*
     * Overflow only happens if two numbers of the same sign are added together,
     * and the sum produces something with a different sign from the two addends
     * (i.e. two positive numbers sum to negative, two negatives sum to positive)
     */
    bool overFlow = false;
    if (((m_accumulator & 0x80) == 0 && (m_fetched_byte & 0x80) == 0) && (tempResult & 0x80) == N)
    {
        overFlow = true;
    }
    else if (((m_accumulator & 0x80) == N && (m_fetched_byte & 0x80) == N) && (tempResult & 0x80) == 0)
    {
        overFlow = true;
    }
    set_flag(V, overFlow);
    set_flag(N, tempResult & 0x80);

    //Load result into m_accumulator
    m_accumulator = tempResult & 0xFF;
}

void cpu::SBC(addr_mode_ptr mode)
{
    (this->*mode)();

    // This works the same as the ADC function because of how binary arithmetic works,
    // just that the addend has its bits flipped (one's complement)
    fetch_byte_at_addr();

    uint16_t invertedVal = (uint16_t)m_fetched_byte ^ 0xFF;

    uint16_t tempResult = (uint16_t)m_accumulator + invertedVal + (uint16_t)get_flag(C);

    set_flag(C, tempResult > UINT8_MAX);
    set_flag(Z, (uint8_t)tempResult == 0);
    /*
     * Overflow only happens if two numbers of the same sign are added together,
     * and the sum produces something with a different sign from the two addends
     * (i.e. two positive numbers sum to negative, two negatives sum to positive)
     */
    bool overFlow = false;
    if (((m_accumulator & 0x80) == 0 && (invertedVal & 0x80) == 0) && (tempResult & 0x80) == N)
    {
        overFlow = true;
    }
    else if (((m_accumulator & 0x80) == N && (invertedVal & 0x80) == N) && (tempResult & 0x80) == 0)
    {
        overFlow = true;
    }
    set_flag(V, overFlow);
    set_flag(N, tempResult & 0x80);

    //Load result into m_accumulator
    m_accumulator = tempResult & 0xFF;
}

void cpu::INC(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    m_fetched_byte++;
    write(m_addr_abs, m_fetched_byte);

    set_flag(Z, m_fetched_byte == 0);
    set_flag(N, m_fetched_byte & 0x80);
}

void cpu::INX(addr_mode_ptr mode)
{
    (this->*mode)();

    m_x_reg++;

    set_flag(Z, m_x_reg == 0);
    set_flag(N, m_x_reg & 0x80);
}

void cpu::INY(addr_mode_ptr mode)
{
    (this->*mode)();

    m_y_reg++;

    set_flag(Z, m_y_reg == 0);
    set_flag(N, m_y_reg & 0x80);
}

void cpu::DEC(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    m_fetched_byte--;
    write(m_addr_abs, m_fetched_byte);

    set_flag(Z, m_fetched_byte == 0);
    set_flag(N, m_fetched_byte & 0x80);
}

void cpu::DEX(addr_mode_ptr mode)
{
    (this->*mode)();

    m_x_reg--;

    set_flag(Z, m_x_reg == 0);
    set_flag(N, m_x_reg & 0x80);
}

void cpu::DEY(addr_mode_ptr mode)
{
    (this->*mode)();

    m_y_reg--;

    set_flag(Z, m_y_reg == 0);
    set_flag(N, m_y_reg & 0x80);
}

void cpu::ASL(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    set_flag(C, (m_fetched_byte & 0x80) > 0);

    uint16_t shifted = (uint16_t)m_fetched_byte << 1;

    set_flag(Z, (shifted & 0xFF) == 0);
    set_flag(N, shifted & 0x80);

    if (mode == &cpu::get_accumulator)
    {
        m_accumulator = shifted & 0xFF;
    }
    else
    {
        write(m_addr_abs, shifted & 0xFF);
    }
}

void cpu::LSR(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    set_flag(C, (m_fetched_byte & 1) > 0);

    uint16_t shifted = (uint16_t)m_fetched_byte >> 1;

    set_flag(Z, (shifted & 0xFF) == 0);
    set_flag(N, shifted & 0x80);

    if (mode == &cpu::get_accumulator)
    {
        m_accumulator = shifted & 0xFF;
    }
    else
    {
        write(m_addr_abs, shifted & 0xFF);
    }
}

void cpu::ROL(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    uint8_t newZeroBit = get_flag(C);
    uint8_t newCarry = (m_fetched_byte & 0x80);

    uint16_t shifted = (uint16_t)m_fetched_byte << 1;

    shifted |= newZeroBit;

    set_flag(C, newCarry > 0);
    set_flag(Z, (shifted & 0xFF) == 0);
    set_flag(N, (shifted & 0x80));

    if (mode == &cpu::get_accumulator)
    {
        m_accumulator = shifted & 0xFF;
    }
    else
    {
        write(m_addr_abs, shifted & 0xFF);
    }
}

void cpu::ROR(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    uint8_t newSevenBit = get_flag(C);
    uint8_t newCarry = (m_fetched_byte & 1);

    uint16_t shifted = ((uint16_t)m_fetched_byte >> 1) | ((newSevenBit << 7));

    set_flag(C, newCarry > 0);
    set_flag(Z, (shifted & 0xFF) == 0);
    set_flag(N, shifted & 0x80);

    if (mode == &cpu::get_accumulator)
    {
        m_accumulator = shifted & 0xFF;
    }
    else
    {
        write(m_addr_abs, shifted & 0xFF);
    }
}

void cpu::AND(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    m_accumulator &= m_fetched_byte;

    set_flag(Z, m_accumulator == 0);
    set_flag(N, m_accumulator & 0x80);
}

void cpu::ORA(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    m_accumulator |= m_fetched_byte;

    set_flag(Z, m_accumulator == 0);
    set_flag(N, m_accumulator & 0x80);
}

void cpu::EOR(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    m_accumulator ^= m_fetched_byte;

    set_flag(Z, m_accumulator == 0);
    set_flag(N, m_accumulator & 0x80);
}

void cpu::CMP(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    uint16_t comp = (uint16_t)m_accumulator - (uint16_t)m_fetched_byte;

    set_flag(C, m_accumulator >= m_fetched_byte);
    set_flag(N, (comp & 0x80));
    set_flag(Z, (comp == 0));
}

void cpu::CPX(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    uint16_t comp = (uint16_t)m_x_reg - (uint16_t)m_fetched_byte;

    set_flag(C, m_x_reg >= m_fetched_byte);
    set_flag(N, (comp & 0x80));
    set_flag(Z, (comp == 0));
}

void cpu::CPY(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    uint16_t comp = (uint16_t)m_y_reg - (uint16_t)m_fetched_byte;

    set_flag(C, m_y_reg >= m_fetched_byte);
    set_flag(N, (comp & 0x80));
    set_flag(Z, (comp == 0));
}

void cpu::BIT(addr_mode_ptr mode)
{
    (this->*mode)();

    fetch_byte_at_addr();

    set_flag(N, (m_fetched_byte) & N);
    set_flag(V, (m_fetched_byte) & V);
    set_flag(Z, (m_fetched_byte & m_accumulator) == 0);
}

void cpu::BCC(addr_mode_ptr mode)
{
    (this->*mode)();

    if (get_flag(C) == 0)
    {
        m_cycles++;
        m_addr_abs = m_prog_counter + m_addr_rel;

        if ((m_addr_abs & 0xFF00) != (m_prog_counter & 0xFF00))
            m_cycles++;

        m_prog_counter = m_addr_abs;
    }
}

void cpu::BCS(addr_mode_ptr mode)
{
    (this->*mode)();

    if (get_flag(C) == 1)
    {
        m_cycles++;
        m_addr_abs = m_prog_counter + m_addr_rel;

        if ((m_addr_abs & 0xFF00) != (m_prog_counter & 0xFF00))
            m_cycles++;

        m_prog_counter = m_addr_abs;
    }
}

void cpu::BNE(addr_mode_ptr mode)
{
    (this->*mode)();

    if (get_flag(Z) == 0)
    {
        m_cycles++;
        m_addr_abs = m_prog_counter + m_addr_rel;

        if ((m_addr_abs & 0xFF00) != (m_prog_counter & 0xFF00))
            m_cycles++;

        m_prog_counter = m_addr_abs;
    }
}

void cpu::BEQ(addr_mode_ptr mode)
{
    (this->*mode)();

    if (get_flag(Z) == 1)
    {
        m_cycles++;
        m_addr_abs = m_prog_counter + m_addr_rel;

        if ((m_addr_abs & 0xFF00) != (m_prog_counter & 0xFF00))
            m_cycles++;

        m_prog_counter = m_addr_abs;
    }
}

void cpu::BPL(addr_mode_ptr mode)
{
    (this->*mode)();

    if (get_flag(N) == 0)
    {
        m_cycles++;
        m_addr_abs = m_prog_counter + m_addr_rel;

        if ((m_addr_abs & 0xFF00) != (m_prog_counter & 0xFF00))
            m_cycles++;

        m_prog_counter = m_addr_abs;
    }
}

void cpu::BMI(addr_mode_ptr mode)
{
    (this->*mode)();

    if (get_flag(N) == 1)
    {
        m_cycles++;
        m_addr_abs = m_prog_counter + m_addr_rel;

        if ((m_addr_abs & 0xFF00) != (m_prog_counter & 0xFF00))
            m_cycles++;

        m_prog_counter = m_addr_abs;
    }
}

void cpu::BVC(addr_mode_ptr mode)
{
    (this->*mode)();

    if (get_flag(V) == 0)
    {
        m_cycles++;
        m_addr_abs = m_prog_counter + m_addr_rel;

        if ((m_addr_abs & 0xFF00) != (m_prog_counter & 0xFF00))
            m_cycles++;

        m_prog_counter = m_addr_abs;
    }
}

void cpu::BVS(addr_mode_ptr mode)
{
    (this->*mode)();

    if (get_flag(V) == 1)
    {
        m_cycles++;
        m_addr_abs = m_prog_counter + m_addr_rel;

        if ((m_addr_abs & 0xFF00) != (m_prog_counter & 0xFF00))
            m_cycles++;

        m_prog_counter = m_addr_abs;
    }
}

void cpu::TAX(addr_mode_ptr mode)
{
    (this->*mode)();

    m_x_reg = m_accumulator;

    set_flag(Z, m_x_reg == 0);
    set_flag(N, m_x_reg & 0x80);
}

void cpu::TXA(addr_mode_ptr mode)
{
    (this->*mode)();

    m_accumulator = m_x_reg;

    set_flag(Z, m_accumulator == 0);
    set_flag(N, m_accumulator & 0x80);
}

void cpu::TAY(addr_mode_ptr mode)
{
    (this->*mode)();

    m_y_reg = m_accumulator;

    set_flag(Z, m_y_reg == 0);
    set_flag(N, m_y_reg & 0x80);
}

void cpu::TYA(addr_mode_ptr mode)
{
    (this->*mode)();

    m_accumulator = m_y_reg;

    set_flag(Z, m_accumulator == 0);
    set_flag(N, m_accumulator & 0x80);
}

void cpu::TSX(addr_mode_ptr mode)
{
    (this->*mode)();

    m_x_reg = m_stack_ptr;

    set_flag(Z, m_x_reg == 0);
    set_flag(N, m_x_reg & 0x80);
}

void cpu::TXS(addr_mode_ptr mode)
{
    (this->*mode)();

    m_stack_ptr = m_x_reg;
}

void cpu::PHA(addr_mode_ptr mode)
{
    (this->*mode)();

    write(0x100 + m_stack_ptr, m_accumulator);
    m_stack_ptr--;
}

void cpu::PLA(addr_mode_ptr mode)
{
    (this->*mode)();

    m_stack_ptr++;
    m_accumulator = read(0x100 + m_stack_ptr);

    set_flag(Z, m_accumulator == 0);
    set_flag(N, m_accumulator & 0x80);
}

void cpu::PHP(addr_mode_ptr mode)
{
    (this->*mode)();

    write(0x100 + m_stack_ptr, m_stat_reg | B | U);
    set_flag(B, false);
    set_flag(U, false);
    m_stack_ptr--;
}

void cpu::PLP(addr_mode_ptr mode)
{
    (this->*mode)();

    m_stack_ptr++;
    m_stat_reg = read(0x100 + m_stack_ptr);
    set_flag(U, true);
}

void cpu::JMP(addr_mode_ptr mode)
{
    (this->*mode)();

    m_prog_counter = m_addr_abs;
}

void cpu::JSR(addr_mode_ptr mode)
{
    (this->*mode)();

    m_prog_counter--;
    write(0x100 + m_stack_ptr, (m_prog_counter >> 8) & 0xFF);
    m_stack_ptr--;
    write(0x100 + m_stack_ptr, (m_prog_counter & 0xFF));
    m_stack_ptr--;

    m_prog_counter = m_addr_abs;
}

void cpu::RTS(addr_mode_ptr mode)
{
    (this->*mode)();

    m_stack_ptr++;
    uint8_t low_byte = read(0x100 + m_stack_ptr);
    m_stack_ptr++;
    uint8_t high_byte = read(0x100 + m_stack_ptr);

    uint16_t returnAddress = ((uint16_t)(high_byte << 8) | (uint16_t)low_byte);
    m_prog_counter = returnAddress;

    m_prog_counter++;
}

void cpu::RTI(addr_mode_ptr mode)
{
//    printf("[![RTI]!]\n");

    (this->*mode)();

    m_stack_ptr++;
    m_stat_reg = read(0x100 + m_stack_ptr);

    m_stack_ptr++;
    uint8_t low_byte = read(0x100 + m_stack_ptr);
    m_stack_ptr++;
    uint8_t high_byte = read(0x100 + m_stack_ptr);

    uint16_t returnAddress = ((uint16_t)(high_byte << 8) | low_byte);
    m_prog_counter = returnAddress;
}

void cpu::CLC(addr_mode_ptr mode)
{
    (this->*mode)();

    set_flag(C, false);
}

void cpu::SEC(addr_mode_ptr mode)
{
    (this->*mode)();

    set_flag(C, true);
}

void cpu::CLD(addr_mode_ptr mode)
{
    (this->*mode)();

    set_flag(D, false);
}

void cpu::SED(addr_mode_ptr mode)
{
    (this->*mode)();

    set_flag(D, true);
}

void cpu::CLI(addr_mode_ptr mode)
{
    (this->*mode)();

    set_flag(I, false);
}

void cpu::SEI(addr_mode_ptr mode)
{
    (this->*mode)();

    set_flag(I, true);
}

void cpu::CLV(addr_mode_ptr mode)
{
    (this->*mode)();

    set_flag(V, false);
}

auto cpu::BRK(addr_mode_ptr mode) -> void
{
    (this->*mode)();

    m_prog_counter++;

    set_flag(I, true);
    set_flag(B, true);

    write(0x100 + m_stack_ptr, (m_prog_counter >> 8) & 0xFF);
    m_stack_ptr--;
    write(0x100 + m_stack_ptr, (m_prog_counter & 0xFF));
    m_stack_ptr--;

    write(0x100 + m_stack_ptr, m_stat_reg);
    m_stack_ptr--;

    uint8_t low_byte = read(0xFFFE);
    uint8_t high_byte = read(0xFFFF);

    m_prog_counter = (high_byte << 8) | low_byte;
}

auto cpu::NOP(addr_mode_ptr mode) -> void
{
    (this->*mode)();
    
    // NOTE: This is currently inaccurate
    // Different NOPs take different # of cycles
}

auto cpu::XXX() -> void
{
    // Does nothing, for all unofficial m_opcodes
    printf("NOTE: Illegal opcode attempt at PC $%04X\n",
           static_cast<int>(m_prog_counter));
}

auto cpu::reset() -> void
{
    printf("[![RESET]!]\n");
    m_addr_abs = 0xFFFC;
    uint16_t low_byte = read(m_addr_abs);
    uint16_t high_byte = read(m_addr_abs + 1);

    m_prog_counter = (high_byte << 8) | low_byte;
//    m_prog_counter = 0xC000;

    m_accumulator = 0;
    m_x_reg = 0;
    m_y_reg = 0;
    m_stack_ptr = 0xFD;
    m_stat_reg = 0b00100100;

    m_addr_rel = 0x0000;
    m_addr_abs = 0x0000;
    m_fetched_byte = 0x00;

    m_cycles = 8;
    m_ppu->reset();
}

auto cpu::interrupt_request() -> void
{
//    printf("[![IRQ]!]\n");
    if (!get_flag(I))
    {
        // push prog counter to stack
        write(0x0100 + m_stack_ptr, (m_prog_counter >> 8) & 0x00FF);
        m_stack_ptr--;
        write(0x0100 + m_stack_ptr, m_prog_counter & 0x00FF);
        m_stack_ptr--;

        // push m_status register to stack
        set_flag(B, false);
        set_flag(U, true);
        set_flag(I, true);
        write(0x0100 + m_stack_ptr, m_stat_reg);
        m_stack_ptr--;

        // read new prog counter from address
        m_addr_abs = 0xFFFE;
        uint16_t low_byte = read(m_addr_abs);
        uint16_t high_byte = read(m_addr_abs + 1);
        m_prog_counter = ((high_byte << 8) | low_byte);

        // adjust cycle count
        m_cycles = 7;
    }
}

auto cpu::nonmaskable_interrupt() -> void
{
//    printf("[![NMI]!]\n");
    // push prog counter to stack
    write(0x0100 + m_stack_ptr, (m_prog_counter >> 8) & 0x00FF);
    m_stack_ptr--;
    write(0x0100 + m_stack_ptr, m_prog_counter & 0x00FF);
    m_stack_ptr--;

    // push m_status register to stack
    set_flag(B, false);
    set_flag(U, true);
    set_flag(I, true);
    write(0x0100 + m_stack_ptr, m_stat_reg);
    m_stack_ptr--;

    // read new prog counter from address
    m_addr_abs = 0xFFFA;
    uint16_t low_byte = read(m_addr_abs);
    uint16_t high_byte = read(m_addr_abs + 1);
    m_prog_counter = ((high_byte << 8) | low_byte);

    // adjust cycle count
    m_cycles = 7;
}

auto cpu::clock() -> void
{
    m_changed_page = false;

    if (m_cycles == 0)
    {
        m_opcode = read(m_prog_counter);

        // THIS IS UGLY
        // make it look better at some point (if possible)
#ifdef CPU_LOG
        std::stringstream logLine;
        logLine << std::uppercase << std::hex << m_prog_counter << " " << std::hex << std::setfill('0') << std::setw(2) << (int)m_opcode;
        std::stringstream regLine;
        std::bitset<8> bits(m_stat_reg);
        regLine << std::uppercase << "A: " << std::setfill('0') << std::setw(2) << std::hex << (int)m_accumulator
                << " X: " << std::setfill('0') << std::setw(2) << std::hex << (int)m_x_reg
                << " Y: " << std::setfill('0') << std::setw(2) << std::hex << (int)m_y_reg
                << " S: " << std::setfill('0') << std::setw(2) << std::hex << (int)m_stat_reg << " " << bits;

        m_bytes.clear();
        m_bytes.str(std::string());
#endif

        m_prog_counter++;

        set_flag(U, true);

        switch (m_opcode)
        {
            case 0x00:
                m_cycles = 7;
                BRK(&cpu::get_implied);
                break;
            case 0x01:
                m_cycles = 6;
                ORA(&cpu::get_indirect_x);
                break;
            case 0x05:
                m_cycles = 3;
                ORA(&cpu::get_zeropage);
                break;
            case 0x06:
                m_cycles = 5;
                ASL(&cpu::get_zeropage);
                break;
            case 0x08:
                m_cycles = 3;
                PHP(&cpu::get_implied);
                break;
            case 0x09:
                m_cycles = 2;
                ORA(&cpu::get_immediate);
                break;
            case 0x0A:
                m_cycles = 2;
                ASL(&cpu::get_accumulator);
                break;
            case 0x0D:
                m_cycles = 4;
                ORA(&cpu::get_absolute);
                break;
            case 0x0E:
                m_cycles = 6;
                ASL(&cpu::get_absolute);
                break;
            case 0x10:
                m_cycles = 2;
                BPL(&cpu::get_relative);
                if (m_changed_page) m_cycles++;
                break;
            case 0x11:
                m_cycles = 5;
                ORA(&cpu::get_indirect_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0x15:
                m_cycles = 4;
                ORA(&cpu::get_zeropage_x);
                break;
            case 0x16:
                m_cycles = 6;
                ASL(&cpu::get_zeropage_x);
                break;
            case 0x18:
                m_cycles = 2;
                CLC(&cpu::get_implied);
                break;
            case 0x19:
                m_cycles = 4;
                ORA(&cpu::get_absolute_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0x1D:
                m_cycles = 4;
                ORA(&cpu::get_absolute_x);
                if (m_changed_page) m_cycles++;
                break;
            case 0x1E:
                m_cycles = 7;
                ASL(&cpu::get_absolute_x);
                break;
            case 0x20:
                m_cycles = 6;
                JSR(&cpu::get_absolute);
                break;
            case 0x21:
                m_cycles = 6;
                AND(&cpu::get_indirect_x);
                break;
            case 0x24:
                m_cycles = 3;
                BIT(&cpu::get_zeropage);
                break;
            case 0x25:
                m_cycles = 3;
                AND(&cpu::get_zeropage);
                break;
            case 0x26:
                m_cycles = 5;
                ROL(&cpu::get_zeropage);
                break;
            case 0x28:
                m_cycles = 4;
                PLP(&cpu::get_implied);
                break;
            case 0x29:
                m_cycles = 2;
                AND(&cpu::get_immediate);
                break;
            case 0x2A:
                m_cycles = 2;
                ROL(&cpu::get_accumulator);
                break;
            case 0x2C:
                m_cycles = 4;
                BIT(&cpu::get_absolute);
                break;
            case 0x2D:
                m_cycles = 4;
                AND(&cpu::get_absolute);
                break;
            case 0x2E:
                m_cycles = 6;
                ROL(&cpu::get_absolute);
                break;
            case 0x30:
                m_cycles = 2;
                BMI(&cpu::get_relative);
                if (m_changed_page) m_cycles++;
                break;
            case 0x31:
                m_cycles = 5;
                AND(&cpu::get_indirect_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0x35:
                m_cycles = 4;
                AND(&cpu::get_zeropage_x);
                break;
            case 0x36:
                m_cycles = 6;
                ROL(&cpu::get_zeropage_x);
                break;
            case 0x38:
                m_cycles = 2;
                SEC(&cpu::get_implied);
                break;
            case 0x39:
                m_cycles = 4;
                AND(&cpu::get_absolute_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0x3D:
                m_cycles = 4;
                AND(&cpu::get_absolute_x);
                if (m_changed_page) m_cycles++;
                break;
            case 0x3E:
                m_cycles = 7;
                ROL(&cpu::get_absolute_x);
                break;
            case 0x40:
                m_cycles = 6;
                RTI(&cpu::get_implied);
                break;
            case 0x41:
                m_cycles = 6;
                EOR(&cpu::get_indirect_x);
                break;
            case 0x45:
                m_cycles = 3;
                EOR(&cpu::get_zeropage);
                break;
            case 0x46:
                m_cycles = 5;
                LSR(&cpu::get_zeropage);
                break;
            case 0x48:
                m_cycles = 3;
                PHA(&cpu::get_implied);
                break;
            case 0x49:
                m_cycles = 2;
                EOR(&cpu::get_immediate);
                break;
            case 0x4A:
                m_cycles = 2;
                LSR(&cpu::get_accumulator);
                break;
            case 0x4C:
                m_cycles = 3;
                JMP(&cpu::get_absolute);
                break;
            case 0x4D:
                m_cycles = 4;
                EOR(&cpu::get_absolute);
                break;
            case 0x4E:
                m_cycles = 6;
                LSR(&cpu::get_absolute);
                break;
            case 0x50:
                m_cycles = 2;
                BVC(&cpu::get_relative);
                if (m_changed_page) m_cycles++;
                break;
            case 0x51:
                m_cycles = 5;
                EOR(&cpu::get_indirect_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0x55:
                m_cycles = 4;
                EOR(&cpu::get_zeropage_x);
                break;
            case 0x56:
                m_cycles = 6;
                LSR(&cpu::get_zeropage_x);
                break;
            case 0x58:
                m_cycles = 2;
                CLI(&cpu::get_implied);
                break;
            case 0x59:
                m_cycles = 4;
                EOR(&cpu::get_absolute_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0x5D:
                m_cycles = 4;
                EOR(&cpu::get_absolute_x);
                if (m_changed_page) m_cycles++;
                break;
            case 0x5E:
                m_cycles = 7;
                LSR(&cpu::get_absolute_x);
                break;
            case 0x60:
                m_cycles = 6;
                RTS(&cpu::get_implied);
                break;
            case 0x61:
                m_cycles = 6;
                ADC(&cpu::get_indirect_x);
                break;
            case 0x65:
                m_cycles = 3;
                ADC(&cpu::get_zeropage);
                break;
            case 0x66:
                m_cycles = 5;
                ROR(&cpu::get_zeropage);
                break;
            case 0x68:
                m_cycles = 4;
                PLA(&cpu::get_implied);
                break;
            case 0x69:
                m_cycles = 2;
                ADC(&cpu::get_immediate);
                break;
            case 0x6A:
                m_cycles = 2;
                ROR(&cpu::get_accumulator);
                break;
            case 0x6C:
                m_cycles = 5;
                JMP(&cpu::get_indirect);
                break;
            case 0x6D:
                m_cycles = 4;
                ADC(&cpu::get_absolute);
                break;
            case 0x6E:
                m_cycles = 6;
                ROR(&cpu::get_absolute);
                break;
            case 0x70:
                m_cycles = 2;
                BVS(&cpu::get_relative);
                if (m_changed_page) m_cycles++;
                break;
            case 0x71:
                m_cycles = 5;
                ADC(&cpu::get_indirect_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0x75:
                m_cycles = 4;
                ADC(&cpu::get_zeropage_x);
                break;
            case 0x76:
                m_cycles = 6;
                ROR(&cpu::get_zeropage_x);
                break;
            case 0x78:
                m_cycles = 2;
                SEI(&cpu::get_implied);
                break;
            case 0x79:
                m_cycles = 4;
                ADC(&cpu::get_absolute_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0x7D:
                m_cycles = 4;
                ADC(&cpu::get_absolute_x);
                if (m_changed_page) m_cycles++;
                break;
            case 0x7E:
                m_cycles = 7;
                ROR(&cpu::get_absolute_x);
                break;
            case 0x81:
                m_cycles = 6;
                STA(&cpu::get_indirect_x);
                break;
            case 0x84:
                m_cycles = 3;
                STY(&cpu::get_zeropage);
                break;
            case 0x85:
                m_cycles = 3;
                STA(&cpu::get_zeropage);
                break;
            case 0x86:
                m_cycles = 3;
                STX(&cpu::get_zeropage);
                break;
            case 0x88:
                m_cycles = 2;
                DEY(&cpu::get_implied);
                break;
            case 0x8A:
                m_cycles = 2;
                TXA(&cpu::get_implied);
                break;
            case 0x8C:
                m_cycles = 4;
                STY(&cpu::get_absolute);
                break;
            case 0x8D:
                m_cycles = 4;
                STA(&cpu::get_absolute);
                break;
            case 0x8E:
                m_cycles = 4;
                STX(&cpu::get_absolute);
                break;
            case 0x90:
                m_cycles = 2;
                BCC(&cpu::get_relative);
                if (m_changed_page) m_cycles++;
                break;
            case 0x91:
                m_cycles = 6;
                STA(&cpu::get_indirect_y);
                break;
            case 0x94:
                m_cycles = 4;
                STY(&cpu::get_zeropage_x);
                break;
            case 0x95:
                m_cycles = 4;
                STA(&cpu::get_zeropage_x);
                break;
            case 0x96:
                m_cycles = 4;
                STX(&cpu::get_zeropage_y);
                break;
            case 0x98:
                m_cycles = 2;
                TYA(&cpu::get_implied);
                break;
            case 0x99:
                m_cycles = 5;
                STA(&cpu::get_absolute_y);
                break;
            case 0x9A:
                m_cycles = 2;
                TXS(&cpu::get_implied);
                break;
            case 0x9D:
                m_cycles = 5;
                STA(&cpu::get_absolute_x);
                break;
            case 0xA0:
                m_cycles = 2;
                LDY(&cpu::get_immediate);
                break;
            case 0xA1:
                m_cycles = 6;
                LDA(&cpu::get_indirect_x);
                break;
            case 0xA2:
                m_cycles = 2;
                LDX(&cpu::get_immediate);
                break;
            case 0xA4:
                m_cycles = 3;
                LDY(&cpu::get_zeropage);
                break;
            case 0xA5:
                m_cycles = 3;
                LDA(&cpu::get_zeropage);
                break;
            case 0xA6:
                m_cycles = 3;
                LDX(&cpu::get_zeropage);
                break;
            case 0xA8:
                m_cycles = 2;
                TAY(&cpu::get_implied);
                break;
            case 0xA9:
                m_cycles = 2;
                LDA(&cpu::get_immediate);
                break;
            case 0xAA:
                m_cycles = 2;
                TAX(&cpu::get_implied);
                break;
            case 0xAC:
                m_cycles = 4;
                LDY(&cpu::get_absolute);
                break;
            case 0xAD:
                m_cycles = 4;
                LDA(&cpu::get_absolute);
                break;
            case 0xAE:
                m_cycles = 4;
                LDX(&cpu::get_absolute);
                break;
            case 0xB0:
                m_cycles = 2;
                BCS(&cpu::get_relative);
                if (m_changed_page) m_cycles++;
                break;
            case 0xB1:
                m_cycles = 5;
                LDA(&cpu::get_indirect_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0xB4:
                m_cycles = 4;
                LDY(&cpu::get_zeropage_x);
                break;
            case 0xB5:
                m_cycles = 4;
                LDA(&cpu::get_zeropage_x);
                break;
            case 0xB6:
                m_cycles = 4;
                LDX(&cpu::get_zeropage_y);
                break;
            case 0xB8:
                m_cycles = 2;
                CLV(&cpu::get_implied);
                break;
            case 0xB9:
                m_cycles = 4;
                LDA(&cpu::get_absolute_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0xBA:
                m_cycles = 2;
                TSX(&cpu::get_implied);
                break;
            case 0xBC:
                m_cycles = 4;
                LDY(&cpu::get_absolute_x);
                if (m_changed_page) m_cycles++;
                break;
            case 0xBD:
                m_cycles = 4;
                LDA(&cpu::get_absolute_x);
                if (m_changed_page) m_cycles++;
                break;
            case 0xBE:
                m_cycles = 4;
                LDX(&cpu::get_absolute_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0xC0:
                m_cycles = 2;
                CPY(&cpu::get_immediate);
                break;
            case 0xC1:
                m_cycles = 6;
                CMP(&cpu::get_indirect_x);
                break;
            case 0xC4:
                m_cycles = 3;
                CPY(&cpu::get_zeropage);
                break;
            case 0xC5:
                m_cycles = 3;
                CMP(&cpu::get_zeropage);
                break;
            case 0xC6:
                m_cycles = 5;
                DEC(&cpu::get_zeropage);
                break;
            case 0xC8:
                m_cycles = 2;
                INY(&cpu::get_implied);
                break;
            case 0xC9:
                m_cycles = 2;
                CMP(&cpu::get_immediate);
                break;
            case 0xCA:
                m_cycles = 2;
                DEX(&cpu::get_implied);
                break;
            case 0xCC:
                m_cycles = 4;
                CPY(&cpu::get_absolute);
                break;
            case 0xCD:
                m_cycles = 4;
                CMP(&cpu::get_absolute);
                break;
            case 0xCE:
                m_cycles = 6;
                DEC(&cpu::get_absolute);
                break;
            case 0xD0:
                m_cycles = 2;
                BNE(&cpu::get_relative);
                if (m_changed_page) m_cycles++;
                break;
            case 0xD1:
                m_cycles = 5;
                CMP(&cpu::get_indirect_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0xD5:
                m_cycles = 4;
                CMP(&cpu::get_zeropage_x);
                break;
            case 0xD6:
                m_cycles = 6;
                DEC(&cpu::get_zeropage_x);
                break;
            case 0xD8:
                m_cycles = 2;
                CLD(&cpu::get_implied);
                break;
            case 0xD9:
                m_cycles = 4;
                CMP(&cpu::get_absolute_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0xDD:
                m_cycles = 4;
                CMP(&cpu::get_absolute_x);
                if (m_changed_page) m_cycles++;
                break;
            case 0xDE:
                m_cycles = 7;
                DEC(&cpu::get_absolute_x);
                break;
            case 0xE0:
                m_cycles = 2;
                CPX(&cpu::get_immediate);
                break;
            case 0xE1:
                m_cycles = 6;
                SBC(&cpu::get_indirect_x);
                break;
            case 0xE4:
                m_cycles = 3;
                CPX(&cpu::get_zeropage);
                break;
            case 0xE5:
                m_cycles = 3;
                SBC(&cpu::get_zeropage);
                break;
            case 0xE6:
                m_cycles = 5;
                INC(&cpu::get_zeropage);
                break;
            case 0xE8:
                m_cycles = 2;
                INX(&cpu::get_implied);
                break;
            case 0xE9:
                m_cycles = 2;
                SBC(&cpu::get_immediate);
                break;
            case 0xEA:
                m_cycles = 2;
                NOP(&cpu::get_implied);
                break;
            case 0xEC:
                m_cycles = 4;
                CPX(&cpu::get_absolute);
                break;
            case 0xED:
                m_cycles = 4;
                SBC(&cpu::get_absolute);
                break;
            case 0xEE:
                m_cycles = 6;
                INC(&cpu::get_absolute);
                break;
            case 0xF0:
                m_cycles = 2;
                BEQ(&cpu::get_relative);
                if (m_changed_page) m_cycles++;
                break;
            case 0xF1:
                m_cycles = 5;
                SBC(&cpu::get_indirect_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0xF5:
                m_cycles = 4;
                SBC(&cpu::get_zeropage_x);
                break;
            case 0xF6:
                m_cycles = 6;
                INC(&cpu::get_zeropage_x);
                break;
            case 0xF8:
                m_cycles = 2;
                SED(&cpu::get_implied);
                break;
            case 0xF9:
                m_cycles = 4;
                SBC(&cpu::get_absolute_y);
                if (m_changed_page) m_cycles++;
                break;
            case 0xFD:
                m_cycles = 4;
                SBC(&cpu::get_absolute_x);
                if (m_changed_page) m_cycles++;
                break;
            case 0xFE:
                m_cycles = 7;
                INC(&cpu::get_absolute_x);
                break;
            default:
                m_cycles = 2;
                /*
                 * this needs to get changed at some point
                 * different nops can have different cycle lengths,
                 * and some illegal m_opcodes are used in some official
                 * games. simply for compatibility, it might make
                 * sense to throw in the more common instances
                 * of these.
                 */
                XXX();
                break;
        }

#ifdef CPU_LOG
        m_log << logLine.str() << " " << std::hex << std::uppercase << std::setfill('0') << std::setw(4) <<
            atoi(m_bytes.str().c_str()) << "\t\t " << regLine.str() << std::endl;
#endif
    }

    m_cycles--;
}

auto cpu::step() -> void
{
    for (int step = 0; step < 3; ++step)
    {
        m_ppu->step();
    }

    if (m_ppu->interr())
    {
        nonmaskable_interrupt();
    }

    if (!m_try_transfer)
    {
        clock();
    }
    else
    {
        if (!m_do_transfer)
        {
            if (m_ticks % 2 != 0)
            {
                m_do_transfer = true;
            }
        }
        else
        {
            if (m_ticks % 2 == 0)
            {
                m_oam_byte = read((static_cast<uint16_t>(m_oam_addr << 8)) | m_oam_index);
            }
            else
            {
                m_ppu->oam_write(m_oam_index, m_oam_byte);
                m_oam_index++;

                if (m_oam_index == 0)
                {
                    m_try_transfer = false;
                    m_do_transfer = false;
                }
            }
        }
    }

    ++m_ticks;
}

auto cpu::connect_controller(std::shared_ptr<controller>& ctrl) -> void
{
    this->m_controller_a = ctrl;
}
