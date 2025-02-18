#include "cartridge.h"

#include <stdexcept>
#include <fstream>

#include <common/bitwise.h>
#include <common/dialog.h>
#include <common/logger.h>

#include "mappers/nombc.h"

Cartridge::Cartridge(GameBoy *gb)
    : m_gb(gb)
{
    loadGBFile();
    verifyCartHeader();
    setMapper();
}

uint8_t Cartridge::romRead(uint16_t address) {
    return mapper->romRead(address);
}

void Cartridge::romWrite(uint16_t address, uint8_t value) {
    return mapper->romWrite(address, value);
}

void Cartridge::loadGBFile() {
    auto filepath = Dialog::openFile("Open GB ROM File", {"*.gb"}, "Game Boy ROMs");
    if (filepath.empty()) throw std::runtime_error("ROM size is too small to properly pass header");

    std::ifstream romFile(filepath, std::ios::binary);
    if (!romFile) throw std::runtime_error("Failed to open ROM file: " + filepath);

    Logger::info("{} {}", "Loaded ROM: ", filepath);

    romFile.seekg(0, std::ios::end);
    auto fileSize = romFile.tellg();
    romFile.seekg(0, std::ios::beg);

    m_rom.resize(fileSize);
    if (!romFile.read(reinterpret_cast<char*>(m_rom.data()), fileSize)) {
        throw std::runtime_error("Failed to read the ROM file.");
    }
}

void Cartridge::verifyCartHeader() {
    // If ROM size is less than required size for cart header, throw error
    if (m_rom.size() < 325) throw std::runtime_error("ROM size is too small to properly parse header");

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
            Logger::warn("{} 0x{:X}", "Unknown value given for number of RAM banks:", m_rom[0x0149]);
            break;
    }

    m_ram.resize((1024 * 8) * cartHeader.ramBanks);

    cartHeader.headerChecksum = m_rom[0x014D];
    cartHeader.globalChecksum = Bitwise::concatBytes(m_rom[0x014E], m_rom[0x014F]);

    m_cartHeader = cartHeader;
}

void Cartridge::setMapper() {
    switch (m_cartHeader.cartType) {
        case 0x01:
            mapper = std::make_unique<NoMBC>(*this);
            break;
        default:
            throw std::runtime_error("Specified cartridge type not supported");
    }
}