#include "bitwise.h"

namespace Bitwise {
    bool checkHalfCarryAdd(uint8_t operand1, uint8_t operand2) {
        return ((((operand1 & 0xF) + (operand2 & 0xF)) & 0x10) == 0x10);
    }

    bool checkHalfCarryAdd(uint16_t operand1, uint16_t operand2) {
        return ((((operand1 & 0xFFF) + (operand2 & 0xFFF)) & 0x1000) == 0x1000);
    }

    bool checkHalfCarrySub(uint8_t operand1, uint8_t operand2) {
        return ((((operand1 & 0xF) - (operand2 & 0xF)) & 0x10) == 0x10);
    }

    bool checkHalfCarrySub(uint16_t operand1, uint16_t operand2) {
        return ((((operand1 & 0xFFF) - (operand2 & 0xFFF)) & 0x1000) == 0x1000);
    }

    uint8_t getBitInByte(uint8_t value, uint8_t n) {
        return (value >> n) & 0b1;
    }

    uint8_t modifyBitInByte(uint8_t value, uint8_t n, uint8_t newVal) {
        return (value & ~(1 << n)) | (newVal << n);
    }

    uint8_t getFirstNibble(uint8_t value) {
        return (value >> 4) & 0xF;
    }

    uint8_t getSecondNibble(uint8_t value) {
        return value & 0xF;
    }

    uint8_t swapNibbles(uint8_t value) {
        return value << 4 | value >> 4;
    }

    uint16_t concatBytes(uint8_t low, uint8_t hi) {
        return hi << 8 | low;
    }
}