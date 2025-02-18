#include "cpu.h"

#include <bit>

#include <fmt/base.h>

#include "gameboy.h"
#include "../common/bitwise.h"

CPU::CPU(GameBoy *gb)
    : gb(gb)
{
}

size_t CPU::step() {
    return opDecode();
}

void CPU::setToBootState() {
    a = 0x01;
    f = 0xB0;
    b = 0x00;
    c = 0x13;
    d = 0x00;
    e = 0xD8;
    h = 0x01;
    l = 0x4D;
    sp = 0xFFFE;
    pc = 0x0100;
}

void CPU::printState() {
    fmt::println(
        "A:{:02X} F:{:02X} B:{:02X} C:{:02X} D:{:02X} E:{:02X} H:{:02X} L:{:02X} SP:{:04X} PC:{:04X} PCMEM:{:02X},{:02X},{:02X},{:02X}",
        a, f, b, c, d ,e ,h, l, sp, pc, memoryRead(pc), memoryRead(pc+1), memoryRead(pc+2), memoryRead(pc+3)
    );
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
    return Bitwise::modifyBitInByte(newValue, 0, carryValue);
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
    return Bitwise::modifyBitInByte(newValue, 7, carryValue);
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
    memoryWrite(0xFF00 | lowByte, a);
}

void CPU::LDH_C_A() {
    memoryWrite(0xFF00 | c, a);
}

void CPU::LDH_A_n16(uint8_t lowByte) {
    a = memoryRead(0xFF00 | lowByte);
}

void CPU::LDH_A_C() {
    a = memoryRead(0xFF00 | c);
}

void CPU::LD_HLI_A() {
    auto hl = getHL();

    memoryWrite(hl, a);
    setHL(hl+1);
}

void CPU::LD_HLD_A() {
    auto hl = getHL();

    memoryWrite(hl, a);
    setHL(hl-1);
}

void CPU::LD_A_HLI() {
    auto hl = getHL();

    a = memoryRead(hl);
    setHL(hl+1);
}

