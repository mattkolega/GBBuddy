#pragma once

#include <cstdint>
#include <expected>
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
public:
    Cartridge() = delete;
    Cartridge(GameBoy *gb);

private:
    CartHeader m_cartHeader;

    std::vector<uint8_t> m_rom {};
    std::vector<uint8_t> m_ram {};

    GameBoy *m_gb;

    // Opens and loads .gb file
    std::expected<std::vector<uint8_t>, std::string> loadGBFile();

    // Grabs all useful cartridge header info. Based on https://gbdev.io/pandocs/The_Cartridge_Header.html
    void verifyCartHeader();
};