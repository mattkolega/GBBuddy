#include "cartridge.h"

#include <stdexcept>
#include <fstream>
#include <iterator>

#include "../utils/bitwise.h"
#include "../utils/dialog.h"
#include "../utils/logger.h"

Cartridge::Cartridge(GameBoy *gb)
    : m_gb(gb)
{
    auto result = loadGBFile();
    if (!result) throw std::runtime_error(result.error());

    m_rom = result.value();
    verifyCartHeader();
}

std::expected<std::vector<uint8_t>, std::string> Cartridge::loadGBFile() {
    auto filepath = Dialog::openFile("Open GB ROM File", {"*.gb"}, "Game Boy ROMs");
    if (filepath.empty()) return std::unexpected("Filepath is empty");

    std::ifstream romFile(filepath, std::ios::binary | std::ios::in);
    if (!romFile) return std::unexpected("Failed to open ROM file: " + filepath);

    Logger::info("{} {}", "Loaded ROM: ", filepath);

    return std::vector<uint8_t> {
        std::istream_iterator<uint8_t>(romFile),
        std::istream_iterator<uint8_t>(),
    };
}

void Cartridge::verifyCartHeader() {
    // If ROM size is less than required size for cart header, throw error
    if (m_rom.size() < 325) throw std::runtime_error("ROM size is too small to properly pass header");

    CartHeader cartHeader;

    size_t titleIndex { 0x0134 };

    for (int i = titleIndex; i <= 0x0143; titleIndex += 1) {
        if (m_rom[i] == 0x00) break;
        cartHeader.title += m_rom[i];
    }

    cartHeader.cartType = m_rom[0x0147];

    cartHeader.romBanks = 2 * (1 << m_rom[0x0148]);

    switch(m_rom[0x0149]) {
        case 0x00:
        case 0x01:
            cartHeader.ramBanks = 0;
            break;
        case 0x02:
            cartHeader.ramBanks = 1;
            break;
        case 0x03:
            cartHeader.ramBanks = 4;
            break;
        case 0x04:
            cartHeader.ramBanks = 16;
            break;
        case 0x05:
            cartHeader.ramBanks = 8;
            break;
        default:
            cartHeader.ramBanks = 0;
            Logger::warn("{}", "Unknown value given for number of RAM banks");
            break;
    }

    m_ram.resize((1024 * 8) * cartHeader.ramBanks);

    cartHeader.headerChecksum = m_rom[0x014D];
    cartHeader.globalChecksum = Bitwise::concatBytes(m_rom[0x014F], m_rom[0x014E]);

    m_cartHeader = cartHeader;
}