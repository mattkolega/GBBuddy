#pragma once

#include <cstdint>

#include <common/bitwise.h>
#include <common/logger.h>

class GameBoy;

enum class RegisterType {
    A,
    B,
    C,
    D,
    E,
    F,
    H,
    L,
    AF,
    BC,
    DE,
    HL,
    SP,
};

struct CPUState {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t f;
    uint8_t h;
    uint8_t l;

    uint16_t sp;  // Stack pointer
    uint16_t pc;  // Program counter

    uint8_t ime;  // Interrupt master enable flag

    std::string toString();
};

bool operator==(CPUState &state1, CPUState &state2);

class CPU {
public:
    CPU() = delete;
    CPU(GameBoy *gb);

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
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t f;
    uint8_t h;
    uint8_t l;

    uint16_t sp;  // Stack pointer
    uint16_t pc;  // Program counter

    uint8_t ime;  // Interrupt master enable flag

    GameBoy *gb;

    size_t cycleDelay { 0 };

    // Wrappers for MMU memory access
    uint8_t memoryRead(uint16_t address);
    void memoryWrite(uint16_t address, uint8_t value);

    // 16-bit register getters/setters
    uint16_t getAF();
    void setAF(uint16_t value);
    uint16_t getBC();
    void setBC(uint16_t value);
    uint16_t getDE();
    void setDE(uint16_t value);
    uint16_t getHL();
    void setHL(uint16_t value);

    // Flag getters/setters
    uint8_t getZero();
    void setZero(uint8_t value);
    uint8_t getSubtract();
    void setSubtract(uint8_t value);
    uint8_t getHalfCarry();
    void setHalfCarry(uint8_t value);
    uint8_t getCarry();
    void setCarry(uint8_t value);

    // Stack helpers
    void pushToStack16(uint16_t value);
    uint16_t popStack16();

    // Decodes a short 8-bit opcode. Returns number of cycles
    size_t opDecode();

    // Decodes a long 16-bit (prefixed with $CB) opcode. Returns number of cycles
    size_t opDecodeCB();

    /* ------------ */
    /* INSTRUCTIONS */
    /* ------------ */

    /*
        8-bit Arithmetic and Logic Instructions
    */

    // Adds value and carry bit to accumulator
    void ADC(uint8_t value);

    // Adds value to accumulator
    void ADD(uint8_t value);

    // Bitwise AND of accumualtor and value
    void AND(uint8_t value);

    // Compares accumulator and value
    void CP(uint8_t value);

    // Decrements 8-bit value
    template <RegisterType regType>
    void DEC8() {
        using enum RegisterType;

        uint8_t originalValue {};

        if constexpr (regType == A) {
            originalValue = a;
            a--;
            setZero(a == 0);
        } else if (regType == B) {
            originalValue = b;
            b--;
            setZero(b == 0);
        } else if (regType == C) {
            originalValue = c;
            c--;
            setZero(c == 0);
        } else if (regType == D) {
            originalValue = d;
            d--;
            setZero(d == 0);
        } else if (regType == E) {
            originalValue = e;
            e--;
            setZero(e == 0);
        } else if (regType == H) {
            originalValue = h;
            h--;
            setZero(h == 0);
        } else if (regType == L) {
            originalValue = l;
            l--;
            setZero(l == 0);
        } else if (regType == HL) {
            originalValue = memoryRead(getHL());
            uint8_t newVal = originalValue - 1;
            memoryWrite(getHL(), newVal);
            setZero(newVal == 0);
        } else {
            Logger::err("{}", "Invalid register provided for DEC8 opcode. Must be A, B, C, D, E, H, L or HL");
            return;
        }

        setSubtract(1);
        setHalfCarry(Bitwise::checkHalfCarrySub(originalValue, 1));
    }

