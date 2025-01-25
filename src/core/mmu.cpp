#include "mmu.h"

#include "cartridge.h"
#include "gameboy.h"
#include "../utils/logger.h"

MMU::MMU(GameBoy *gb)
    : m_gb(gb)
{
}

uint8_t MMU::memoryRead(uint16_t address) {
    if (address >= 0x0000 && address <= 0x7FFF) {
        return m_gb->cartridge.romRead(address);
    } else if (address >= 0x8000 && address <= 0xFFFF) {
        return m_memory[address];
    } else {
        Logger::err("{} {:X}", "Invalid address for memory read: ", address);
        return 0x00;
    }
}

void MMU::memoryWrite(uint16_t address, uint8_t value) {
    if (address >= 0x0000 && address <= 0x7FFF) {
        m_gb->cartridge.romWrite(address, value);
    } else if (address >= 0x8000 && address <= 0xFFFF) {
        m_memory[address] = value;
    } else {
        Logger::err("{} {:X}", "Invalid address for memory write: ", address);
    }
}