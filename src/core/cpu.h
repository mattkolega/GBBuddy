#pragma once

#include <cstdint>

class GameBoy;

class CPU {
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
    void pushToStack16(uint8_t value);
    uint16_t popStack16();

public:
    CPU() = delete;
    CPU(GameBoy *gb);
};