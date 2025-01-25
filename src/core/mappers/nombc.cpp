#include "nombc.h"

#include "../cartridge.h"

NoMBC::NoMBC(Cartridge &cart)
    : Mapper(cart)
{
}

uint8_t NoMBC::romRead(uint16_t address) {
    return m_cartridge.m_rom[address];
}

void NoMBC::romWrite(uint16_t address, uint8_t value) {
    m_cartridge.m_rom[address] = value;
}