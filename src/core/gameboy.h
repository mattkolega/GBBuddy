#pragma once

#include "cpu.h"
#include "mmu.h"

class GameBoy {
public:
    CPU cpu;
    MMU mmu;

    GameBoy();
};