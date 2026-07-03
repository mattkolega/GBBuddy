#include "bus.h"

#include "common/log.h"

#include "cartridge.h"

u8 RealBus::read(u16 address) {
    if (address <= 0x7FFF) {
        return m_cartridge.romRead(address);
    } else if (address <= 0xDFFF) {
        return m_memory[address];
    } else if (address <= 0xFDFF) {
        return m_memory[address - 0x2000];
    } else if (address <= 0xFF44) {
        return m_memory[address];
    } else if (address <= 0xFFFF) {
        return m_memory[address];
    } else {
        log::warn("Attempted read from unmapped memory at address: {:X}", address);
        return 0;
    }
}

void RealBus::write(u16 address, u8 value) {
    if (address <= 0x7FFF) {
       m_cartridge.romWrite(address, value);
    } else if (address <= 0xDFFF) {
        m_memory[address] = value;
    } else if (address <= 0xFDFF) {
        m_memory[address - 0x2000] = value;
    } else if (address <= 0xFFFF) {
        m_memory[address] = value;
    } else {
        log::warn("Illegal write to unmapped memory at address: {:X}", address);
    }
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