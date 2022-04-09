#include <cstdint>
#include <sstream>
#include <fstream>
#include <array>
#include <memory>
#include <functional>
#include "ppu.hpp"

class cpu
{
        // Constants for frequently used numbers
        const int stack_offset = 0x100;
        const int minus_bit = 0x80;

        // Functions for accessing RAM / ROM / etc.
        auto read(uint16_t addr) -> uint8_t;
        auto write(uint16_t addr, uint8_t byte) -> void;

        // General purpose RAM for CPU
        std::array<uint8_t, 0x800> m_ram;

        // ROM file / Cartridge
        std::shared_ptr<cartridge> m_cart = nullptr;
        std::shared_ptr<ppu> m_ppu = nullptr;

        // Type definition for passing addressing mode to instruction
        //using addr_mode_ptr = uint16_t(cpu::*)();
        using addr_mode_ptr = auto (cpu::*)(void) -> void;
        // typedef uint16_t(CPU::*addrModePtr)();
    
        // Used for logging
        std::ofstream m_log;
        std::stringstream m_bytes;

        // Check to see if page boundaries are crossed for variable-length instructions
        bool m_changed_page;

        auto fetch_byte_at_addr() -> uint8_t;
        uint8_t m_fetched_byte = 0x00;

        // Helper variables dictating the what / where of instructions
        uint16_t m_addr_abs = 0x0000;
        uint16_t m_addr_rel = 0x0000;
        uint8_t m_opcode = 0x00;
        uint8_t m_cycles = 0;

        /*
                Registers and pointers, etc.

                Brief aside:
                I've seen program counter be called the
                "instruction pointer," which is honestly
                a much better name for it.
        */
        uint8_t m_accumulator;
        uint8_t m_x_reg, m_y_reg;
        uint16_t m_prog_counter;
        uint8_t m_stack_ptr;
        uint8_t m_stat_reg;

        // Flags used for the m_status register
        enum m_flags
        {
                C = 0b00000001,
                Z = 0b00000010,
                I = 0b00000100,
                D = 0b00001000,
                B = 0b00010000,
                U = 0b00100000,
                V = 0b01000000,
                N = 0b10000000
        };

        auto set_flag(m_flags flag, bool status) -> void;
        auto get_flag(m_flags flag) -> uint8_t;

        // Different addressing modes
        auto get_accumulator() -> void;
        auto get_absolute() -> void;
        auto get_absolute_x() -> void;
        auto get_absolute_y() -> void;
        auto get_immediate() -> void;
        auto get_implied() -> void;
        auto get_indirect() -> void;
        auto get_indirect_x() -> void;
        auto get_indirect_y() -> void;
        auto get_relative() -> void;
        auto get_zeropage() -> void;
        auto get_zeropage_x() -> void;
        auto get_zeropage_y() -> void;
        
        // All the (legal) instructions:
        // Load / Store
        auto LDA(addr_mode_ptr mode) -> void;
        auto LDX(addr_mode_ptr mode) -> void;
        auto LDY(addr_mode_ptr mode) -> void;
        auto STA(addr_mode_ptr mode) -> void;
        auto STX(addr_mode_ptr mode) -> void;
        auto STY(addr_mode_ptr mode) -> void;

        // Arithmetic
        auto ADC(addr_mode_ptr mode) -> void;
        auto SBC(addr_mode_ptr mode) -> void;
        auto INC(addr_mode_ptr mode) -> void;
        auto INX(addr_mode_ptr mode) -> void;
        auto INY(addr_mode_ptr mode) -> void;
        auto DEC(addr_mode_ptr mode) -> void;
        auto DEX(addr_mode_ptr mode) -> void;
        auto DEY(addr_mode_ptr mode) -> void;

        // Shift / Rotate
        auto ASL(addr_mode_ptr mode) -> void;
        auto LSR(addr_mode_ptr mode) -> void;
        auto ROL(addr_mode_ptr mode) -> void;
        auto ROR(addr_mode_ptr mode) -> void;

        // Logical
        auto AND(addr_mode_ptr mode) -> void;
        auto ORA(addr_mode_ptr mode) -> void;
        auto EOR(addr_mode_ptr mode) -> void;

        // Compare / Test
        auto CMP(addr_mode_ptr mode) -> void;
        auto CPX(addr_mode_ptr mode) -> void;
        auto CPY(addr_mode_ptr mode) -> void;
        auto BIT(addr_mode_ptr mode) -> void;

        // Branching
        auto BCC(addr_mode_ptr mode) -> void;
        auto BCS(addr_mode_ptr mode) -> void;
        auto BNE(addr_mode_ptr mode) -> void;
        auto BEQ(addr_mode_ptr mode) -> void;
        auto BPL(addr_mode_ptr mode) -> void;
        auto BMI(addr_mode_ptr mode) -> void;
        auto BVC(addr_mode_ptr mode) -> void;
        auto BVS(addr_mode_ptr mode) -> void;

        // Transfer
        auto TAX(addr_mode_ptr mode) -> void;
        auto TXA(addr_mode_ptr mode) -> void;
        auto TAY(addr_mode_ptr mode) -> void;
        auto TYA(addr_mode_ptr mode) -> void;
        auto TSX(addr_mode_ptr mode) -> void;
        auto TXS(addr_mode_ptr mode) -> void;

        // Stack
        auto PHA(addr_mode_ptr mode) -> void;
        auto PLA(addr_mode_ptr mode) -> void;
        auto PHP(addr_mode_ptr mode) -> void;
        auto PLP(addr_mode_ptr mode) -> void;

        // Subroutines / Jumping
        auto JMP(addr_mode_ptr mode) -> void;
        auto JSR(addr_mode_ptr mode) -> void;
        auto RTS(addr_mode_ptr mode) -> void;
        auto RTI(addr_mode_ptr mode) -> void;

        // Set / Clear
        auto CLC(addr_mode_ptr mode) -> void;
        auto SEC(addr_mode_ptr mode) -> void;
        auto CLD(addr_mode_ptr mode) -> void;
        auto SED(addr_mode_ptr mode) -> void;
        auto CLI(addr_mode_ptr mode) -> void;
        auto SEI(addr_mode_ptr mode) -> void;
        auto CLV(addr_mode_ptr mode) -> void;

        // Misc.
        auto BRK(addr_mode_ptr mode) -> void;
        auto NOP(addr_mode_ptr mode) -> void;
        auto XXX() -> void;

public:
        cpu();

        auto connect_cartridge(std::shared_ptr<cartridge>& cart) -> void;

        auto clock() -> void;
        auto step() -> void;
        auto reset() -> void;
        auto interrupt_request() -> void;
        auto nonmaskable_interrupt() -> void;
};