    // Increments 8-bit value
    template <RegisterType regType>
    void INC8() {
        using enum RegisterType;

        uint8_t originalValue {};

        if constexpr (regType == A) {
            originalValue = a;
            a++;
            setZero(a == 0);
        } else if (regType == B) {
            originalValue = b;
            b++;
            setZero(b == 0);
        } else if (regType == C) {
            originalValue = c;
            c++;
            setZero(c == 0);
        } else if (regType == D) {
            originalValue = d;
            d++;
            setZero(d == 0);
        } else if (regType == E) {
            originalValue = e;
            e++;
            setZero(e == 0);
        } else if (regType == H) {
            originalValue = h;
            h++;
            setZero(h == 0);
        } else if (regType == L) {
            originalValue = l;
            l++;
            setZero(l == 0);
        } else if (regType == HL) {
            originalValue = memoryRead(getHL());
            uint8_t newVal = originalValue + 1;
            memoryWrite(getHL(), newVal);
            setZero(newVal == 0);
        } else {
            Logger::err("{}", "Invalid register provided for INC8 opcode. Must be A, B, C, D, E, H, L or HL");
            return;
        }

        setSubtract(0);
        setHalfCarry(Bitwise::checkHalfCarryAdd(originalValue, 1));
    }

    // Bitwise OR of accumulator and value
    void OR(uint8_t value);

    // Subtracts value and carry bit from accumulator
    void SBC(uint8_t value);

    // Subtracts value from accumulator
    void SUB(uint8_t value);

    // Bitwise XOR of accumulator and value
    void XOR(uint8_t value);

    /*
        16-bit Arithmetic Instructions
    */

    // Adds value to HL
    void ADD(uint16_t value);

    // Decrements 16-bit value
    template <RegisterType regType>
    void DEC16() {
        using enum RegisterType;

        if constexpr (regType == AF) {
            setAF(getAF()-1);
        } else if (regType == BC) {
            setBC(getBC()-1);
        } else if (regType == DE) {
            setDE(getDE()-1);
        } else if (regType == HL) {
            setHL(getHL()-1);
        } else if (regType == SP) {
            sp--;
        } else {
            Logger::err("{}", "Invalid register provided for DEC16 opcode. Must be AF, BC, DE, HL or SP");
        }
    }

    // Increments 16-bit value
    template <RegisterType regType>
    void INC16() {
        using enum RegisterType;

        if constexpr (regType == AF) {
            setAF(getAF()+1);
        } else if (regType == BC) {
            setBC(getBC()+1);
        } else if (regType == DE) {
            setDE(getDE()+1);
        } else if (regType == HL) {
            setHL(getHL()+1);
        } else if (regType == SP) {
            sp++;
        } else {
            Logger::err("{}", "Invalid register provided for INC16 opcode. Must be AF, BC, DE, HL or SP");
        }
    }

    /*
        Bit Operation Instructions
    */

    // Checks if bit is set
    void BIT(uint8_t bitPos, uint8_t value);

    // Resets bit in value
    template <RegisterType regType>
    void RES(uint8_t bitPos) {
        using enum RegisterType;

        if constexpr (regType == A) {
            a = Bitwise::modifyBitInByte(a, bitPos, 0);
        } else if (regType == B) {
            b = Bitwise::modifyBitInByte(b, bitPos, 0);
        } else if (regType == C) {
            c = Bitwise::modifyBitInByte(c, bitPos, 0);
        } else if (regType == D) {
            d = Bitwise::modifyBitInByte(d, bitPos, 0);
        } else if (regType == E) {
            e = Bitwise::modifyBitInByte(e, bitPos, 0);
        } else if (regType == H) {
            h = Bitwise::modifyBitInByte(h, bitPos, 0);
        } else if (regType == L) {
            l = Bitwise::modifyBitInByte(l, bitPos, 0);
        } else if (regType == HL) {
            auto newValue = Bitwise::modifyBitInByte(memoryRead(getHL()), bitPos, 0);
            memoryWrite(getHL(), newValue);
        } else {
            Logger::err("{}", "Invalid register provided for RES opcode. Must be A, B, C, D, E, H, L or HL");
        }
    }

