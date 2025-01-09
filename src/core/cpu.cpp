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