#pragma once

#include <cstdint>

class Cartridge;

class Mapper {
public:
    Mapper(Cartridge &cart);
    virtual ~Mapper() = default;

    virtual uint8_t romRead(uint16_t address) = 0;
    virtual void romWrite(uint16_t address, uint8_t value) = 0;

protected:
    Cartridge &m_cartridge;
};