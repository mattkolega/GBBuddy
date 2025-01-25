#include "gameboy.h"

GameBoy::GameBoy()
    : cartridge(this)
    , cpu(this)
    , mmu(this)
{
    cpu.setToBootState();
}

void GameBoy::run() {
    cpu.step();
    cpu.printState();
}