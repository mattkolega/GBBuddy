#pragma once

#include <cstdint>
#include <string>
#include <vector>

class GameBoy;

struct CartHeader {
    std::string title;
    uint8_t cartType;
    uint16_t romBanks;  // Each ROM bank is 16KiB
    uint8_t ramBanks;   // Each RAM bank is 8KiB
    uint8_t headerChecksum;
    uint16_t globalChecksum;
};

class Cartridge {
private:
    CartHeader cartHeader;

    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;

    GameBoy *gb;

    /// Opens and loads .gb file
    void loadGBFile();

    /// Grabs all useful cartridge header info. Based on https://gbdev.io/pandocs/The_Cartridge_Header.html
    void verifyCartHeader();

public:
    Cartridge() = delete;
    Cartridge(GameBoy *gb);
};