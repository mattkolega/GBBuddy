#include "gameboy.h"

GameBoy::GameBoy()
    : cartridge(this)
    , cpu(this)
    , mmu(this)
{
    cpu.setToBootState();
}

void GameBoy::init() {
    cartridge.init();
}

void GameBoy::initForTests() {
    cartridge.initForTests();
}

void GameBoy::run() {
    cpu.step();
    cpu.printState();
}