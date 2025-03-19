#include "gameboy.h"

GameBoy::GameBoy()
    : cartridge(this)
    , cpu(this)
{
}

void GameBoy::init() {
    cpu.setState({
        .a = 0x01,
        .b = 0x00,
        .c = 0x13,
        .d = 0x00,
        .e = 0xD8,
        .f = 0xB0,
        .h = 0x01,
        .l = 0x4D,
        .sp = 0xFFFE,
        .pc = 0x0100,
    });
    cartridge.init();
    mmu = std::make_unique<RealMMU>(this);
}

void GameBoy::initForTests() {
    cartridge.initForTests();
    mmu = std::make_unique<MockMMU>();
}

void GameBoy::run() {
    cpu.step();
    cpu.printState();
}