    // Sets bit
    template <RegisterType regType>
    void SET(uint8_t bitPos) {
        using enum RegisterType;

        if constexpr (regType == A) {
            a = Bitwise::modifyBitInByte(a, bitPos, 1);
        } else if (regType == B) {
            b = Bitwise::modifyBitInByte(b, bitPos, 1);
        } else if (regType == C) {
            c = Bitwise::modifyBitInByte(c, bitPos, 1);
        } else if (regType == D) {
            d = Bitwise::modifyBitInByte(d, bitPos, 1);
        } else if (regType == E) {
            e = Bitwise::modifyBitInByte(e, bitPos, 1);
        } else if (regType == H) {
            h = Bitwise::modifyBitInByte(h, bitPos, 1);
        } else if (regType == L) {
            l = Bitwise::modifyBitInByte(l, bitPos, 1);
        } else if (regType == HL) {
            auto newValue = Bitwise::modifyBitInByte(memoryRead(getHL()), bitPos, 1);
            memoryWrite(getHL(), newValue);
        } else {
            Logger::err("{}", "Invalid register provided for SET opcode. Must be A, B, C, D, E, H, L or HL");
        }
    }

    // Swaps upper 4 bits and lower 4 bits of value
    template <RegisterType regType>
    void SWAP() {
        using enum RegisterType;

        if constexpr (regType == A) {
            a = Bitwise::swapNibbles(a);
            setZero(a == 0);
        } else if (regType == B) {
            b = Bitwise::swapNibbles(b);
            setZero(b == 0);
        } else if (regType == C) {
            c = Bitwise::swapNibbles(c);
            setZero(c == 0);
        } else if (regType == D) {
            d = Bitwise::swapNibbles(d);
            setZero(d == 0);
        } else if (regType == E) {
            e = Bitwise::swapNibbles(e);
            setZero(e == 0);
        } else if (regType == H) {
            h = Bitwise::swapNibbles(h);
            setZero(h == 0);
        } else if (regType == L) {
            l = Bitwise::swapNibbles(l);
            setZero(l == 0);
        } else if (regType == HL) {
            auto newValue = Bitwise::swapNibbles(memoryRead(getHL()));
            memoryWrite(getHL(), newValue);
            setZero(newValue == 0);
        } else {
            Logger::err("{}", "Invalid register provided for SWAP opcode. Must be A, B, C, D, E, H, L or HL");
            return;
        }

        setSubtract(0);
        setHalfCarry(0);
        setCarry(0);
    }

    /*
        Bit Shift Instructions
    */

    // Rotates carry flag + value left
    uint8_t RL(uint8_t value);

    // Rotates carry flag + register left
    template <RegisterType regType>
    void RL_r8() {
        using enum RegisterType;

        if constexpr (regType == A) {
            a = RL(a);
            setZero(a == 0);
        } else if (regType == B) {
            b = RL(b);
            setZero(b == 0);
        } else if (regType == C) {
            c = RL(c);
            setZero(c == 0);
        } else if (regType == D) {
            d = RL(d);
            setZero(d == 0);
        } else if (regType == E) {
            e = RL(e);
            setZero(e == 0);
        } else if (regType == H) {
            h = RL(h);
            setZero(h == 0);
        } else if (regType == L) {
            l = RL(l);
            setZero(l == 0);
        } else {
            Logger::err("{}", "Invalid register provided for RL opcode. Must be A, B, C, D, E, H, or L");
            return;
        }

        setSubtract(0);
        setHalfCarry(0);
    }

    // Rotates carry flag + value at address HL left
    void RL_HL();

    // Rotates carry flag + accumulator left
    void RLA();

    // Rotates value left. Bit 7 is stored in carry flag
    uint8_t RLC(uint8_t value);

    // Rotates register left
    template <RegisterType regType>
    void RLC_r8() {
        using enum RegisterType;

        if constexpr (regType == A) {
            a = RLC(a);
            setZero(a == 0);
        } else if (regType == B) {
            b = RLC(b);
            setZero(b == 0);
        } else if (regType == C) {
            c = RLC(c);
            setZero(c == 0);
        } else if (regType == D) {
            d = RLC(d);
            setZero(d == 0);
        } else if (regType == E) {
            e = RLC(e);
            setZero(e == 0);
        } else if (regType == H) {
            h = RLC(h);
            setZero(h == 0);
        } else if (regType == L) {
            l = RLC(l);
            setZero(l == 0);
        } else {
            Logger::err("{}", "Invalid register provided for RLC opcode. Must be A, B, C, D, E, H, or L");
            return;
        }

        setSubtract(0);
        setHalfCarry(0);
    }

