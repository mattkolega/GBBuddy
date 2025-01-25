#pragma once

#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <vector>

#include "mappers/mapper.h"

class GameBoy;

class NoMBC;

struct CartHeader {
    std::string title;
    uint8_t cartType;
    uint16_t romBanks;  // Each ROM bank is 16KiB
    uint8_t ramBanks;   // Each RAM bank is 8KiB
    uint8_t headerChecksum;
    uint16_t globalChecksum;
};

class Cartridge {
public:
    Cartridge() = delete;
    Cartridge(GameBoy *gb);

    uint8_t romRead(uint16_t address);
    void romWrite(uint16_t address, uint8_t value);

private:
    CartHeader m_cartHeader;

    std::unique_ptr<Mapper> mapper;

    std::vector<uint8_t> m_rom {};
    std::vector<uint8_t> m_ram {};

    GameBoy *m_gb { nullptr };

    friend NoMBC;

    // Opens and loads .gb file
    void loadGBFile();

    // Grabs all useful cartridge header info. Based on https://gbdev.io/pandocs/The_Cartridge_Header.html
    void verifyCartHeader();

    // Sets the Mapper/MBC based on cartridge type specified in header
    void setMapper();
};