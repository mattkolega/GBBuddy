#pragma once

#include <cstdint>

#include "mapper.h"

class NoMBC : public Mapper {
public:
    NoMBC(Cartridge &cart);

    uint8_t romRead(uint16_t address) override;
    void romWrite(uint16_t address, uint8_t value) override;
};