    // Rotates value at address HL left
    void RLC_HL();

    // Rotates accumulator left
    void RLCA();

    // Rotates carry flag + value right
    uint8_t RR(uint8_t value);

    // Rotates carry flag + register right
    template <RegisterType regType>
    void RR_r8() {
        using enum RegisterType;

        if constexpr (regType == A) {
            a = RR(a);
            setZero(a == 0);
        } else if (regType == B) {
            b = RR(b);
            setZero(b == 0);
        } else if (regType == C) {
            c = RR(c);
            setZero(c == 0);
        } else if (regType == D) {
            d = RR(d);
            setZero(d == 0);
        } else if (regType == E) {
            e = RR(e);
            setZero(e == 0);
        } else if (regType == H) {
            h = RR(h);
            setZero(h == 0);
        } else if (regType == L) {
            l = RR(l);
            setZero(l == 0);
        } else {
            Logger::err("{}", "Invalid register provided for RR opcode. Must be A, B, C, D, E, H, or L");
            return;
        }

        setSubtract(0);
        setHalfCarry(0);
    }

    // Rotates carry flag + value at address HL right
    void RR_HL();

    // Rotates carry flag + accumulator right
    void RRA();

    // Rotates value right. Bit 0 is stored in carry flag
    uint8_t RRC(uint8_t value);

    // Rotates register right
    template <RegisterType regType>
    void RRC_r8() {
        using enum RegisterType;

        if constexpr (regType == A) {
            a = RRC(a);
            setZero(a == 0);
        } else if (regType == B) {
            b = RRC(b);
            setZero(b == 0);
        } else if (regType == C) {
            c = RRC(c);
            setZero(c == 0);
        } else if (regType == D) {
            d = RRC(d);
            setZero(d == 0);
        } else if (regType == E) {
            e = RRC(e);
            setZero(e == 0);
        } else if (regType == H) {
            h = RRC(h);
            setZero(h == 0);
        } else if (regType == L) {
            l = RRC(l);
            setZero(l == 0);
        } else {
            Logger::err("{}", "Invalid register provided for RRC opcode. Must be A, B, C, D, E, H, or L");
            return;
        }

        setSubtract(0);
        setHalfCarry(0);
    }

    // Rotates value at address HL right
    void RRC_HL();

    // Rotates accumulator right
    void RRCA();

    // Shifts left arithmetically. Bit 0 is zeroed
    template <RegisterType regType>
    void SLA() {
        using enum RegisterType;

        if constexpr (regType == A) {
            setCarry(Bitwise::getBitInByte(a, 7));
            a <<= 1;
            setZero(a == 0);
        } else if (regType == B) {
            setCarry(Bitwise::getBitInByte(b, 7));
            b <<= 1;
            setZero(b == 0);
        } else if (regType == C) {
            setCarry(Bitwise::getBitInByte(c, 7));
            c <<= 1;
            setZero(c == 0);
        } else if (regType == D) {
            setCarry(Bitwise::getBitInByte(d, 7));
            d <<= 1;
            setZero(d == 0);
        } else if (regType == E) {
            setCarry(Bitwise::getBitInByte(e, 7));
            e <<= 1;
            setZero(e == 0);
        } else if (regType == H) {
            setCarry(Bitwise::getBitInByte(h, 7));
            h <<= 1;
            setZero(h == 0);
        } else if (regType == L) {
            setCarry(Bitwise::getBitInByte(l, 7));
            l <<= 1;
            setZero(l == 0);
        } else if (regType == HL) {
            auto newVal = memoryRead(getHL());
            setCarry(Bitwise::getBitInByte(newVal, 7));
            newVal <<= 1;
            memoryWrite(getHL(), newVal);
            setZero(newVal == 0);
        } else {
            Logger::err("{}", "Invalid register provided for SLA opcode. Must be A, B, C, D, E, H, L or HL");
            return;
        }

        setSubtract(0);
        setHalfCarry(0);
    }

