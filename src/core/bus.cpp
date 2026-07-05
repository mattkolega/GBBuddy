#include "bus.h"

#include "common/log.h"

#include "cartridge.h"

u8 RealBus::read(u16 address) {
    if (address <= 0x7FFF) return m_cartridge.romRead(address);
    if (address <= 0xDFFF) return m_memory[address];
    if (address <= 0xFDFF) return m_memory[address - 0x2000];
    if (address <= 0xFF44) return m_memory[address];
    if (address <= 0xFFFF) return m_memory[address];

    log::warn("Attempted read from unmapped memory at address: {:X}", address);
    return 0;
}

void RealBus::write(u16 address, u8 value) {
    if (address <= 0x7FFF) { m_cartridge.romWrite(address, value); return; }
    if (address <= 0xDFFF) { m_memory[address] = value; return; }
    if (address <= 0xFDFF) { m_memory[address - 0x2000] = value; return; }
    if (address <= 0xFFFF) { m_memory[address] = value;  return; }

    log::warn("Illegal write to unmapped memory at address: {:X}", address);
}

u8 MockBus::read(u16 address) {
    if (address <= 0xFFFF) {
        return m_memory[address];
    } else {
        log::warn("Attempted read from unmapped memory at address: {:X}", address);
        return 0;
    }
}

void MockBus::write(u16 address, u8 value) {
    if (address <= 0xFFFF) {
        m_memory[address] = value;
    } else {
        log::warn("Illegal write to unmapped memory at address: {:X}", address);
    }
}
