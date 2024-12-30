#pragma once

#include <cstdint>

namespace Bitwise {
    // Checks if half carry occurred in 8-bit addition
    bool checkHalfCarryAdd(uint8_t operand1, uint8_t operand2);

    // Checks if half carry occurred in 16-bit addition
    bool checkHalfCarryAdd(uint16_t operand1, uint16_t operand2);

    // Checks if half carry occurred in 8-bit subtraction
    bool checkHalfCarrySub(uint8_t operand1, uint8_t operand2);

    // Checks if half carry occurred in 16-bit subtraction
    bool checkHalfCarrySub(uint16_t operand1, uint16_t operand2);

    // Gets n-th bit from an 8-bit value
    uint8_t getBitInByte(uint8_t value);

    // Modifies a specific bit in a 8-bit value
    uint8_t modifyBitInByte(uint8_t value, uint8_t n, uint8_t newVal);

    // Grabs the first 4-bits from a byte
    uint8_t getFirstNibble(uint8_t value);

    // Grabs the second 4-bits from a byte
    uint8_t getSecondNibble(uint8_t value);

    // Swaps the upper and lower nibbles of a byte
    uint8_t swapNibbles(uint8_t value);

    // Joins two bytes together to make a 16-bit value
    uint16_t concatBytes(uint8_t low, uint8_t hi);
}