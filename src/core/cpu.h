#pragma once

#include <string>

#include "common/types.h"

class GameBoy;


struct CPUState {
    u8 a {}, f {}, b {}, c {}, d {}, e {}, h {}, l {};

    u16 sp; // Stack pointer
    u16 pc; // Program counter

    u8 ime;  // Interrupt master enable flag

    std::string toString();
};

bool operator==(CPUState& state1, CPUState& state2);

enum class CPUFlags : u8 {
    C = 4, // Carry
    H = 5, // Half Carry
    N = 6, // Subtract
    Z = 7, // Zero
};

union RegisterPair {
    u16 full;
    struct { u8 lo; u8 hi; };
};

enum class Reg8  { A, F, B, C, D, E, H, L, HLMem };
enum class Reg16 { AF, BC, DE, HL, SP };

class Bus;

class CPU {
public:
    int cycleDelay {0};

    CPU(Bus& bus)
        : m_bus(bus) {};

    // Executes a single opcode. Returns number of cycles
    void step();

    // Sets CPU state
    void setState(CPUState state);

    // Returns CPU state in an easy to access object
    CPUState getState();

    // Prints CPU state to console
    void printState();

private:
    // Registers
    RegisterPair m_af {};
    RegisterPair m_bc {};
    RegisterPair m_de {};
    RegisterPair m_hl {};

    u16 m_pc {}; // Stack pointer
    u16 m_sp {}; // Program counter

    u8 m_ime;  // Interrupt master enable flag

    Bus& m_bus;

    usize m_cycleDelay {0};

    u8 readReg8(Reg8 reg) const;
    u16 readReg16(Reg16 reg) const;
    void writeReg8(Reg8 reg, u8 value);
    void writeReg16(Reg16 reg, u16 value);

    // Flag getters
    u8 getCarry() const;
    u8 getHalfCarry() const;
    u8 getSubtract() const;
    u8 getZero() const;

    // Flag setters
    void setCarry(bool value);
    void setHalfCarry(bool  value);
    void setSubtract(bool value);
    void setZero(bool value);

    // Stack helpers
    void pushToStack(u16 value);
    u16 popStack();

    // Decodes a short 8-bit opcode.
    void decode(u8 opcode);

    // Decodes a long 16-bit (prefixed with $CB) opcode.
    void decodeCB(u8 opcode);

    // ------------
    // INSTRUCTIONS
    // ------------

    // - 8-bit Arithmetic and Logic Instructions

    // Adds value and carry bit to accumulator
    void ADC(uint8_t value);

    // Adds value to accumulator
    void ADD8(uint8_t value);

    // ANDs value with accumulator
    void AND(uint8_t value);

    // Compares accumulator and value
    void CP(uint8_t value);

    // Decrements 8-bit value
    void DEC8(Reg8 reg);

    // Increments 8-bit value
    void INC8(Reg8 reg);

    // ORs value with accumulator
    void OR(uint8_t value);

    // Subtracts value and carry bit from accumulator
    void SBC(uint8_t value);

    // Subtracts value from accumulator
    void SUB(uint8_t value);

    // XORs value with accumulator
    void XOR(uint8_t value);

    // - 16-bit Arithmetic Instructions

    // Adds value to HL
    void ADD16(Reg16 reg);

    // Decrements 16-bit value
    void DEC16(Reg16 reg);

    // Increments 16-bit value
    void INC16(Reg16 reg);

    // - Bit Operation Instructions

    // Checks if bit is set
    void BIT(Reg8 reg, u8 bitPos);

    // Resets bit in value
    void RES(Reg8 reg, u8 bitPos);

    // Sets bit
    void SET(Reg8 reg, u8 bitPos);

    // Swaps upper 4 bits and lower 4 bits of value
    void SWAP(Reg8 reg);

    // - Bit Shift Instructions

    // Rotates carry flag + value left
    u8 RL(u8 value);

    // Rotates carry flag + register left
    void RL_r8(Reg8 reg);

