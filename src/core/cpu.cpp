#include "cpu.h"

#include <bit>

#include "gameboy.h"
#include "../utils/bitwise.h"

CPU::CPU(GameBoy *gb)
    : gb(gb)
{
}

uint8_t CPU::memoryRead(uint16_t address) {
    return gb->mmu.memoryRead(address);
}

void CPU::memoryWrite(uint16_t address, uint8_t value) {
    gb->mmu.memoryWrite(address, value);
}

uint16_t CPU::getAF() {
    return Bitwise::concatBytes(f, a);
}

void CPU::setAF(uint16_t value) {
    a = (value >> 8) & 0xFF;
    f = value & 0xFF;
}

uint16_t CPU::getBC() {
    return Bitwise::concatBytes(c, b);
}

void CPU::setBC(uint16_t value) {
    b = (value >> 8) & 0xFF;
    c = value & 0xFF;
}

uint16_t CPU::getDE() {
    return Bitwise::concatBytes(e, d);
}

void CPU::setDE(uint16_t value) {
    d = (value >> 8) & 0xFF;
    e = value & 0xFF;
}

uint16_t CPU::getHL() {
    return Bitwise::concatBytes(l, h);
}

void CPU::setHL(uint16_t value) {
    h = (value >> 8) & 0xFF;
    l = value & 0xFF;
}

uint8_t CPU::getZero() {
    return Bitwise::getBitInByte(f, 7);
}

void CPU::setZero(uint8_t value) {
    f = Bitwise::modifyBitInByte(f, 7, value);
}

uint8_t CPU::getSubtract() {
    return Bitwise::getBitInByte(f, 6);
}

void CPU::setSubtract(uint8_t value) {
    f = Bitwise::modifyBitInByte(f, 6, value);
}

uint8_t CPU::getHalfCarry() {
    return Bitwise::getBitInByte(f, 5);
}

void CPU::setHalfCarry(uint8_t value) {
    f = Bitwise::modifyBitInByte(f, 5, value);
}

uint8_t CPU::getCarry() {
    return Bitwise::getBitInByte(f, 4);
}

void CPU::setCarry(uint8_t value) {
    f = Bitwise::modifyBitInByte(f, 4, value);
}

void CPU::pushToStack16(uint16_t value) {
    sp--;
    memoryWrite(sp, (value >> 8) & 0xFF);
    sp--;
    memoryWrite(sp, value & 0xFF);
}

uint16_t CPU::popStack16() {
    auto lo = memoryRead(sp);
    sp++;
    auto hi = memoryRead(sp);
    sp++;
    return Bitwise::concatBytes(lo, hi);
}

/**
8-bit Arithmetic and Logic Instructions
 */

void CPU::ADC(uint8_t value) {
    auto originalValue = a;
    a += value + getCarry();

    // Set flags
    setZero(a == 0);
    setSubtract(0);
    setHalfCarry(Bitwise::checkHalfCarryAdd(originalValue, (value + getCarry())));
    setCarry(a < originalValue);
}

void CPU::ADD(uint8_t value) {
    auto originalValue = a;
    a += value;

    // Set flags
    setZero(a == 0);
    setSubtract(0);
    setHalfCarry(Bitwise::checkHalfCarryAdd(originalValue, value));
    setCarry(a < originalValue);
}

void CPU::AND(uint8_t value) {
    a &= value;

    // Set flags
    setZero(a == 0);
    setSubtract(0);
    setHalfCarry(1);
    setCarry(0);
}

void CPU::CP(uint8_t value) {
    auto originalValue = a;
    uint8_t subResult = a - value;

    // Set flags
    setZero(subResult == 0);
    setSubtract(1);
    setHalfCarry(Bitwise::checkHalfCarrySub(originalValue, value));
    setCarry(subResult > originalValue);
}

void CPU::OR(uint8_t value) {
    a |= value;

    // Set flags
    setZero(a == 0);
    setSubtract(0);
    setHalfCarry(0);
    setCarry(0);
}

void CPU::SBC(uint8_t value) {
    auto originalValue = a;
    a -= (value + getCarry());

    // Set flags
    setZero(a == 0);
    setSubtract(1);
    setHalfCarry(Bitwise::checkHalfCarrySub(originalValue, value + getCarry()));
    setCarry(a > originalValue);

}

void CPU::SUB(uint8_t value) {
    auto originalValue = a;
    a -= value;

    // Set flags
    setZero(a == 0);
    setSubtract(1);
    setHalfCarry(Bitwise::checkHalfCarrySub(originalValue, value));
    setCarry(a > originalValue);
}

void CPU::XOR(uint8_t value) {
    a ^= value;

    // Set flags
    setZero(a == 0);
    setSubtract(0);
    setHalfCarry(0);
    setCarry(0);
}

/**
16-bit Arithmetic Instructions
 */

void CPU::ADD(uint16_t value) {
    auto originalValue = getHL();
    uint16_t result = originalValue + value;
    setHL(result);

    // Set flags
    setSubtract(0);
    setHalfCarry(Bitwise::checkHalfCarryAdd(originalValue, value));
    setCarry(result < originalValue);
}

/**
Bit Operation Instructions
 */

void CPU::BIT(uint8_t bitPos, uint8_t value) {
    setZero((value & bitPos) == 0);
    setSubtract(0);
    setHalfCarry(1);
}

/**
Bit Shift Instructions
 */

uint8_t CPU::RL(uint8_t value) {
    auto carryValue = getCarry();  // Get current carry flag value
    setCarry(Bitwise::getBitInByte(value, 7));  // Set carry flag to leftmost bit
    uint8_t newValue = std::rotl(value, 1);
    return newValue & carryValue;
}

