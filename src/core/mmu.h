#pragma once

#include <array>

class GameBoy;

// Abstract implementation of the MMU
class MMU {
public:
    virtual ~MMU() = default;

    virtual uint8_t memoryRead(uint16_t address) = 0;
    virtual void memoryWrite(uint16_t address, uint8_t value) = 0;
};

// Proper MMU implementation to be used in the emulator
class RealMMU : public MMU {
private:
    std::array<uint8_t, 1024 * 64> m_memory {};  // Temporary 64KB will gradually be reduced to actual RAM size as proper memory mapping is introduced

    GameBoy *m_gb { nullptr };

public:
    RealMMU() = delete;
    RealMMU(GameBoy *gb);

    uint8_t memoryRead(uint16_t address);
    void memoryWrite(uint16_t address, uint8_t value);
};

// Mock MMU implementation ONLY to be used for CPU testing
class MockMMU : public MMU {
private:
    std::array<uint8_t, 1024 * 64> m_memory {};  // Flat 64kb of memory. No registers or mapping

public:
    uint8_t memoryRead(uint16_t address);
    void memoryWrite(uint16_t address, uint8_t value);
};