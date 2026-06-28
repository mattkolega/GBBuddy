#pragma once

#include <bit>

#include "types.h"

namespace bits {
    // Checks if half carry occurred in 8-bit addition
    constexpr bool checkHalfCarryAdd(u8 operand1, u8 operand2, u8 carry = 0) {
        return ((operand1 & 0xF) + (operand2 & 0xF) + (carry & 0b1)) > 0xF;
    }

    // Checks if half carry occurred in 16-bit addition
    constexpr bool checkHalfCarryAdd(u16 operand1, u16 operand2, u8 carry = 0) {
        return ((operand1 & 0xFFF) + (operand2 & 0xFFF) + (carry & 0b1)) > 0xFFF;
    }

    // Checks if half carry occurred in 8-bit subtraction
    constexpr bool checkHalfCarrySub(u8 operand1, u8 operand2, u8 carry = 0) {
        return (operand1 & 0xF) < ((operand2 & 0xF) + (carry & 0b1));
    }

    // Checks if half carry occurred in 16-bit subtraction
    constexpr bool checkHalfCarrySub(u16 operand1, u16 operand2, u8 carry = 0) {
        return (operand1 & 0xFFF) < ((operand2 & 0xFFF) + (carry & 0b1));
    }

    // Gets n-th bit from an 8-bit value
    constexpr u8 getBitInByte(u8 value, u8 n) {
        return (value >> n) & 0b1;
    }

    // Modifies a specific bit in a 8-bit value
    constexpr u8 modifyBitInByte(u8 value, u8 n, u8 newVal) {
        return (value & ~(1 << n)) | ((newVal & 1) << n);
    }

    // Grabs the first 4-bits from a byte
    constexpr u8 getFirstNibble(u8 value) {
        return (value >> 4) & 0xF;
    }

    // Grabs the second 4-bits from a byte
    constexpr u8 getSecondNibble(u8 value) {
        return value & 0xF;
    }

    // Swaps the upper and lower nibbles of a byte
    constexpr u8 swapNibbles(u8 value) {
        return (value << 4) | (value >> 4);
    }

    // Joins two bytes together to make a 16-bit value
    constexpr u16 concatBytes(u8 low, u8 hi) {
        return (hi << 8) | low;
    }

    // Checks the sign of a byte. True if 7th bit set, false if not
    constexpr bool isNegative(u8 value) {
        return getBitInByte(value, 7);
    }

    // Checks parity of a byte. True if even, false if odd
    constexpr bool checkParity(u8 value) {
        return std::popcount(value) % 2 == 0;
    }
}
