#pragma once

#include <array>

#include "common/types.h"

static constexpr usize BUS_MEMORY_SIZE {1024 * 64};

class Cartridge;

class Bus {
public:
    virtual ~Bus() = default;
    virtual u8   read(u16 address)          = 0;
    virtual void write(u16 address, u8 val) = 0;
};

// Proper Bus implementation to be used in the emulator
class RealBus : public Bus {
public:
    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;

    explicit RealBus(Cartridge& cartridge)
        : m_cartridge(cartridge) {}

private:
    std::array<u8, BUS_MEMORY_SIZE> m_memory {};  // Temporary 64KB will gradually be reduced to actual RAM size as proper memory mapping is introduced

    Cartridge& m_cartridge;
};

// Mock Bus implementation ONLY to be used for CPU testing
class MockBus : public Bus {
public:
    u8 read(u16 address) override;
    void write(u16 address, u8 value) override;

private:
    std::array<u8, BUS_MEMORY_SIZE> m_memory {};  // Flat 64kb of memory. No registers or mapping
};