#include <cstdint>
#include <random>

#include <catch2/catch_test_macros.hpp>

#include <core/gameboy.h>

void testWriteAndRead(GameBoy &gb, uint16_t addr, uint8_t value) {
    gb.mmu.memoryWrite(addr, value);
    REQUIRE(gb.mmu.memoryRead(addr) == value);
}

TEST_CASE("Memory writes are successful") {
    GameBoy gb;
    gb.initForTests();

    // Setup random number generation
    std::mt19937 mt{ std::random_device{}() };
    std::uniform_int_distribution byte{ 0, 0xFF };

    SECTION("Writes to ROM are successful") {
        for (int addr = 0; addr <= 0x7FFF; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to VRAM are successful") {
        for (int addr = 0x8000; addr <= 0x9FFF; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to External RAM are successful") {
        for (int addr = 0xA000; addr <= 0xBFFF; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to Work RAM are successful") {
        for (int addr = 0xC000; addr <= 0xDFFF; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to Echo RAM are successful") {
        for (int addr = 0xE000; addr <= 0xFDFF; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to OAM are successful") {
        for (int addr = 0xFE00; addr <= 0xFE9F; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to unusable range are successful") {
        for (int addr = 0xFEA0; addr <= 0xFEFF; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to I/O registers are successful") {
        for (int addr = 0xFF00; addr <= 0xFF7F; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to High RAM are successful") {
        for (int addr = 0xFF80; addr <= 0xFFFE; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }

    SECTION("Writes to Interrupt Enable Register are successful") {
        for (int addr = 0xFFFF; addr <= 0xFFFF; addr++) {
            uint8_t value = byte(mt);
            testWriteAndRead(gb, addr, value);
        }
    }
}