void CPU::LD_A_HLD() {
    auto hl = getHL();

    a = memoryRead(hl);
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

// Reports an invalid opcode and returns 0 to be used as number of cycles
static size_t logInvalidOpcode(uint16_t opcode) {
    Logger::err("{} {:X}", "Invalid opcode: ", opcode);
    return 0;
}

size_t CPU::opDecode() {
    auto opcode = memoryRead(pc);
    pc += 1;

    switch (Bitwise::getFirstNibble(opcode)) {
        case 0x0: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    NOP();
                    return 1;
                }
                case 0x1: {
                    auto value = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    LD_r16<RegisterType::BC>(value);
                    return 3;
                }
                case 0x2: {
                    LD_n16_A(getBC());
                    return 2;
                }
                case 0x3: {
                    INC16<RegisterType::BC>();
                    return 2;
                }
                case 0x4: {
                    INC8<RegisterType::B>();
                    return 1;
                }
                case 0x5: {
                    DEC8<RegisterType::B>();
                    return 1;
                }
                case 0x6: {
                    LD_r8<RegisterType::B>(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0x7: {
                    RLCA();
                    return 1;
                }
                case 0x8: {
                    auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    LD_n16_SP(address);
                    return 5;
                }
                case 0x9: {
                    ADD(getBC());
                    return 2;
                }
                case 0xA: {
                    LD_r8<RegisterType::A>(memoryRead(getBC()));
                    return 2;
                }
                case 0xB: {
                    DEC16<RegisterType::BC>();
                    return 2;
                }
                case 0xC: {
                    INC8<RegisterType::C>();
                    return 1;
                }
                case 0xD: {
                    DEC8<RegisterType::C>();
                    return 1;
                }
                case 0xE: {
                    LD_r8<RegisterType::C>(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0xF: {
                    RRCA();
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x1: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    STOP();
                    return 2;
                }
                case 0x1: {
                    auto value = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    LD_r16<RegisterType::DE>(value);
                    return 3;
                }
                case 0x2: {
                    LD_n16_A(getDE());
                    return 2;
                }
                case 0x3: {
                    INC16<RegisterType::DE>();
                    return 2;
                }
                case 0x4: {
                    INC8<RegisterType::D>();
                    return 1;
                }
                case 0x5: {
                    DEC8<RegisterType::D>();
                    return 1;
                }
                case 0x6: {
                    LD_r8<RegisterType::D>(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0x7: {
                    RLA();
                    return 1;
                }
                case 0x8: {
                    JR(memoryRead(pc));
                    pc++;
                    return 3;
                }
                case 0x9: {
                    ADD(getDE());
                    return 2;
                }
                case 0xA: {
                    LD_A_n16(getDE());
                    return 2;
                }
                case 0xB: {
                    DEC16<RegisterType::DE>();
                    return 2;
                }
                case 0xC: {
                    INC8<RegisterType::E>();
                    return 1;
                }
                case 0xD: {
                    DEC8<RegisterType::E>();
                    return 1;
                }
                case 0xE: {
                    LD_r8<RegisterType::E>(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0xF: {
                    RRA();
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x2: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    if (getZero() == 0) {
                        JR(memoryRead(pc));
                        pc++;
                        return 3;
                    } else {
                        pc++;
                        return 2;
                    }
                }
                case 0x1: {
                    auto value = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    LD_r16<RegisterType::HL>(value);
                    return 3;
                }
                case 0x2: {
                    LD_HLI_A();
                    return 2;
                }
                case 0x3: {
                    INC16<RegisterType::HL>();
                    return 2;
                }
                case 0x4: {
                    INC8<RegisterType::H>();
                    return 1;
                }
                case 0x5: {
                    DEC8<RegisterType::H>();
                    return 1;
                }
                case 0x6: {
                    LD_r8<RegisterType::H>(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0x7: {
                    DAA();
                    return 1;
                }
                case 0x8: {
                    if (getZero() == 1) {
                        JR(memoryRead(pc));
                        pc++;
                        return 3;
                    } else {
                        pc++;
                        return 2;
                    }
                }
                case 0x9: {
                    ADD(getHL());
                    return 2;
                }
                case 0xA: {
                    LD_A_HLI();
                    return 2;
                }
                case 0xB: {
                    DEC16<RegisterType::HL>();
                    return 2;
                }
                case 0xC: {
                    INC8<RegisterType::L>();
                    return 1;
                }
                case 0xD: {
                    DEC8<RegisterType::L>();
                    return 1;
                }
                case 0xE: {
                    LD_r8<RegisterType::L>(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0xF: {
                    CPL();
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x3: {
           switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    if (getCarry() == 0) {
                        JR(memoryRead(pc));
                        pc++;
                        return 3;
                    } else {
                        pc++;
                        return 2;
                    }
                }
                case 0x1: {
                    auto value = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    LD_SP_n16(value);
                    return 3;
                }
                case 0x2: {
                    LD_HLD_A();
                    return 2;
                }
                case 0x3: {
                    INC16<RegisterType::SP>();
                    return 2;
                }
                case 0x4: {
                    INC8<RegisterType::HL>();
                    return 3;
                }
                case 0x5: {
                    DEC8<RegisterType::HL>();
                    return 3;
                }
                case 0x6: {
                    LD_HL(memoryRead(pc));
                    pc++;
                    return 3;
                }
                case 0x7: {
                    SCF();
                    return 1;
                }
                case 0x8: {
                    if (getCarry() == 1) {
                        JR(memoryRead(pc));
                        pc++;
                        return 3;
                    } else {
                        pc++;
                        return 2;
                    }
                }
                case 0x9: {
                    ADD_HL_SP();
                    return 2;
                }
                case 0xA: {
                    LD_A_HLD();
                    return 2;
                }
                case 0xB: {
                    DEC16<RegisterType::SP>();
                    return 2;
                }
                case 0xC: {
                    INC8<RegisterType::A>();
                    return 1;
                }
                case 0xD: {
                    DEC8<RegisterType::A>();
                    return 1;
                }
                case 0xE: {
                    LD_r8<RegisterType::A>(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0xF: {
                    CCF();
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x4: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    LD_r8<RegisterType::B>(b);
                    return 1;
                }
                case 0x1: {
                    LD_r8<RegisterType::B>(c);
                    return 1;
                }
                case 0x2: {
                    LD_r8<RegisterType::B>(d);
                    return 1;
                }
                case 0x3: {
                    LD_r8<RegisterType::B>(e);
                    return 1;
                }
                case 0x4: {
                    LD_r8<RegisterType::B>(h);
                    return 1;
                }
                case 0x5: {
                    LD_r8<RegisterType::B>(l);
                    return 1;
                }
                case 0x6: {
                    LD_r8<RegisterType::B>(memoryRead(getHL()));
                    return 2;
                }
                case 0x7: {
                    LD_r8<RegisterType::B>(a);
                    return 1;
                }
                case 0x8: {
                    LD_r8<RegisterType::C>(b);
                    return 1;
                }
                case 0x9: {
                    LD_r8<RegisterType::C>(c);
                    return 1;
                }
                case 0xA: {
                    LD_r8<RegisterType::C>(d);
                    return 1;
                }
                case 0xB: {
                    LD_r8<RegisterType::C>(e);
                    return 1;
                }
                case 0xC: {
                    LD_r8<RegisterType::C>(h);
                    return 1;
                }
                case 0xD: {
                    LD_r8<RegisterType::C>(l);
                    return 1;
                }
                case 0xE: {
                    LD_r8<RegisterType::C>(memoryRead(getHL()));
                    return 2;
                }
                case 0xF: {
                    LD_r8<RegisterType::C>(a);
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x5: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    LD_r8<RegisterType::D>(b);
                    return 1;
                }
                case 0x1: {
                    LD_r8<RegisterType::D>(c);
                    return 1;
                }
                case 0x2: {
                    LD_r8<RegisterType::D>(d);
                    return 1;
                }
                case 0x3: {
                    LD_r8<RegisterType::D>(e);
                    return 1;
                }
                case 0x4: {
                    LD_r8<RegisterType::D>(h);
                    return 1;
                }
                case 0x5: {
                    LD_r8<RegisterType::D>(l);
                    return 1;
                }
                case 0x6: {
                    LD_r8<RegisterType::D>(memoryRead(getHL()));
                    return 2;
                }
                case 0x7: {
                    LD_r8<RegisterType::D>(a);
                    return 1;
                }
                case 0x8: {
                    LD_r8<RegisterType::E>(b);
                    return 1;
                }
                case 0x9: {
                    LD_r8<RegisterType::E>(c);
                    return 1;
                }
                case 0xA: {
                    LD_r8<RegisterType::E>(d);
                    return 1;
                }
                case 0xB: {
                    LD_r8<RegisterType::E>(e);
                    return 1;
                }
                case 0xC: {
                    LD_r8<RegisterType::E>(h);
                    return 1;
                }
                case 0xD: {
                    LD_r8<RegisterType::E>(l);
                    return 1;
                }
                case 0xE: {
                    LD_r8<RegisterType::E>(memoryRead(getHL()));
                    return 2;
                }
                case 0xF: {
                    LD_r8<RegisterType::E>(a);
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x6: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    LD_r8<RegisterType::H>(b);
                    return 1;
                }
                case 0x1: {
                    LD_r8<RegisterType::H>(c);
                    return 1;
                }
                case 0x2: {
                    LD_r8<RegisterType::H>(d);
                    return 1;
                }
                case 0x3: {
                    LD_r8<RegisterType::H>(e);
                    return 1;
                }
                case 0x4: {
                    LD_r8<RegisterType::H>(h);
                    return 1;
                }
                case 0x5: {
                    LD_r8<RegisterType::H>(l);
                    return 1;
                }
                case 0x6: {
                    LD_r8<RegisterType::H>(memoryRead(getHL()));
                    return 2;
                }
                case 0x7: {
                    LD_r8<RegisterType::H>(a);
                    return 1;
                }
                case 0x8: {
                    LD_r8<RegisterType::L>(b);
                    return 1;
                }
                case 0x9: {
                    LD_r8<RegisterType::L>(c);
                    return 1;
                }
                case 0xA: {
                    LD_r8<RegisterType::L>(d);
                    return 1;
                }
                case 0xB: {
                    LD_r8<RegisterType::L>(e);
                    return 1;
                }
                case 0xC: {
                    LD_r8<RegisterType::L>(h);
                    return 1;
                }
                case 0xD: {
                    LD_r8<RegisterType::L>(l);
                    return 1;
                }
                case 0xE: {
                    LD_r8<RegisterType::L>(memoryRead(getHL()));
                    return 2;
                }
                case 0xF: {
                    LD_r8<RegisterType::L>(a);
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x7: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    LD_HL(b);
                    return 2;
                }
                case 0x1: {
                    LD_HL(c);
                    return 2;
                }
                case 0x2: {
                    LD_HL(d);
                    return 2;
                }
                case 0x3: {
                    LD_HL(e);
                    return 2;
                }
                case 0x4: {
                    LD_HL(h);
                    return 2;
                }
                case 0x5: {
                    LD_HL(l);
                    return 2;
                }
                case 0x6: {
                    HALT();
                    return 1;
                }
                case 0x7: {
                    LD_HL(b);
                    return 2;
                }
                case 0x8: {
                    LD_r8<RegisterType::A>(b);
                    return 1;
                }
                case 0x9: {
                    LD_r8<RegisterType::A>(c);
                    return 1;
                }
                case 0xA: {
                    LD_r8<RegisterType::A>(d);
                    return 1;
                }
                case 0xB: {
                    LD_r8<RegisterType::A>(e);
                    return 1;
                }
                case 0xC: {
                    LD_r8<RegisterType::A>(h);
                    return 1;
                }
                case 0xD: {
                    LD_r8<RegisterType::A>(l);
                    return 1;
                }
                case 0xE: {
                    LD_r8<RegisterType::A>(memoryRead(getHL()));
                    return 2;
                }
                case 0xF: {
                    LD_r8<RegisterType::A>(a);
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x8: {
           switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    ADD(b);
                    return 1;
                }
                case 0x1: {
                    ADD(c);
                    return 1;
                }
                case 0x2: {
                    ADD(d);
                    return 1;
                }
                case 0x3: {
                    ADD(e);
                    return 1;
                }
                case 0x4: {
                    ADD(h);
                    return 1;
                }
                case 0x5: {
                    ADD(l);
                    return 1;
                }
                case 0x6: {
                    ADD(memoryRead(pc));
                    return 2;
                }
                case 0x7: {
                    ADD(a);
                    return 1;
                }
                case 0x8: {
                    ADC(b);
                    return 1;
                }
                case 0x9: {
                    ADC(c);
                    return 1;
                }
                case 0xA: {
                    ADC(d);
                    return 1;
                }
                case 0xB: {
                    ADC(e);
                    return 1;
                }
                case 0xC: {
                    ADC(h);
                    return 1;
                }
                case 0xD: {
                    ADC(l);
                    return 1;
                }
                case 0xE: {
                    ADC(memoryRead(getHL()));
                    return 2;
                }
                case 0xF: {
                    ADC(a);
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x9: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    SUB(b);
                    return 1;
                }
                case 0x1: {
                    SUB(c);
                    return 1;
                }
                case 0x2: {
                    SUB(d);
                    return 1;
                }
                case 0x3: {
                    SUB(e);
                    return 1;
                }
                case 0x4: {
                    SUB(h);
                    return 1;
                }
                case 0x5: {
                    SUB(l);
                    return 1;
                }
                case 0x6: {
                    SUB(memoryRead(pc));
                    return 2;
                }
                case 0x7: {
                    SUB(a);
                    return 1;
                }
                case 0x8: {
                    SBC(b);
                    return 1;
                }
                case 0x9: {
                    SBC(c);
                    return 1;
                }
                case 0xA: {
                    SBC(d);
                    return 1;
                }
                case 0xB: {
                    SBC(e);
                    return 1;
                }
                case 0xC: {
                    SBC(h);
                    return 1;
                }
                case 0xD: {
                    SBC(l);
                    return 1;
                }
                case 0xE: {
                    SBC(memoryRead(getHL()));
                    return 2;
                }
                case 0xF: {
                    SBC(a);
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xA: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    AND(b);
                    return 1;
                }
                case 0x1: {
                    AND(c);
                    return 1;
                }
                case 0x2: {
                    AND(d);
                    return 1;
                }
                case 0x3: {
                    AND(e);
                    return 1;
                }
                case 0x4: {
                    AND(h);
                    return 1;
                }
                case 0x5: {
                    AND(l);
                    return 1;
                }
                case 0x6: {
                    AND(memoryRead(pc));
                    return 2;
                }
                case 0x7: {
                    AND(a);
                    return 1;
                }
                case 0x8: {
                    XOR(b);
                    return 1;
                }
                case 0x9: {
                    XOR(c);
                    return 1;
                }
                case 0xA: {
                    XOR(d);
                    return 1;
                }
                case 0xB: {
                    XOR(e);
                    return 1;
                }
                case 0xC: {
                    XOR(h);
                    return 1;
                }
                case 0xD: {
                    XOR(l);
                    return 1;
                }
                case 0xE: {
                    XOR(memoryRead(getHL()));
                    return 2;
                }
                case 0xF: {
                    XOR(a);
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xB: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    OR(b);
                    return 1;
                }
                case 0x1: {
                    OR(c);
                    return 1;
                }
                case 0x2: {
                    OR(d);
                    return 1;
                }
                case 0x3: {
                    OR(e);
                    return 1;
                }
                case 0x4: {
                    OR(h);
                    return 1;
                }
                case 0x5: {
                    OR(l);
                    return 1;
                }
                case 0x6: {
                    OR(memoryRead(getHL()));
                    return 2;
                }
                case 0x7: {
                    OR(a);
                    return 1;
                }
                case 0x8: {
                    CP(b);
                    return 1;
                }
                case 0x9: {
                    CP(c);
                    return 1;
                }
                case 0xA: {
                    CP(d);
                    return 1;
                }
                case 0xB: {
                    CP(e);
                    return 1;
                }
                case 0xC: {
                    CP(h);
                    return 1;
                }
                case 0xD: {
                    CP(l);
                    return 1;
                }
                case 0xE: {
                    CP(memoryRead(getHL()));
                    return 2;
                }
                case 0xF: {
                    CP(a);
                    return 1;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xC: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    if (getZero() == 0) {
                        RET();
                        return 5;
                    } else {
                        return 2;
                    }
                }
                case 0x1: {
                    POP<RegisterType::BC>();
                    return 3;
                }
                case 0x2: {
                    if (getZero() == 0) {
                        auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                        pc += 2;
                        JP(address);
                        return 4;
                    } else {
                        pc += 2;
                        return 3;
                    }
                }
                case 0x3: {
                    auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    JP(address);
                    return 4;
                }
                case 0x4: {
                    if (getZero() == 0) {
                        auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                        pc += 2;
                        CALL(address);
                        return 6;
                    } else {
                        pc += 2;
                        return 3;
                    }
                }
                case 0x5: {
                    PUSH<RegisterType::BC>();
                    return 4;
                }
                case 0x6: {
                    ADD(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0x7: {
                    RST(0x00);
                    return 4;
                }
                case 0x8: {
                    if (getZero() == 1) {
                        RET();
                        return 5;
                    } else {
                        return 2;
                    }
                }
                case 0x9: {
                    RET();
                    return 4;
                }
                case 0xA: {
                    if (getZero() == 1) {
                        auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                        pc += 2;
                        JP(address);
                        return 4;
                    } else {
                        pc += 2;
                        return 3;
                    }
                }
                case 0xB: {
                    return opDecodeCB();
                }
                case 0xC: {
                    if (getZero() == 1) {
                        auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                        pc += 2;
                        CALL(address);
                        return 6;
                    } else {
                        pc += 2;
                        return 3;
                    }
                }
                case 0xD: {
                    auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    CALL(address);
                    return 6;
                }
                case 0xE: {
                    ADC(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0xF: {
                    RST(0x08);
                    return 4;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xD: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    if (getCarry() == 0) {
                        RET();
                        return 5;
                    } else {
                        return 2;
                    }
                }
                case 0x1: {
                    POP<RegisterType::DE>();
                    return 3;
                }
                case 0x2: {
                    if (getCarry() == 0) {
                        auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                        pc += 2;
                        JP(address);
                        return 4;
                    } else {
                        pc += 2;
                        return 3;
                    }
                }
                case 0x4: {
                    if (getCarry() == 0) {
                        auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                        pc += 2;
                        CALL(address);
                        return 6;
                    } else {
                        pc += 2;
                        return 3;
                    }
                }
                case 0x5: {
                    PUSH<RegisterType::DE>();
                    return 4;
                }
                case 0x6: {
                    SUB(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0x7: {
                    RST(0x10);
                    return 4;
                }
                case 0x8: {
                    if (getCarry() == 1) {
                        RET();
                        return 5;
                    } else {
                        return 2;
                    }
                }
                case 0x9: {
                    RETI();
                    return 4;
                }
                case 0xA: {
                    if (getCarry() == 1) {
                        auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                        pc += 2;
                        JP(address);
                        return 4;
                    } else {
                        pc += 2;
                        return 3;
                    }
                }
                case 0xC: {
                    if (getCarry() == 1) {
                        auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                        pc += 2;
                        CALL(address);
                        return 6;
                    } else {
                        pc += 2;
                        return 3;
                    }
                }
                case 0xE: {
                    SBC(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0xF: {
                    RST(0x18);
                    return 4;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xE: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    LDH_n16_A(memoryRead(pc));
                    pc++;
                    return 3;
                }
                case 0x1: {
                    POP<RegisterType::HL>();
                    return 3;
                }
                case 0x2: {
                    LDH_C_A();
                    return 2;
                }
                case 0x5: {
                    PUSH<RegisterType::HL>();
                    return 4;
                }
                case 0x6: {
                    AND(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0x7: {
                    RST(0x20);
                    return 4;
                }
                case 0x8: {
                    ADD_SP_e8(memoryRead(pc));
                    pc++;
                    return 4;
                }
                case 0x9: {
                    JP(getHL());
                    return 1;
                }
                case 0xA: {
                    auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    LD_n16_A(address);
                    return 4;
                }
                case 0xE: {
                    XOR(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0xF: {
                    RST(0x28);
                    return 4;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xF: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    LDH_A_n16(memoryRead(pc));
                    pc++;
                    return 3;
                }
                case 0x1: {
                    POP<RegisterType::AF>();
                    return 3;
                }
                case 0x2: {
                    LDH_A_C();
                    return 2;
                }
                case 0x3: {
                    DI();
                    return 1;
                }
                case 0x5: {
                    PUSH<RegisterType::AF>();
                    return 4;
                }
                case 0x6: {
                    OR(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0x7: {
                    RST(0x30);
                    return 4;
                }
                case 0x8: {
                    LD_HL_SP(memoryRead(pc));
                    pc++;
                    return 3;
                }
                case 0x9: {
                    LD_SP_HL();
                    return 2;
                }
                case 0xA: {
                    auto address = Bitwise::concatBytes(memoryRead(pc), memoryRead(pc+1));
                    pc += 2;
                    LD_A_n16(address);
                    return 4;
                }
                case 0xB: {
                    EI();
                    return 1;
                }
                case 0xE: {
                    CP(memoryRead(pc));
                    pc++;
                    return 2;
                }
                case 0xF: {
                    RST(0x38);
                    return 4;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        default:
            return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
    }
}

size_t CPU::opDecodeCB() {
    auto opcode = memoryRead(pc);
    pc += 1;

    switch (Bitwise::getFirstNibble(opcode)) {
        case 0x0: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    RLC_r8<RegisterType::B>();
                    return 2;
                }
                case 0x1: {
                    RLC_r8<RegisterType::C>();
                    return 2;
                }
                case 0x2: {
                    RLC_r8<RegisterType::D>();
                    return 2;
                }
                case 0x3: {
                    RLC_r8<RegisterType::E>();
                    return 2;
                }
                case 0x4: {
                    RLC_r8<RegisterType::H>();
                    return 2;
                }
                case 0x5: {
                    RLC_r8<RegisterType::L>();
                    return 2;
                }
                case 0x6: {
                    RLC_HL();
                    return 4;
                }
                case 0x7: {
                    RLC_r8<RegisterType::A>();
                    return 2;
                }
                case 0x8: {
                    RRC_r8<RegisterType::B>();
                    return 2;
                }
                case 0x9: {
                    RRC_r8<RegisterType::C>();
                    return 2;
                }
                case 0xA: {
                    RRC_r8<RegisterType::D>();
                    return 2;
                }
                case 0xB: {
                    RRC_r8<RegisterType::E>();
                    return 2;
                }
                case 0xC: {
                    RRC_r8<RegisterType::H>();
                    return 2;
                }
                case 0xD: {
                    RRC_r8<RegisterType::L>();
                    return 2;
                }
                case 0xE: {
                    RRC_HL();
                    return 4;
                }
                case 0xF: {
                    RRC_r8<RegisterType::A>();
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x1: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    RL_r8<RegisterType::B>();
                    return 2;
                }
                case 0x1: {
                    RL_r8<RegisterType::C>();
                    return 2;
                }
                case 0x2: {
                    RL_r8<RegisterType::D>();
                    return 2;
                }
                case 0x3: {
                    RL_r8<RegisterType::E>();
                    return 2;
                }
                case 0x4: {
                    RL_r8<RegisterType::H>();
                    return 2;
                }
                case 0x5: {
                    RL_r8<RegisterType::L>();
                    return 2;
                }
                case 0x6: {
                    RL_HL();
                    return 4;
                }
                case 0x7: {
                    RL_r8<RegisterType::A>();
                    return 2;
                }
                case 0x8: {
                    RR_r8<RegisterType::B>();
                    return 2;
                }
                case 0x9: {
                    RR_r8<RegisterType::C>();
                    return 2;
                }
                case 0xA: {
                    RR_r8<RegisterType::D>();
                    return 2;
                }
                case 0xB: {
                    RR_r8<RegisterType::E>();
                    return 2;
                }
                case 0xC: {
                    RR_r8<RegisterType::H>();
                    return 2;
                }
                case 0xD: {
                    RR_r8<RegisterType::L>();
                    return 2;
                }
                case 0xE: {
                    RR_HL();
                    return 4;
                }
                case 0xF: {
                    RR_r8<RegisterType::A>();
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x2: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    SLA<RegisterType::B>();
                    return 2;
                }
                case 0x1: {
                    SLA<RegisterType::C>();
                    return 2;
                }
                case 0x2: {
                    SLA<RegisterType::D>();
                    return 2;
                }
                case 0x3: {
                    SLA<RegisterType::E>();
                    return 2;
                }
                case 0x4: {
                    SLA<RegisterType::H>();
                    return 2;
                }
                case 0x5: {
                    SLA<RegisterType::L>();
                    return 2;
                }
                case 0x6: {
                    SLA<RegisterType::HL>();
                    return 4;
                }
                case 0x7: {
                    SLA<RegisterType::A>();
                    return 2;
                }
                case 0x8: {
                    SRA<RegisterType::B>();
                    return 2;
                }
                case 0x9: {
                    SRA<RegisterType::C>();
                    return 2;
                }
                case 0xA: {
                    SRA<RegisterType::D>();
                    return 2;
                }
                case 0xB: {
                    SRA<RegisterType::E>();
                    return 2;
                }
                case 0xC: {
                    SRA<RegisterType::H>();
                    return 2;
                }
                case 0xD: {
                    SRA<RegisterType::L>();
                    return 2;
                }
                case 0xE: {
                    SRA<RegisterType::HL>();
                    return 4;
                }
                case 0xF: {
                    SRA<RegisterType::A>();
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x3: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    SWAP<RegisterType::B>();
                    return 2;
                }
                case 0x1: {
                    SWAP<RegisterType::C>();
                    return 2;
                }
                case 0x2: {
                    SWAP<RegisterType::D>();
                    return 2;
                }
                case 0x3: {
                    SWAP<RegisterType::E>();
                    return 2;
                }
                case 0x4: {
                    SWAP<RegisterType::H>();
                    return 2;
                }
                case 0x5: {
                    SWAP<RegisterType::L>();
                    return 2;
                }
                case 0x6: {
                    SWAP<RegisterType::HL>();
                    return 4;
                }
                case 0x7: {
                    SWAP<RegisterType::A>();
                    return 2;
                }
                case 0x8: {
                    SRL<RegisterType::B>();
                    return 2;
                }
                case 0x9: {
                    SRL<RegisterType::C>();
                    return 2;
                }
                case 0xA: {
                    SRL<RegisterType::D>();
                    return 2;
                }
                case 0xB: {
                    SRL<RegisterType::E>();
                    return 2;
                }
                case 0xC: {
                    SRL<RegisterType::H>();
                    return 2;
                }
                case 0xD: {
                    SRL<RegisterType::L>();
                    return 2;
                }
                case 0xE: {
                    SRL<RegisterType::HL>();
                    return 4;
                }
                case 0xF: {
                    SRL<RegisterType::A>();
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x4: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    BIT(0, b);
                    return 2;
                }
                case 0x1: {
                    BIT(0, c);
                    return 2;
                }
                case 0x2: {
                    BIT(0, d);
                    return 2;
                }
                case 0x3: {
                    BIT(0, e);
                    return 2;
                }
                case 0x4: {
                    BIT(0, h);
                    return 2;
                }
                case 0x5: {
                    BIT(0, l);
                    return 2;
                }
                case 0x6: {
                    BIT(0, memoryRead(getHL()));
                    return 4;
                }
                case 0x7: {
                    BIT(0, a);
                    return 2;
                }
                case 0x8: {
                    BIT(1, b);
                    return 2;
                }
                case 0x9: {
                    BIT(1, c);
                    return 2;
                }
                case 0xA: {
                    BIT(1, d);
                    return 2;
                }
                case 0xB: {
                    BIT(1, e);
                    return 2;
                }
                case 0xC: {
                    BIT(1, h);
                    return 2;
                }
                case 0xD: {
                    BIT(1, l);
                    return 2;
                }
                case 0xE: {
                    BIT(1, memoryRead(getHL()));
                    return 4;
                }
                case 0xF: {
                    BIT(1, a);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x5: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    BIT(2, b);
                    return 2;
                }
                case 0x1: {
                    BIT(2, c);
                    return 2;
                }
                case 0x2: {
                    BIT(2, d);
                    return 2;
                }
                case 0x3: {
                    BIT(2, e);
                    return 2;
                }
                case 0x4: {
                    BIT(2, h);
                    return 2;
                }
                case 0x5: {
                    BIT(2, l);
                    return 2;
                }
                case 0x6: {
                    BIT(2, memoryRead(getHL()));
                    return 4;
                }
                case 0x7: {
                    BIT(2, a);
                    return 2;
                }
                case 0x8: {
                    BIT(3, b);
                    return 2;
                }
                case 0x9: {
                    BIT(3, c);
                    return 2;
                }
                case 0xA: {
                    BIT(3, d);
                    return 2;
                }
                case 0xB: {
                    BIT(3, e);
                    return 2;
                }
                case 0xC: {
                    BIT(3, h);
                    return 2;
                }
                case 0xD: {
                    BIT(3, l);
                    return 2;
                }
                case 0xE: {
                    BIT(3, memoryRead(getHL()));
                    return 4;
                }
                case 0xF: {
                    BIT(3, a);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x6: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    BIT(4, b);
                    return 2;
                }
                case 0x1: {
                    BIT(4, c);
                    return 2;
                }
                case 0x2: {
                    BIT(4, d);
                    return 2;
                }
                case 0x3: {
                    BIT(4, e);
                    return 2;
                }
                case 0x4: {
                    BIT(4, h);
                    return 2;
                }
                case 0x5: {
                    BIT(4, l);
                    return 2;
                }
                case 0x6: {
                    BIT(4, memoryRead(getHL()));
                    return 4;
                }
                case 0x7: {
                    BIT(4, a);
                    return 2;
                }
                case 0x8: {
                    BIT(5, b);
                    return 2;
                }
                case 0x9: {
                    BIT(5, c);
                    return 2;
                }
                case 0xA: {
                    BIT(5, d);
                    return 2;
                }
                case 0xB: {
                    BIT(5, e);
                    return 2;
                }
                case 0xC: {
                    BIT(5, h);
                    return 2;
                }
                case 0xD: {
                    BIT(5, l);
                    return 2;
                }
                case 0xE: {
                    BIT(5, memoryRead(getHL()));
                    return 4;
                }
                case 0xF: {
                    BIT(5, a);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x7: {
           switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    BIT(6, b);
                    return 2;
                }
                case 0x1: {
                    BIT(6, c);
                    return 2;
                }
                case 0x2: {
                    BIT(6, d);
                    return 2;
                }
                case 0x3: {
                    BIT(6, e);
                    return 2;
                }
                case 0x4: {
                    BIT(6, h);
                    return 2;
                }
                case 0x5: {
                    BIT(6, l);
                    return 2;
                }
                case 0x6: {
                    BIT(6, memoryRead(getHL()));
                    return 4;
                }
                case 0x7: {
                    BIT(6, a);
                    return 2;
                }
                case 0x8: {
                    BIT(7, b);
                    return 2;
                }
                case 0x9: {
                    BIT(7, c);
                    return 2;
                }
                case 0xA: {
                    BIT(7, d);
                    return 2;
                }
                case 0xB: {
                    BIT(7, e);
                    return 2;
                }
                case 0xC: {
                    BIT(7, h);
                    return 2;
                }
                case 0xD: {
                    BIT(7, l);
                    return 2;
                }
                case 0xE: {
                    BIT(7, memoryRead(getHL()));
                    return 4;
                }
                case 0xF: {
                    BIT(7, a);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x8: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    RES<RegisterType::B>(0);
                    return 2;
                }
                case 0x1: {
                    RES<RegisterType::C>(0);
                    return 2;
                }
                case 0x2: {
                    RES<RegisterType::D>(0);
                    return 2;
                }
                case 0x3: {
                    RES<RegisterType::E>(0);
                    return 2;
                }
                case 0x4: {
                    RES<RegisterType::H>(0);
                    return 2;
                }
                case 0x5: {
                    RES<RegisterType::L>(0);
                    return 2;
                }
                case 0x6: {
                    RES<RegisterType::HL>(0);
                    return 4;
                }
                case 0x7: {
                    RES<RegisterType::A>(0);
                    return 2;
                }
                case 0x8: {
                    RES<RegisterType::B>(1);
                    return 2;
                }
                case 0x9: {
                    RES<RegisterType::C>(1);
                    return 2;
                }
                case 0xA: {
                    RES<RegisterType::D>(1);
                    return 2;
                }
                case 0xB: {
                    RES<RegisterType::E>(1);
                    return 2;
                }
                case 0xC: {
                    RES<RegisterType::H>(1);
                    return 2;
                }
                case 0xD: {
                    RES<RegisterType::L>(1);
                    return 2;
                }
                case 0xE: {
                    RES<RegisterType::HL>(1);
                    return 4;
                }
                case 0xF: {
                    RES<RegisterType::A>(1);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0x9: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    RES<RegisterType::B>(2);
                    return 2;
                }
                case 0x1: {
                    RES<RegisterType::C>(2);
                    return 2;
                }
                case 0x2: {
                    RES<RegisterType::D>(2);
                    return 2;
                }
                case 0x3: {
                    RES<RegisterType::E>(2);
                    return 2;
                }
                case 0x4: {
                    RES<RegisterType::H>(2);
                    return 2;
                }
                case 0x5: {
                    RES<RegisterType::L>(2);
                    return 2;
                }
                case 0x6: {
                    RES<RegisterType::HL>(2);
                    return 4;
                }
                case 0x7: {
                    RES<RegisterType::A>(2);
                    return 2;
                }
                case 0x8: {
                    RES<RegisterType::B>(3);
                    return 2;
                }
                case 0x9: {
                    RES<RegisterType::C>(3);
                    return 2;
                }
                case 0xA: {
                    RES<RegisterType::D>(3);
                    return 2;
                }
                case 0xB: {
                    RES<RegisterType::E>(3);
                    return 2;
                }
                case 0xC: {
                    RES<RegisterType::H>(3);
                    return 2;
                }
                case 0xD: {
                    RES<RegisterType::L>(3);
                    return 2;
                }
                case 0xE: {
                    RES<RegisterType::HL>(3);
                    return 4;
                }
                case 0xF: {
                    RES<RegisterType::A>(3);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xA: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    RES<RegisterType::B>(4);
                    return 2;
                }
                case 0x1: {
                    RES<RegisterType::C>(4);
                    return 2;
                }
                case 0x2: {
                    RES<RegisterType::D>(4);
                    return 2;
                }
                case 0x3: {
                    RES<RegisterType::E>(4);
                    return 2;
                }
                case 0x4: {
                    RES<RegisterType::H>(4);
                    return 2;
                }
                case 0x5: {
                    RES<RegisterType::L>(4);
                    return 2;
                }
                case 0x6: {
                    RES<RegisterType::HL>(4);
                    return 4;
                }
                case 0x7: {
                    RES<RegisterType::A>(4);
                    return 2;
                }
                case 0x8: {
                    RES<RegisterType::B>(5);
                    return 2;
                }
                case 0x9: {
                    RES<RegisterType::C>(5);
                    return 2;
                }
                case 0xA: {
                    RES<RegisterType::D>(5);
                    return 2;
                }
                case 0xB: {
                    RES<RegisterType::E>(5);
                    return 2;
                }
                case 0xC: {
                    RES<RegisterType::H>(5);
                    return 2;
                }
                case 0xD: {
                    RES<RegisterType::L>(5);
                    return 2;
                }
                case 0xE: {
                    RES<RegisterType::HL>(5);
                    return 4;
                }
                case 0xF: {
                    RES<RegisterType::A>(5);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xB: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    RES<RegisterType::B>(6);
                    return 2;
                }
                case 0x1: {
                    RES<RegisterType::C>(6);
                    return 2;
                }
                case 0x2: {
                    RES<RegisterType::D>(6);
                    return 2;
                }
                case 0x3: {
                    RES<RegisterType::E>(6);
                    return 2;
                }
                case 0x4: {
                    RES<RegisterType::H>(6);
                    return 2;
                }
                case 0x5: {
                    RES<RegisterType::L>(6);
                    return 2;
                }
                case 0x6: {
                    RES<RegisterType::HL>(6);
                    return 4;
                }
                case 0x7: {
                    RES<RegisterType::A>(6);
                    return 2;
                }
                case 0x8: {
                    RES<RegisterType::B>(7);
                    return 2;
                }
                case 0x9: {
                    RES<RegisterType::C>(7);
                    return 2;
                }
                case 0xA: {
                    RES<RegisterType::D>(7);
                    return 2;
                }
                case 0xB: {
                    RES<RegisterType::E>(7);
                    return 2;
                }
                case 0xC: {
                    RES<RegisterType::H>(7);
                    return 2;
                }
                case 0xD: {
                    RES<RegisterType::L>(7);
                    return 2;
                }
                case 0xE: {
                    RES<RegisterType::HL>(7);
                    return 4;
                }
                case 0xF: {
                    RES<RegisterType::A>(7);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xC: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    SET<RegisterType::B>(0);
                    return 2;
                }
                case 0x1: {
                    SET<RegisterType::C>(0);
                    return 2;
                }
                case 0x2: {
                    SET<RegisterType::D>(0);
                    return 2;
                }
                case 0x3: {
                    SET<RegisterType::E>(0);
                    return 2;
                }
                case 0x4: {
                    SET<RegisterType::H>(0);
                    return 2;
                }
                case 0x5: {
                    SET<RegisterType::L>(0);
                    return 2;
                }
                case 0x6: {
                    SET<RegisterType::HL>(0);
                    return 4;
                }
                case 0x7: {
                    SET<RegisterType::A>(0);
                    return 2;
                }
                case 0x8: {
                    SET<RegisterType::B>(1);
                    return 2;
                }
                case 0x9: {
                    SET<RegisterType::C>(1);
                    return 2;
                }
                case 0xA: {
                    SET<RegisterType::D>(1);
                    return 2;
                }
                case 0xB: {
                    SET<RegisterType::E>(1);
                    return 2;
                }
                case 0xC: {
                    SET<RegisterType::H>(1);
                    return 2;
                }
                case 0xD: {
                    SET<RegisterType::L>(1);
                    return 2;
                }
                case 0xE: {
                    SET<RegisterType::HL>(1);
                    return 4;
                }
                case 0xF: {
                    SET<RegisterType::A>(1);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xD: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    SET<RegisterType::B>(2);
                    return 2;
                }
                case 0x1: {
                    SET<RegisterType::C>(2);
                    return 2;
                }
                case 0x2: {
                    SET<RegisterType::D>(2);
                    return 2;
                }
                case 0x3: {
                    SET<RegisterType::E>(2);
                    return 2;
                }
                case 0x4: {
                    SET<RegisterType::H>(2);
                    return 2;
                }
                case 0x5: {
                    SET<RegisterType::L>(2);
                    return 2;
                }
                case 0x6: {
                    SET<RegisterType::HL>(2);
                    return 4;
                }
                case 0x7: {
                    SET<RegisterType::A>(2);
                    return 2;
                }
                case 0x8: {
                    SET<RegisterType::B>(3);
                    return 2;
                }
                case 0x9: {
                    SET<RegisterType::C>(3);
                    return 2;
                }
                case 0xA: {
                    SET<RegisterType::D>(3);
                    return 2;
                }
                case 0xB: {
                    SET<RegisterType::E>(3);
                    return 2;
                }
                case 0xC: {
                    SET<RegisterType::H>(3);
                    return 2;
                }
                case 0xD: {
                    SET<RegisterType::L>(3);
                    return 2;
                }
                case 0xE: {
                    SET<RegisterType::HL>(3);
                    return 4;
                }
                case 0xF: {
                    SET<RegisterType::A>(3);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xE: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    SET<RegisterType::B>(4);
                    return 2;
                }
                case 0x1: {
                    SET<RegisterType::C>(4);
                    return 2;
                }
                case 0x2: {
                    SET<RegisterType::D>(4);
                    return 2;
                }
                case 0x3: {
                    SET<RegisterType::E>(4);
                    return 2;
                }
                case 0x4: {
                    SET<RegisterType::H>(4);
                    return 2;
                }
                case 0x5: {
                    SET<RegisterType::L>(4);
                    return 2;
                }
                case 0x6: {
                    SET<RegisterType::HL>(4);
                    return 4;
                }
                case 0x7: {
                    SET<RegisterType::A>(4);
                    return 2;
                }
                case 0x8: {
                    SET<RegisterType::B>(5);
                    return 2;
                }
                case 0x9: {
                    SET<RegisterType::C>(5);
                    return 2;
                }
                case 0xA: {
                    SET<RegisterType::D>(5);
                    return 2;
                }
                case 0xB: {
                    SET<RegisterType::E>(5);
                    return 2;
                }
                case 0xC: {
                    SET<RegisterType::H>(5);
                    return 2;
                }
                case 0xD: {
                    SET<RegisterType::L>(5);
                    return 2;
                }
                case 0xE: {
                    SET<RegisterType::HL>(5);
                    return 4;
                }
                case 0xF: {
                    SET<RegisterType::A>(5);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        case 0xF: {
            switch (Bitwise::getSecondNibble(opcode)) {
                case 0x0: {
                    SET<RegisterType::B>(6);
                    return 2;
                }
                case 0x1: {
                    SET<RegisterType::C>(6);
                    return 2;
                }
                case 0x2: {
                    SET<RegisterType::D>(6);
                    return 2;
                }
                case 0x3: {
                    SET<RegisterType::E>(6);
                    return 2;
                }
                case 0x4: {
                    SET<RegisterType::H>(6);
                    return 2;
                }
                case 0x5: {
                    SET<RegisterType::L>(6);
                    return 2;
                }
                case 0x6: {
                    SET<RegisterType::HL>(6);
                    return 4;
                }
                case 0x7: {
                    SET<RegisterType::A>(6);
                    return 2;
                }
                case 0x8: {
                    SET<RegisterType::B>(7);
                    return 2;
                }
                case 0x9: {
                    SET<RegisterType::C>(7);
                    return 2;
                }
                case 0xA: {
                    SET<RegisterType::D>(7);
                    return 2;
                }
                case 0xB: {
                    SET<RegisterType::E>(7);
                    return 2;
                }
                case 0xC: {
                    SET<RegisterType::H>(7);
                    return 2;
                }
                case 0xD: {
                    SET<RegisterType::L>(7);
                    return 2;
                }
                case 0xE: {
                    SET<RegisterType::HL>(7);
                    return 4;
                }
                case 0xF: {
                    SET<RegisterType::A>(7);
                    return 2;
                }
                default:
                    return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
            }
        }
        default:
            return logInvalidOpcode(Bitwise::concatBytes(opcode, 0xCB));
    }
}