    // Rotates carry flag + accumulator left
    void RLA();

    // Rotates value left. Bit 7 is stored in carry flag
    u8 RLC(u8 value);

    // Rotates register left
    void RLC_r8(Reg8 reg);

    // Rotates accumulator left
    void RLCA();

    // Rotates carry flag + value right
    u8 RR(u8 value);

    // Rotates carry flag + register right
    void RR_r8(Reg8 reg);

    // Rotates carry flag + accumulator right
    void RRA();

    // Rotates value right. Bit 0 is stored in carry flag
    u8 RRC(u8 value);

    // Rotates register right
    void RRC_r8(Reg8 reg);

    // Rotates accumulator right
    void RRCA();

    // Shifts left arithmetically. Bit 0 is zeroed
    void SLA(Reg8 reg);

    // Shifts right arithmetically. Bit 7 remains the same
    void SRA(Reg8 reg);

    // Shifts right logically. Bit 7 is zeroed
    void SRL(Reg8 reg);

    // - Load Instructions

    // Loads 8-bit value into 8-bit register
    void LD8(Reg8 reg, u8 value);

    // Loads 16-bit value into 16-bit register
    void LD16(Reg16 reg, u16 value);

    // Gets value from accumulator and writes to memory at provided address
    void LD_n16_A(u16 address);

    // Gets value at memory address and writes to accumulator
    void LD_A_n16(u16 address);

    // Gets value from accumulator and writes to memory.
    // Memory address must be between $FF00 and $FFFF
    void LDH_n16_A(u8 lowByte);

    // Gets value from accumulator and writes to memory with address $FF00 + offset
    void LDH_C_A();

    // Gets value from memory and stores in accumulator.
    // Memory address must be between $FF00 and $FFFF
    void LDH_A_n16(u8 lowByte);

    // Gets value from memory at address $FF00 + offset and writes to accumulator
    void LDH_A_C();

    // Gets value from memory at address stored in HL and writes to accumulator. Increments HL afterwards
    void LD_HLI_A();

    // Gets value from memory at address stored in HL and writes to accumulator. Decrements HL afterwards
    void LD_HLD_A();

    // Gets value from accumulator and writes to memory at address HL. Increments HL afterwards
    void LD_A_HLI();

    // Gets value from accumulator and writes to memory at address HL. Decrements HL afterwards
    void LD_A_HLD();

    // - Jumps and Subroutines

    // Calls address
    void CALL(bool condition = true);

    // Jumps to an address
    void JP(bool condition = true);

    // Jumps to the address stored in HL
    void JP_HL();

    // Performs a relative jump to an address
    void JR(bool condition = true);

    // Returns from a subroutine
    void RET(bool condition = true);

    // Returns from subroutine and enables interrupts
    void RETI();

    // Calls address vec
    void RST(u8 vec);

    // - Stack Operation Instructions

    // Adds SP to HL
    void ADD_HL_SP();

    // Adds signed 8-bit value to SP
    void ADD_SP_e8(i8 value);

    // Loads stack pointer with 16-bit value
    void LD_SP_n16(u16 value);

    // Writes SP & $FF to address and SP >> 8 to address+1
    void LD_a16_SP(u16 address);

    // Adds signed 8-bit value to SP and stores result in HL
    void LD_HL_SP(i8 value);

    // Loads HL into SP
    void LD_SP_HL();

    // Gets 16-bit register from stack
    void POP(Reg16 reg);

    // Writes 16-bit register to stack
    void PUSH(Reg16 reg);

    // - Miscellaneous Instructions

    // Completes carry flag
    void CCF();

    // Complements accumulator
    void CPL();

    // Gets BCD representation of value in accumulator
    void DAA();

    // Disables interrupts
    void DI();

    // Enables interrupts
    void EI();

    // Enters CPU low-power mode
    void HALT();

    // Performs no operation
    void NOP();

    // Sets carry flag
    void SCF();

    // Stops GB execution
    void STOP();
};