void CPU::RL_HL() {
    auto newValue = RL(memoryRead(getHL()));
    memoryWrite(getHL(), newValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::RLA() {
    a = RL(a);

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(0);
}

uint8_t CPU::RLC(uint8_t value) {
    setCarry(Bitwise::getBitInByte(value, 7));  // Set carry flag to leftmost bit
    return std::rotl(value, 1);
}

void CPU::RLC_HL() {
    auto newValue = RLC(memoryRead(getHL()));
    memoryWrite(getHL(), newValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::RLCA() {
    a = RLC(a);

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(0);
}

uint8_t CPU::RR(uint8_t value) {
    auto carryValue = getCarry();  // Get current carry flag value
    setCarry(Bitwise::getBitInByte(value, 0));  // Set carry flag to right-most bit
    uint8_t newValue = std::rotr(value, 1);
    return newValue & (carryValue << 7);
}

void CPU::RR_HL() {
    auto newValue = RR(memoryRead(getHL()));
    memoryWrite(getHL(), newValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::RRA() {
    a = RR(a);

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(0);
}

uint8_t CPU::RRC(uint8_t value) {
    setCarry(Bitwise::getBitInByte(value, 0));  // Set carry flag to right-most bit
    return std::rotr(value, 1);
}

void CPU::RRC_HL() {
    auto newValue = RRC(memoryRead(getHL()));
    memoryWrite(getHL(), newValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::RRCA() {
    a = RRC(a);

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(0);
}

/**
Load Instructions
 */

void CPU::LD_HL(uint8_t value) {
    memoryWrite(getHL(), value);
}

void CPU::LD_n16_A(uint16_t address) {
    memoryWrite(address, a);
}

void CPU::LD_A_n16(uint16_t address) {
    a = memoryRead(address);
}

void CPU::LDH_n16_A(uint8_t lowByte) {
    memoryWrite(0xFF00 & lowByte, a);
}

void CPU::LDH_C_A() {
    memoryWrite(0xFF00 & c, a);
}

void CPU::LDH_A_n16(uint8_t lowByte) {
    a = memoryRead(0xFF00 & lowByte);
}

void CPU::LDH_A_C() {
    a = memoryRead(0xFF00 & c);
}

void CPU::LD_HLI_A() {
    auto hl = getHL();

    a = memoryRead(hl);
    setHL(hl+1);
}

void CPU::LD_HLD_A() {
    auto hl = getHL();

    a = memoryRead(hl);
    setHL(hl-1);
}

void CPU::LD_A_HLI() {
    auto hl = getHL();

    memoryWrite(hl, a);
    setHL(hl+1);
}

void CPU::LD_A_HLD() {
    auto hl = getHL();

    memoryWrite(hl, a);
    setHL(hl-1);
}

/**
Jumps and Subroutines
 */

void CPU::CALL(uint16_t address) {
    pushToStack16(pc);
    pc = address;
}

void CPU::JP(uint16_t address) {
    pc = address;
}

void CPU::JR(int8_t offset) {
    pc += offset;
}

void CPU::RET() {
    pc = popStack16();
}

void CPU::RETI() {
    pc = popStack16();
    ime = 1;
}

void CPU::RST(uint8_t vec) {
    pushToStack16(pc);
    pc = vec;
}

/**
Stack Operation Instructions
 */

void CPU::ADD_HL_SP() {
    auto originalValue = getHL();
    uint16_t result = originalValue + sp;
    setHL(result);

    // Set flags
    setSubtract(0);
    setHalfCarry(Bitwise::checkHalfCarryAdd(originalValue, sp));
    setCarry(result < originalValue);
}

void CPU::ADD_SP_e8(int8_t value) {
    auto originalValue = sp;
    uint16_t result = sp + value;
    sp = result;

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(Bitwise::checkHalfCarryAdd(originalValue, value));
    setCarry(result < originalValue);
}

void CPU::LD_SP_n16(uint16_t value) {
    sp = value;
}

void CPU::LD_n16_SP(uint16_t address) {
    memoryWrite(address, sp & 0xFF);
    memoryWrite(address + 1, (sp >> 8) & 0xFF);
}

void CPU::LD_HL_SP(int8_t value) {
    uint16_t result = sp + value;
    setHL(value);

    setZero(0);
    setSubtract(0);
    setHalfCarry(Bitwise::checkHalfCarryAdd(sp, value));
    setCarry(result < sp);
}

void CPU::LD_SP_HL() {
    sp = getHL();
}

/**
Miscellaneous Instructions
 */

void CPU::CCF() {
    setSubtract(0);
    setHalfCarry(0);
    setCarry(~getCarry());
}

void CPU::CPL() {
    a = ~a;

    // Set flags
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::DAA() {
    uint8_t offset { 0 };

    if (getSubtract() == 0 && (a & 0xF) > 0x9 || getHalfCarry() == 1) {
        offset |= 0x06;
    }

    if (getSubtract() == 0 && a > 0x99 || getCarry() == 1) {
        offset |= 0x60;
        setCarry(1);
    }

    (getSubtract() == 0) ? a += offset : a -= offset;

    // Set flags
    setZero(a == 0);
    setHalfCarry(0);
}

void CPU::DI() {
    ime = 0;
}

void CPU::EI() {
    // TODO: Add delayed setting of IME flag
}

void CPU::HALT() {
    // TODO: Implement behaviour
}

void CPU::NOP() {
    return;
}

void CPU::SCF() {
    setSubtract(0);
    setHalfCarry(0);
    setCarry(1);
}

void CPU::STOP() {
    // TODO: implement behaviour
}