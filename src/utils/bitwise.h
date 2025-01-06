#pragma once

#include <cstdint>

namespace Bitwise {
    // Checks if half carry occurred in 8-bit addition
    inline bool checkHalfCarryAdd(uint8_t operand1, uint8_t operand2) {
        return ((((operand1 & 0xF) + (operand2 & 0xF)) & 0x10) == 0x10);
    }

    // Checks if half carry occurred in 16-bit addition
    inline bool checkHalfCarryAdd(uint16_t operand1, uint16_t operand2) {
        return ((((operand1 & 0xFFF) + (operand2 & 0xFFF)) & 0x1000) == 0x1000);
    }

    // Checks if half carry occurred in 8-bit subtraction
    inline bool checkHalfCarrySub(uint8_t operand1, uint8_t operand2) {
        return ((((operand1 & 0xF) - (operand2 & 0xF)) & 0x10) == 0x10);
    }

    // Checks if half carry occurred in 16-bit subtraction
    inline bool checkHalfCarrySub(uint16_t operand1, uint16_t operand2) {
        return ((((operand1 & 0xFFF) - (operand2 & 0xFFF)) & 0x1000) == 0x1000);
    }

    // Gets n-th bit from an 8-bit value
    inline uint8_t getBitInByte(uint8_t value, uint8_t n) {
        return (value >> n) & 0b1;
    }

    // Modifies a specific bit in a 8-bit value
    inline uint8_t modifyBitInByte(uint8_t value, uint8_t n, uint8_t newVal) {
        return (value & ~(1 << n)) | (newVal << n);
    }

    // Grabs the first 4-bits from a byte
    inline uint8_t getFirstNibble(uint8_t value) {
        return (value >> 4) & 0xF;
    }

    // Grabs the second 4-bits from a byte
    inline uint8_t getSecondNibble(uint8_t value) {
        return value & 0xF;
    }

    // Swaps the upper and lower nibbles of a byte
    inline uint8_t swapNibbles(uint8_t value) {
        return (value << 4) | (value >> 4);
    }

    // Joins two bytes together to make a 16-bit value
    inline uint16_t concatBytes(uint8_t low, uint8_t hi) {
        return (hi << 8) | low;
    }
}