    // Shifts right arithmetically. Bit 7 remains the same
    template <RegisterType regType>
    void SRA() {
        using enum RegisterType;

        if constexpr (regType == A) {
            setCarry(Bitwise::getBitInByte(a, 0));
            uint8_t bit7 = Bitwise::getBitInByte(a, 7);
            a = (a >> 1) | (bit7 << 7);
            setZero(a == 0);
        } else if (regType == B) {
            setCarry(Bitwise::getBitInByte(b, 0));
            uint8_t bit7 = Bitwise::getBitInByte(b, 7);
            b = (b >> 1) | (bit7 << 7);
            setZero(b == 0);
        } else if (regType == C) {
            setCarry(Bitwise::getBitInByte(c, 0));
            uint8_t bit7 = Bitwise::getBitInByte(c, 7);
            c = (c >> 1) | (bit7 << 7);
            setZero(c == 0);
        } else if (regType == D) {
            setCarry(Bitwise::getBitInByte(d, 0));
            uint8_t bit7 = Bitwise::getBitInByte(d, 7);
            d = (d >> 1) | (bit7 << 7);
            setZero(d == 0);
        } else if (regType == E) {
            setCarry(Bitwise::getBitInByte(e, 0));
            uint8_t bit7 = Bitwise::getBitInByte(e, 7);
            e = (e >> 1) | (bit7 << 7);
            setZero(e == 0);
        } else if (regType == H) {
            setCarry(Bitwise::getBitInByte(h, 0));
            uint8_t bit7 = Bitwise::getBitInByte(h, 7);
            h = (h >> 1) | (bit7 << 7);
            setZero(h == 0);
        } else if (regType == L) {
            setCarry(Bitwise::getBitInByte(l, 0));
            uint8_t bit7 = Bitwise::getBitInByte(l, 7);
            l = (l >> 1) | (bit7 << 7);
            setZero(l == 0);
        } else if (regType == HL) {
            auto newVal = memoryRead(getHL());
            setCarry(Bitwise::getBitInByte(newVal, 0));
            uint8_t bit7 = Bitwise::getBitInByte(newVal, 7);
            newVal = (newVal >> 1) | (bit7 << 7);
            memoryWrite(getHL(), newVal);
            setZero(newVal == 0);
        } else {
            Logger::err("{}", "Invalid register provided for SRA opcode. Must be A, B, C, D, E, H, L or HL");
            return;
        }

        setSubtract(0);
        setHalfCarry(0);
    }

    // Shifts right logically. Bit 7 is zeroed
    template <RegisterType regType>
    void SRL() {
        using enum RegisterType;

        if constexpr (regType == A) {
            setCarry(Bitwise::getBitInByte(a, 0));
            a >>= 1;
            setZero(a == 0);
        } else if (regType == B) {
            setCarry(Bitwise::getBitInByte(b, 0));
            b >>= 1;
            setZero(b == 0);
        } else if (regType == C) {
            setCarry(Bitwise::getBitInByte(c, 0));
            c >>= 1;
            setZero(c == 0);
        } else if (regType == D) {
            setCarry(Bitwise::getBitInByte(d, 0));
            d >>= 1;
            setZero(d == 0);
        } else if (regType == E) {
            setCarry(Bitwise::getBitInByte(e, 0));
            e >>= 1;
            setZero(e == 0);
        } else if (regType == H) {
            setCarry(Bitwise::getBitInByte(h, 0));
            h >>= 1;
            setZero(h == 0);
        } else if (regType == L) {
            setCarry(Bitwise::getBitInByte(l, 0));
            l >>= 1;
            setZero(l == 0);
        } else if (regType == HL) {
            auto newVal = memoryRead(getHL());
            setCarry(Bitwise::getBitInByte(newVal, 0));
            newVal >>= 1;
            memoryWrite(getHL(), newVal);
            setZero(newVal == 0);
        } else {
            Logger::err("{}", "Invalid register provided for SRL opcode. Must be A, B, C, D, E, H, L or HL");
            return;
        }

        setSubtract(0);
        setHalfCarry(0);
    }

