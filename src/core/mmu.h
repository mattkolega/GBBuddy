#pragma once

#include <array>

class GameBoy;

class MMU {
private:
    std::array<uint8_t, 1024 * 64> m_memory {};  // Temporary 64KB will gradually be reduced to actual RAM size as proper memory mapping is introduced

    GameBoy *m_gb { nullptr };

public:
    MMU() = delete;
    MMU(GameBoy *gb);

    uint8_t memoryRead(uint16_t address);
    void memoryWrite(uint16_t address, uint8_t value);
};