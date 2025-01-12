#include "cpu.h"

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

void CPU::setZero(bool isValZero) {
    f = Bitwise::modifyBitInByte(f, 7, isValZero ? 1 : 0);
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

void CPU::setCarry(bool carryOccurred) {
    f = Bitwise::modifyBitInByte(f, 4, carryOccurred ? 1 : 0);
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
    setCarry(static_cast<uint8_t>(0));
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
    setCarry(static_cast<uint8_t>(0));
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
    setCarry(static_cast<uint8_t>(0));
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