    /*
        Load Instructions
    */

    // Loads 8-bit value into 8-bit register
    template <RegisterType regType>
    void LD_r8(uint8_t value) {
        using enum RegisterType;

        if constexpr (regType == A) {
            a = value;
        } else if (regType == B) {
            b = value;
        } else if (regType == C) {
            c = value;
        } else if (regType == D) {
            d = value;
        } else if (regType == E) {
            e = value;
        } else if (regType == H) {
            h = value;
        } else if (regType == L) {
            l = value;
        } else {
            Logger::err("{}", "Invalid register provided for LD_r8 opcode. Must be A, B, C, D, E, H or L");
        }
    }

    // Loads 16-bit value into 16-bit register
    template <RegisterType regType>
    void LD_r16(uint16_t value) {
        using enum RegisterType;

        if constexpr (regType == AF) {
            setAF(value);
        } else if (regType == BC) {
            setBC(value);
        } else if (regType == DE) {
            setDE(value);
        } else if (regType == HL) {
            setHL(value);
        } else {
            Logger::err("{}", "Invalid register provided for LD_r16 opcode. Must be AF, BC, DE or HL");
        }
    }

    // Loads 8-bit value into byte pointed to by HL
    void LD_HL(uint8_t value);

    // Gets value from accumulator and writes to memory at provided address
    void LD_n16_A(uint16_t address);

    // Gets value at memory address and writes to accumulator
    void LD_A_n16(uint16_t address);

    // Gets value from accumulator and writes to memory.
    // Memory address must be between $FF00 and $FFFF
    void LDH_n16_A(uint8_t lowByte);

    // Gets value from accumulator and writes to memory with address $FF00 + offset
    void LDH_C_A();

    // Gets value from memory and stores in accumulator.
    // Memory address must be between $FF00 and $FFFF
    void LDH_A_n16(uint8_t lowByte);

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

    /*
        Jumps and Subroutines
    */

    // Calls address
    void CALL(uint16_t address);

    // Jumps to an address
    void JP(uint16_t address);

    // Performs a relative jump to an address
    void JR(int8_t offset);

    // Returns from a subroutine
    void RET();

    // Returns from subroutine and enables interrupts
    void RETI();

    // Calls address vec
    void RST(uint8_t vec);

    /*
        Stack Operation Instructions
    */

    // Adds SP to HL
    void ADD_HL_SP();

    // Adds signed 8-bit value to SP
    void ADD_SP_e8(int8_t value);

    // Loads stack pointer with 16-bit value
    void LD_SP_n16(uint16_t value);

    // Writes SP & $FF to address and SP >> 8 to address+1
    void LD_n16_SP(uint16_t address);

    // Adds signed 8-bit value to SP and stores result in HL
    void LD_HL_SP(int8_t value);

    // Loads HL into SP
    void LD_SP_HL();

    // Gets 16-bit register from stack
    template <RegisterType regType>
    void POP() {
        using enum RegisterType;

        if constexpr (regType == AF) {
            setAF(popStack16());
        } else if (regType == BC) {
            setBC(popStack16());
        } else if (regType == DE) {
            setDE(popStack16());
        } else if (regType == HL) {
            setHL(popStack16());
        } else {
            Logger::err("{}" , "Unsupported register for POP operation. Must be AF, BC, DE or HL");
        }
    }

    // Writes 16-bit register to stack
    template <RegisterType regType>
    void PUSH() {
        using enum RegisterType;

        if constexpr (regType == AF) {
            pushToStack16(getAF());
        } else if (regType == BC) {
            pushToStack16(getBC());
        } else if (regType == DE) {
            pushToStack16(getDE());
        } else if (regType == HL) {
            pushToStack16(getHL());
        } else {
            Logger::err("{}" , "Unsupported register for PUSH operation. Must be AF, BC, DE or HL");
        }
    }

    /*
        Miscellaneous Instructions
    */

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