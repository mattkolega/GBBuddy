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

    // Sets up emulator and components
    void init();

    // Sets up emulator for use in testing
    void initForTests();

    void run();
};