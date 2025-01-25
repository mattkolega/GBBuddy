#pragma once

#include "cartridge.h"
#include "cpu.h"
#include "mmu.h"

class GameBoy {
public:
    Cartridge cartridge;
    CPU cpu;
    MMU mmu;

    GameBoy();

    void run();
};