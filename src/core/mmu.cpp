#include "mmu.h"

#include "../utils/logger.h"

MMU::MMU(GameBoy *gb)
    : gb(gb)
{
}

uint8_t MMU::memoryRead(uint16_t address) {
    if (address >= 0x0000 && address <= 0xFFFF) {
        return rom[address];
    } else {
        Logger::err("{} {:X}", "Invalid address for memory read: ", address);
        return 0x00;
    }
}

void MMU::memoryWrite(uint16_t address, uint8_t value) {
    if (address >= 0x0000 && address <= 0xFFFF) {
        rom[address] = value;
    } else {
        Logger::err("{} {:X}", "Invalid address for memory write: ", address);
    }
}