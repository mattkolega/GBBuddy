#include "cpu.h"

#include <bit>
#include <sstream>
#include <utility>

#include <fmt/base.h>

#include "common/bits.h"

#include "bus.h"

// Make sure we're running on a little-endian system, CPU won't work properly if not
static_assert(std::endian::native == std::endian::little,
    "Host system must be little-endian");

std::string CPUState::toString() {
    std::ostringstream stream {};
    stream << "A: " << +a
           << " F: " << +f
           << " B: " << +b
           << " C: " << +c
           << " D: " << +d
           << " E: " << +e
           << " H: " << +h
           << " L: " << +l
           << " SP: " << +sp
           << " PC: " << +pc;
    return stream.str();
}

bool operator==(CPUState &state1, CPUState &state2) {
    if (state1.a != state2.a) return false;
    if (state1.f != state2.f) return false;
    if (state1.b != state2.b) return false;
    if (state1.c != state2.c) return false;
    if (state1.d != state2.d) return false;
    if (state1.e != state2.e) return false;
    if (state1.h != state2.h) return false;
    if (state1.l != state2.l) return false;
    if (state1.sp != state2.sp) return false;
    if (state1.pc != state2.pc) return false;
    if (state1.ime != state2.ime) return false;

    return true;
}

void CPU::step() {
    u8 opcode {m_bus.read(m_pc++)};
    decode(opcode);
}

void CPU::setState(CPUState state) {
    m_af.hi = state.a;
    m_af.lo = state.f;
    m_bc.hi = state.b;
    m_bc.lo = state.c;
    m_de.hi = state.d;
    m_de.lo = state.e;
    m_hl.hi = state.h;
    m_hl.lo = state.l;
    m_sp = state.sp;
    m_pc = state.pc;
    m_ime = state.ime;
}

CPUState CPU::getState() {
    return CPUState{
        .a = m_af.hi,
        .f = m_af.lo,
        .b = m_bc.hi,
        .c = m_bc.lo,
        .d = m_de.hi,
        .e = m_de.lo,
        .h = m_hl.hi,
        .l = m_hl.lo,
        .sp = m_sp,
        .pc = m_pc
    };
}

void CPU::printState() {
    fmt::println(
        "A:{:02X} F:{:02X} B:{:02X} C:{:02X} D:{:02X} E:{:02X} H:{:02X} L:{:02X} SP:{:04X} PC:{:04X} PCMEM:{:02X},{:02X},{:02X},{:02X}",
        m_af.hi, m_af.lo, m_bc.hi, m_bc.lo, m_de.hi, m_de.lo ,m_hl.hi, m_hl.lo, m_sp, m_pc, m_bus.read(m_pc), m_bus.read(m_pc+1), m_bus.read(m_pc+2), m_bus.read(m_pc+3)
    );
}

u8 CPU::readReg8(Reg8 reg) const {
    using enum Reg8;

    switch (reg) {
    case A: return m_af.hi;
    case F: return m_af.lo;
    case B: return m_bc.hi;
    case C: return m_bc.lo;
    case D: return m_de.hi;
    case E: return m_de.lo;
    case H: return m_hl.hi;
    case L: return m_hl.lo;
    case HLMem: return m_bus.read(m_hl.full);
    }

    std::unreachable();
}

u16 CPU::readReg16(Reg16 reg) const {
    using enum Reg16;

    switch (reg) {
    case AF: return m_af.full;
    case BC: return m_bc.full;
    case DE: return m_de.full;
    case HL: return m_hl.full;
    case SP: return m_sp;
    }

    std::unreachable();
}

void CPU::writeReg8(Reg8 reg, u8 value) {
    using enum Reg8;

    switch (reg) {
    case A: m_af.hi = value; break;
    case F: m_af.lo = value; break;
    case B: m_bc.hi = value; break;
    case C: m_bc.lo = value; break;
    case D: m_de.hi = value; break;
    case E: m_de.lo = value; break;
    case H: m_hl.hi = value; break;
    case L: m_hl.lo = value; break;
    case HLMem: m_bus.write(m_hl.full, value); break;
    }

    std::unreachable();
}

void CPU::writeReg16(Reg16 reg, u16 value) {
    using enum Reg16;

    switch (reg) {
    case AF: m_af.full = value; break;
    case BC: m_bc.full = value; break;
    case DE: m_de.full = value; break;
    case HL: m_hl.full = value; break;
    case SP: m_sp = value; break;
    }

    std::unreachable();
}

u8 CPU::getCarry() const     { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::C)); }
u8 CPU::getHalfCarry() const { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::H)); }
u8 CPU::getSubtract() const  { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::N)); }
u8 CPU::getZero() const      { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::Z)); }

void CPU::setCarry(bool value)     { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::C), value); }
void CPU::setHalfCarry(bool value) { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::H), value); }
void CPU::setSubtract(bool value)  { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::N), value); }
void CPU::setZero(bool value)      { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::Z), value); }

void CPU::pushToStack(u16 value) {
    m_sp--;
    m_bus.write(m_sp, (value >> 8) & 0xFF);
    m_sp--;
    m_bus.write(m_sp, value & 0xFF);
}

u16 CPU::popStack() {
    u8 lo = m_bus.read(m_sp);
    m_sp++;
    u8 hi = m_bus.read(m_sp);
    m_sp++;
    return bits::concatBytes(lo, hi);
}

/**
8-bit Arithmetic and Logic Instructions
 */

void CPU::ADC(u8 value) {
    u8 originalValue = readReg8(Reg8::A);
    u8 newValue = originalValue + value + getCarry();

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(bits::checkHalfCarryAdd(originalValue, value, getCarry()));
    setCarry((originalValue + value + getCarry()) > 0xFF);

    writeReg8(Reg8::A, newValue);
}

void CPU::ADD8(u8 value) {
    u8 originalValue = readReg8(Reg8::A);
    u8 newValue = originalValue + value;

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(bits::checkHalfCarryAdd(originalValue, value));
    setCarry((originalValue + value) > 0xFF);

    writeReg8(Reg8::A, newValue);
}

void CPU::AND(u8 value) {
    u8 newValue = readReg8(Reg8::A) & value;

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(1);
    setCarry(0);

    writeReg8(Reg8::A, newValue);
}

void CPU::CP(u8 value) {
    u8 accumulator = readReg8(Reg8::A);
    u8 result = accumulator - value;

    // Set flags
    setZero(result == 0);
    setSubtract(1);
    setHalfCarry(bits::checkHalfCarrySub(accumulator, value));
    setCarry((accumulator - value) < 0);
}

void CPU::DEC8(Reg8 reg) {
    u8 originalValue = readReg8(reg);
    u8 newValue = originalValue - 1;

    setZero(newValue == 0);
    setSubtract(1);
    setHalfCarry(bits::checkHalfCarrySub(newValue, 1));

    writeReg8(reg, newValue);
}

void CPU::INC8(Reg8 reg) {
    u8 originalValue = readReg8(reg);
    u8 newValue = originalValue + 1;

    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(bits::checkHalfCarryAdd(newValue, 1));

    writeReg8(reg, newValue);
}

void CPU::OR(u8 value) {
    u8 newValue = readReg8(Reg8::A) | value;

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
    setCarry(0);

    writeReg8(Reg8::A, newValue);
}

void CPU::SBC(u8 value) {
    u8 originalValue = readReg8(Reg8::A);
    u8 newValue = originalValue - value - getCarry();

    // Set flags
    setZero(newValue == 0);
    setSubtract(1);
    setHalfCarry(bits::checkHalfCarrySub(originalValue, value, getCarry()));
    setCarry((originalValue - value - getCarry()) < 0);

    writeReg8(Reg8::A, newValue);
}

void CPU::SUB(u8 value) {
    u8 originalValue = readReg8(Reg8::A);
    u8 newValue = originalValue - value;

    // Set flags
    setZero(newValue == 0);
    setSubtract(1);
    setHalfCarry(bits::checkHalfCarrySub(originalValue, value));
    setCarry((originalValue - value) < 0);

    writeReg8(Reg8::A, newValue);
}

void CPU::XOR(u8 value) {
    u8 newValue = readReg8(Reg8::A) ^ value;

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
    setCarry(0);

    writeReg8(Reg8::A, newValue);
}

/**
16-bit Arithmetic Instructions
 */

void CPU::ADD16(Reg16 reg) {
    u16 originalValue = readReg16(Reg16::HL);
    u16 newValue = originalValue + readReg16(reg);

    // Set flags
    setSubtract(0);
    setHalfCarry(bits::checkHalfCarryAdd(originalValue, readReg16(reg)));
    setCarry((originalValue + readReg16(reg)) > 0xFFFF);

    writeReg16(Reg16::HL, newValue);
}

// Decrements 16-bit value
void CPU::DEC16(Reg16 reg) {
    writeReg16(reg, readReg16(reg)-1);
}

// Increments 16-bit value
void CPU::INC16(Reg16 reg) {
    writeReg16(reg, readReg16(reg)+1);
}

/**
Bit Operation Instructions
 */

void CPU::BIT(Reg8 reg, u8 bitPos) {
    setZero(bits::getBitInByte(readReg8(reg), bitPos) == 0);
    setSubtract(0);
    setHalfCarry(1);
}

// Resets bit in value
void CPU::RES(Reg8 reg, u8 bitPos) {
    writeReg8(reg, bits::modifyBitInByte(readReg8(reg), bitPos, 0));
}

// Sets bit
void CPU::SET(Reg8 reg, u8 bitPos) {
    writeReg8(reg, bits::modifyBitInByte(readReg8(reg), bitPos, 1));
}

// Swaps upper 4 bits and lower 4 bits of value
void CPU::SWAP(Reg8 reg) {
    u8 originalValue = readReg8(reg);
    u8 newValue = bits::swapNibbles(originalValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
    setCarry(0);

    writeReg8(reg, newValue);
}

/**
Bit Shift Instructions
 */

u8 CPU::RL(u8 value) {
    auto carryValue = getCarry();  // Get current carry flag value
    setCarry(bits::getBitInByte(value, 7));  // Set carry flag to leftmost bit
    u8 newValue = std::rotl(value, 1);
    return bits::modifyBitInByte(newValue, 0, carryValue);
}

void CPU::RL_r8(Reg8 reg) {
    u8 newValue = RL(m_bus.read(readReg8(reg)));
    m_bus.write(readReg8(reg), newValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::RLA() {
    writeReg8(Reg8::A, RL(readReg8(Reg8::A)));

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(0);
}

u8 CPU::RLC(u8 value) {
    setCarry(bits::getBitInByte(value, 7));  // Set carry flag to leftmost bit
    return std::rotl(value, 1);
}

void CPU::RLC_r8(Reg8 reg) {
    u8 newValue = RLC(m_bus.read(readReg8(reg)));
    m_bus.write(readReg8(reg), newValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::RLCA() {
    writeReg8(Reg8::A, RLC(readReg8(Reg8::A)));

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(0);
}

u8 CPU::RR(u8 value) {
    auto carryValue = getCarry();  // Get current carry flag value
    setCarry(bits::getBitInByte(value, 0));  // Set carry flag to right-most bit
    u8 newValue = std::rotr(value, 1);
    return bits::modifyBitInByte(newValue, 7, carryValue);
}

void CPU::RR_r8(Reg8 reg) {
    u8 newValue = RR(m_bus.read(readReg8(reg)));
    m_bus.write(readReg8(reg), newValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::RRA() {
    writeReg8(Reg8::A, RR(readReg8(Reg8::A)));

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(0);
}

u8 CPU::RRC(u8 value) {
    setCarry(bits::getBitInByte(value, 0));  // Set carry flag to right-most bit
    return std::rotr(value, 1);
}

void CPU::RRC_r8(Reg8 reg) {
     u8 newValue = RRC(m_bus.read(readReg8(reg)));
    m_bus.write(readReg8(reg), newValue);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::RRCA() {
    writeReg8(Reg8::A, RRC(readReg8(Reg8::A)));

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(0);
}

void CPU::SLA(Reg8 reg) {
    u8 originalValue = readReg8(reg);
    u8 newValue = originalValue << 1;

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
    setCarry(bits::getBitInByte(originalValue, 7));

    writeReg8(reg, newValue);
}

void CPU::SRA(Reg8 reg) {
    u8 originalValue = readReg8(reg);
    u8 bit7 = bits::getBitInByte(originalValue, 7);
    u8 newValue = (originalValue >> 1) | (bit7 << 7);

    // Set flags
    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
    setCarry(bits::getBitInByte(originalValue, 0));

    writeReg8(reg, newValue);
}

void CPU::SRL(Reg8 reg) {
    u8 originalValue = readReg8(reg);
    u8 newValue = originalValue >> 1;

    setZero(newValue == 0);
    setSubtract(0);
    setHalfCarry(0);
    setCarry(bits::getBitInByte(originalValue, 0));

    writeReg8(reg, newValue);
}

//
// Load Instructions
//

void CPU::LD8(Reg8 reg, u8 value) {
    writeReg8(reg, value);
}

void CPU::LD16(Reg16 reg, u16 value) {
    writeReg16(reg, value);
}

void CPU::LD_n16_A(u16 address) {
    m_bus.write(address, readReg8(Reg8::A));
}

void CPU::LD_A_n16(u16 address) {
    writeReg8(Reg8::A, m_bus.read(address));
}

void CPU::LDH_n16_A(u8 lowByte) {
    m_bus.write(0xFF00 | lowByte, readReg8(Reg8::A));
}

void CPU::LDH_C_A() {
    m_bus.write(0xFF00 | readReg8(Reg8::C), readReg8(Reg8::A));
}

void CPU::LDH_A_n16(u8 lowByte) {
    writeReg8(Reg8::A, m_bus.read(0xFF00 | lowByte));
}

void CPU::LDH_A_C() {
    writeReg8(Reg8::A, m_bus.read(0xFF00 | readReg8(Reg8::C)));
}

void CPU::LD_HLI_A() {
    m_bus.write(m_hl.full, readReg8(Reg8::A));
    m_hl.full++;
}

void CPU::LD_HLD_A() {
    m_bus.write(m_hl.full, readReg8(Reg8::A));
    m_hl.full--;
}

void CPU::LD_A_HLI() {
    writeReg8(Reg8::A, m_bus.read(m_hl.full));
    m_hl.full++;
}

void CPU::LD_A_HLD() {
    writeReg8(Reg8::A, m_bus.read(m_hl.full));
    m_hl.full--;
}

/**
Jumps and Subroutines
 */

void CPU::CALL(bool condition) {
    if (condition) {
        pushToStack(m_pc);
        u16 address = bits::concatBytes(m_bus.read(m_pc), m_bus.read(m_pc+1));
        m_pc = address;
    } else {
        m_pc += 2;
    }
}

void CPU::JP(bool condition) {
    if (condition) {
        u16 address = bits::concatBytes(m_bus.read(m_pc), m_bus.read(m_pc+1));
        m_pc = address;
    } else {
        m_pc += 2;
    }
}

void CPU::JP_HL() {
    m_pc = m_hl.full;
}

void CPU::JR(bool condition) {
    if (condition) {
        i8 offset = m_bus.read(m_pc++);
        m_pc += offset;
    } else {
        m_pc++;
    }
}

void CPU::RET(bool condition) {
    if (condition) {
        m_pc = popStack();
    }
}

void CPU::RETI() {
    m_pc = popStack();
    m_ime = 1;
}

void CPU::RST(u8 vec) {
    pushToStack(m_pc);
    m_pc = vec;
}

/**
Stack Operation Instructions
 */

void CPU::ADD_HL_SP() {
    u16 originalValue = m_hl.full;
    m_hl.full += m_sp;

    // Set flags
    setSubtract(0);
    setHalfCarry(bits::checkHalfCarryAdd(originalValue, m_sp));
    setCarry((originalValue + m_sp) > 0xFFFF);
}

void CPU::ADD_SP_e8(i8 value) {
    u16 originalValue = m_sp;
    m_sp += value;

    // Set flags
    setZero(0);
    setSubtract(0);
    setHalfCarry(bits::checkHalfCarryAdd(static_cast<u8>(originalValue), value));
    setCarry(((originalValue & 0xFF) + static_cast<u8>(value)) > 0xFF);
}

void CPU::LD_SP_n16(u16 value) {
    m_sp = value;
}

void CPU::LD_a16_SP(u16 address) {
    m_bus.write(address, m_sp & 0xFF);
    m_bus.write(address + 1, (m_sp >> 8) & 0xFF);
}

void CPU::LD_HL_SP(i8 value) {
    u16 result = m_sp + value;
    m_hl.full = result;

    setZero(0);
    setSubtract(0);
    setHalfCarry(bits::checkHalfCarryAdd(static_cast<u8>(m_sp), value));
    setCarry(((m_sp & 0xFF) + static_cast<u8>(value)) > 0xFF);
}

void CPU::LD_SP_HL() {
    m_sp = m_hl.full;
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
    writeReg8(Reg8::A, ~readReg8(Reg8::A));

    // Set flags
    setSubtract(1);
    setHalfCarry(1);
}

void CPU::DAA() {
    u8 accumulator = readReg8(Reg8::A);
    u8 adjustment {0};

    if (getSubtract() == 0 && (accumulator & 0xF) > 0x9 || getHalfCarry() == 1) {
        adjustment |= 0x06;
    }

    if (getSubtract() == 0 && accumulator > 0x99 || getCarry() == 1) {
        adjustment |= 0x60;
        setCarry(1);
    }

    (getSubtract() == 0) ? accumulator += adjustment : accumulator -= adjustment;

    // Set flags
    setZero(accumulator == 0);
    setHalfCarry(0);

    writeReg8(Reg8::A, accumulator);
}

void CPU::DI() {
    m_ime = 0;
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

void CPU::decode(u8 opcode) {
    using enum Reg8;
    using enum Reg16;

    switch (opcode) {
    case 0x00: NOP(); break;
    case 0x01: LD16(BC, m_bus.read(m_pc++)); break;
    case 0x02: LD_n16_A(readReg16(BC)); break;
    case 0x03: INC16(BC); break;
    case 0x04: INC8(B); break;
    case 0x05: DEC8(B); break;
    case 0x06: LD8(B, m_bus.read(m_pc++)); break;
    case 0x07: RLCA(); break;
    case 0x08: LD_a16_SP(bits::concatBytes(m_bus.read(m_pc), m_bus.read(m_pc+1))); m_pc+=2; break;
    case 0x09: ADD16(BC); break;
    case 0x0A: LD_A_n16(readReg16(BC)); break;
    case 0x0B: DEC16(BC); break;
    case 0x0C: INC8(C); break;
    case 0x0D: DEC8(C); break;
    case 0x0E: LD8(C, m_bus.read(m_pc++)); break;
    case 0x0F: RRCA(); break;

    case 0x10: STOP(); m_pc++; break;
    case 0x11: LD16(DE, m_bus.read(m_pc++)); break;
    case 0x12: LD_n16_A(readReg16(DE)); break;
    case 0x13: INC16(DE); break;
    case 0x14: INC8(D); break;
    case 0x15: DEC8(D); break;
    case 0x16: LD8(D, m_bus.read(m_pc++)); break;
    case 0x17: RLA(); break;
    case 0x18: JR(m_bus.read(m_pc++)); break;
    case 0x19: ADD16(DE); break;
    case 0x1A: LD_A_n16(readReg16(DE)); break;
    case 0x1B: DEC16(DE); break;
    case 0x1C: INC8(E); break;
    case 0x1D: DEC8(E); break;
    case 0x1E: LD8(E, m_bus.read(m_pc++)); break;
    case 0x1F: RRA(); break;

    case 0x20: JR(getZero() == 0); break;
    case 0x21: LD16(HL, m_bus.read(m_pc++)); break;
    case 0x22: LD_HLI_A(); break;
    case 0x23: INC16(HL); break;
    case 0x24: INC8(H); break;
    case 0x25: DEC8(H); break;
    case 0x26: LD8(H, m_bus.read(m_pc++)); break;
    case 0x27: DAA(); break;
    case 0x28: JR(getZero() == 1); break;
    case 0x29: ADD16(HL); break;
    case 0x2A: LD_A_HLI(); break;
    case 0x2B: DEC16(HL); break;
    case 0x2C: INC8(L); break;
    case 0x2D: DEC8(L); break;
    case 0x2E: LD8(L, m_bus.read(m_pc++)); break;
    case 0x2F: CPL(); break;

    case 0x30: JR(getCarry() == 0); break;
    case 0x31: LD_SP_n16(bits::concatBytes(m_bus.read(m_pc), m_bus.read(m_pc+1))); m_pc+=2; break;
    case 0x32: LD_HLD_A(); break;
    case 0x33: INC16(SP); break;
    case 0x34: INC8(HLMem); break;
    case 0x35: DEC8(HLMem); break;
    case 0x36: LD8(HLMem, m_bus.read(m_pc++)); break;
    case 0x37: SCF(); break;
    case 0x38: JR(getCarry() == 0); break;
    case 0x39: ADD_HL_SP(); break;
    case 0x3A: LD_A_HLD(); break;
    case 0x3B: DEC16(SP); break;
    case 0x3C: INC8(A); break;
    case 0x3D: DEC8(A); break;
    case 0x3E: LD8(A, m_bus.read(m_pc++)); break;
    case 0x3F: CCF(); break;

    case 0x40: LD8(B, readReg8(B)); break;
    case 0x41: LD8(B, readReg8(C)); break;
    case 0x42: LD8(B, readReg8(D)); break;
    case 0x43: LD8(B, readReg8(E)); break;
    case 0x44: LD8(B, readReg8(H)); break;
    case 0x45: LD8(B, readReg8(L)); break;
    case 0x46: LD8(B, readReg8(HLMem)); break;
    case 0x47: LD8(B, readReg8(A)); break;
    case 0x48: LD8(C, readReg8(B)); break;
    case 0x49: LD8(C, readReg8(C)); break;
    case 0x4A: LD8(C, readReg8(D)); break;
    case 0x4B: LD8(C, readReg8(E)); break;
    case 0x4C: LD8(C, readReg8(H)); break;
    case 0x4D: LD8(C, readReg8(L)); break;
    case 0x4E: LD8(C, readReg8(HLMem)); break;
    case 0x4F: LD8(C, readReg8(A)); break;

    case 0x50: LD8(D, readReg8(B)); break;
    case 0x51: LD8(D, readReg8(C)); break;
    case 0x52: LD8(D, readReg8(D)); break;
    case 0x53: LD8(D, readReg8(E)); break;
    case 0x54: LD8(D, readReg8(H)); break;
    case 0x55: LD8(D, readReg8(L)); break;
    case 0x56: LD8(D, readReg8(HLMem)); break;
    case 0x57: LD8(D, readReg8(A)); break;
    case 0x58: LD8(E, readReg8(B)); break;
    case 0x59: LD8(E, readReg8(C)); break;
    case 0x5A: LD8(E, readReg8(D)); break;
    case 0x5B: LD8(E, readReg8(E)); break;
    case 0x5C: LD8(E, readReg8(H)); break;
    case 0x5D: LD8(E, readReg8(L)); break;
    case 0x5E: LD8(E, readReg8(HLMem)); break;
    case 0x5F: LD8(E, readReg8(A)); break;

    case 0x60: LD8(H, readReg8(B)); break;
    case 0x61: LD8(H, readReg8(C)); break;
    case 0x62: LD8(H, readReg8(D)); break;
    case 0x63: LD8(H, readReg8(E)); break;
    case 0x64: LD8(H, readReg8(H)); break;
    case 0x65: LD8(H, readReg8(L)); break;
    case 0x66: LD8(H, readReg8(HLMem)); break;
    case 0x67: LD8(H, readReg8(A)); break;
    case 0x68: LD8(L, readReg8(B)); break;
    case 0x69: LD8(L, readReg8(C)); break;
    case 0x6A: LD8(L, readReg8(D)); break;
    case 0x6B: LD8(L, readReg8(E)); break;
    case 0x6C: LD8(L, readReg8(H)); break;
    case 0x6D: LD8(L, readReg8(L)); break;
    case 0x6E: LD8(L, readReg8(HLMem)); break;
    case 0x6F: LD8(L, readReg8(A)); break;

    case 0x70: LD8(HLMem, readReg8(B)); break;
    case 0x71: LD8(HLMem, readReg8(C)); break;
    case 0x72: LD8(HLMem, readReg8(D)); break;
    case 0x73: LD8(HLMem, readReg8(E)); break;
    case 0x74: LD8(HLMem, readReg8(H)); break;
    case 0x75: LD8(HLMem, readReg8(L)); break;
    case 0x76: HALT(); break;
    case 0x77: LD8(HLMem, readReg8(A)); break;
    case 0x78: LD8(A, readReg8(B)); break;
    case 0x79: LD8(A, readReg8(C)); break;
    case 0x7A: LD8(A, readReg8(D)); break;
    case 0x7B: LD8(A, readReg8(E)); break;
    case 0x7C: LD8(A, readReg8(H)); break;
    case 0x7D: LD8(A, readReg8(L)); break;
    case 0x7E: LD8(A, readReg8(HLMem)); break;
    case 0x7F: LD8(A, readReg8(A)); break;

    case 0x80: ADD8(readReg8(B)); break;
    case 0x81: ADD8(readReg8(C)); break;
    case 0x82: ADD8(readReg8(D)); break;
    case 0x83: ADD8(readReg8(E)); break;
    case 0x84: ADD8(readReg8(H)); break;
    case 0x85: ADD8(readReg8(L)); break;
    case 0x86: ADD8(readReg8(HLMem)); break;
    case 0x87: ADD8(readReg8(A)); break;
    case 0x88: ADC(readReg8(B)); break;
    case 0x89: ADC(readReg8(C)); break;
    case 0x8A: ADC(readReg8(D)); break;
    case 0x8B: ADC(readReg8(E)); break;
    case 0x8C: ADC(readReg8(H)); break;
    case 0x8D: ADC(readReg8(L)); break;
    case 0x8E: ADC(readReg8(HLMem)); break;
    case 0x8F: ADC(readReg8(A)); break;

    case 0x90: SUB(readReg8(B)); break;
    case 0x91: SUB(readReg8(C)); break;
    case 0x92: SUB(readReg8(D)); break;
    case 0x93: SUB(readReg8(E)); break;
    case 0x94: SUB(readReg8(H)); break;
    case 0x95: SUB(readReg8(L)); break;
    case 0x96: SUB(readReg8(HLMem)); break;
    case 0x97: SUB(readReg8(A)); break;
    case 0x98: SBC(readReg8(B)); break;
    case 0x99: SBC(readReg8(C)); break;
    case 0x9A: SBC(readReg8(D)); break;
    case 0x9B: SBC(readReg8(E)); break;
    case 0x9C: SBC(readReg8(H)); break;
    case 0x9D: SBC(readReg8(L)); break;
    case 0x9E: SBC(readReg8(HLMem)); break;
    case 0x9F: SBC(readReg8(A)); break;

    case 0xA0: AND(readReg8(B)); break;
    case 0xA1: AND(readReg8(C)); break;
    case 0xA2: AND(readReg8(D)); break;
    case 0xA3: AND(readReg8(E)); break;
    case 0xA4: AND(readReg8(H)); break;
    case 0xA5: AND(readReg8(L)); break;
    case 0xA6: AND(readReg8(HLMem)); break;
    case 0xA7: AND(readReg8(A)); break;
    case 0xA8: XOR(readReg8(B)); break;
    case 0xA9: XOR(readReg8(C)); break;
    case 0xAA: XOR(readReg8(D)); break;
    case 0xAB: XOR(readReg8(E)); break;
    case 0xAC: XOR(readReg8(H)); break;
    case 0xAD: XOR(readReg8(L)); break;
    case 0xAE: XOR(readReg8(HLMem)); break;
    case 0xAF: XOR(readReg8(A)); break;

    case 0xB0: OR(readReg8(B)); break;
    case 0xB1: OR(readReg8(C)); break;
    case 0xB2: OR(readReg8(D)); break;
    case 0xB3: OR(readReg8(E)); break;
    case 0xB4: OR(readReg8(H)); break;
    case 0xB5: OR(readReg8(L)); break;
    case 0xB6: OR(readReg8(HLMem)); break;
    case 0xB7: OR(readReg8(A)); break;
    case 0xB8: CP(readReg8(B)); break;
    case 0xB9: CP(readReg8(C)); break;
    case 0xBA: CP(readReg8(D)); break;
    case 0xBB: CP(readReg8(E)); break;
    case 0xBC: CP(readReg8(H)); break;
    case 0xBD: CP(readReg8(L)); break;
    case 0xBE: CP(readReg8(HLMem)); break;
    case 0xBF: CP(readReg8(A)); break;

    case 0xC0: RET(getZero() == 0); break;
    case 0xC1: POP(BC); break;
    case 0xC2: JP(getZero() == 0); break;
    case 0xC3: JP(); break;
    case 0xC4: CALL(getZero() == 0); break;
    case 0xC5: PUSH(BC); break;
    case 0xC6: ADD8(m_bus.read(m_pc++)); break;
    case 0xC7: RST(0x00); break;
    case 0xC8: RET(getZero() == 1); break;
    case 0xC9: RET(); break;
    case 0xCA: JP(getZero() == 1); break;
    case 0xCB: decodeCB(m_bus.read(m_pc++)); break;
    case 0xCC: CALL(getZero() == 1); break;
    case 0xCD: CALL(); break;
    case 0xCE: ADC(m_bus.read(m_pc++)); break;
    case 0xCF: RST(0x08); break;

    case 0xD0: RET(getCarry() == 0); break;
    case 0xD1: POP(DE); break;
    case 0xD2: JP(getCarry() == 0); break;
    case 0xD3: break;
    case 0xD4: CALL(getCarry() == 0); break;
    case 0xD5: PUSH(DE); break;
    case 0xD6: SUB(m_bus.read(m_pc++)); break;
    case 0xD7: RST(0x10); break;
    case 0xD8: RET(getCarry() == 1); break;
    case 0xD9: RETI(); break;
    case 0xDA: JP(getCarry() == 1); break;
    case 0xDB: break;
    case 0xDC: CALL(getCarry() == 1); break;
    case 0xDD: break;
    case 0xDE: SBC(m_bus.read(m_pc++)); break;
    case 0xDF: RST(0x18); break;

    case 0xE0: LDH_n16_A(m_bus.read(m_pc++)); break;
    case 0xE1: POP(HL); break;
    case 0xE2: LDH_C_A(); break;
    case 0xE3: break;
    case 0xE4: break;
    case 0xE5: PUSH(HL); break;
    case 0xE6: AND(m_bus.read(m_pc++)); break;
    case 0xE7: RST(0x20); break;
    case 0xE8: ADD_SP_e8(m_bus.read(m_pc++)); break;
    case 0xE9: JP_HL(); break;
    case 0xEA: LD_n16_A(bits::concatBytes(m_bus.read(m_pc), m_bus.read(m_pc+1))); m_pc+=2; break;
    case 0xEB: break;
    case 0xEC: break;
    case 0xED: break;
    case 0xEE: XOR(m_bus.read(m_pc++)); break;
    case 0xEF: RST(0x18); break;

    case 0xF0: LDH_A_n16(m_bus.read(m_pc++)); break;
    case 0xF1: POP(AF); break;
    case 0xF2: LDH_A_C(); break;
    case 0xF3: DI(); break;
    case 0xF4: break;
    case 0xF5: PUSH(AF); break;
    case 0xF6: OR(m_bus.read(m_pc++)); break;
    case 0xF7: RST(0x30); break;
    case 0xF8: LD_HL_SP(m_bus.read(m_pc++)); break;
    case 0xF9: LD_SP_HL(); break;
    case 0xFA: LD_A_n16(bits::concatBytes(m_bus.read(m_pc), m_bus.read(m_pc+1))); m_pc+=2; break;
    case 0xFB: EI();
    case 0xFC: break;
    case 0xFD: break;
    case 0xFE: CP(m_bus.read(m_pc++)); break;
    case 0xFF: RST(0x38);
    }

    std::unreachable();
}

void CPU::decodeCB(u8 opcode) {
    using enum Reg8;
    using enum Reg16;

    switch (opcode) {
    case 0x00: RLC_r8(B); break;
    case 0x01: RLC_r8(C); break;
    case 0x02: RLC_r8(D); break;
    case 0x03: RLC_r8(E); break;
    case 0x04: RLC_r8(H); break;
    case 0x05: RLC_r8(L); break;
    case 0x06: RLC_r8(HLMem); break;
    case 0x07: RLC_r8(A); break;
    case 0x08: RRC_r8(B); break;
    case 0x09: RRC_r8(C); break;
    case 0x0A: RRC_r8(D); break;
    case 0x0B: RRC_r8(E); break;
    case 0x0C: RRC_r8(H); break;
    case 0x0D: RRC_r8(L); break;
    case 0x0E: RRC_r8(HLMem); break;
    case 0x0F: RRC_r8(A); break;

    case 0x10: RL_r8(B); break;
    case 0x11: RL_r8(C); break;
    case 0x12: RL_r8(D); break;
    case 0x13: RL_r8(E); break;
    case 0x14: RL_r8(H); break;
    case 0x15: RL_r8(L); break;
    case 0x16: RL_r8(HLMem); break;
    case 0x17: RL_r8(A); break;
    case 0x18: RR_r8(B); break;
    case 0x19: RR_r8(C); break;
    case 0x1A: RR_r8(D); break;
    case 0x1B: RR_r8(E); break;
    case 0x1C: RR_r8(H); break;
    case 0x1D: RR_r8(L); break;
    case 0x1E: RR_r8(HLMem); break;
    case 0x1F: RR_r8(A); break;

    case 0x20: SLA(B); break;
    case 0x21: SLA(C); break;
    case 0x22: SLA(D); break;
    case 0x23: SLA(E); break;
    case 0x24: SLA(H); break;
    case 0x25: SLA(L); break;
    case 0x26: SLA(HLMem); break;
    case 0x27: SLA(A); break;
    case 0x28: SRA(B); break;
    case 0x29: SRA(C); break;
    case 0x2A: SRA(D); break;
    case 0x2B: SRA(E); break;
    case 0x2C: SRA(H); break;
    case 0x2D: SRA(L); break;
    case 0x2E: SRA(HLMem); break;
    case 0x2F: SRA(A); break;

    case 0x30: SWAP(B); break;
    case 0x31: SWAP(C); break;
    case 0x32: SWAP(D); break;
    case 0x33: SWAP(E); break;
    case 0x34: SWAP(H); break;
    case 0x35: SWAP(L); break;
    case 0x36: SWAP(HLMem); break;
    case 0x37: SWAP(A); break;
    case 0x38: SRL(B); break;
    case 0x39: SRL(C); break;
    case 0x3A: SRL(D); break;
    case 0x3B: SRL(E); break;
    case 0x3C: SRL(H); break;
    case 0x3D: SRL(L); break;
    case 0x3E: SRL(HLMem); break;
    case 0x3F: SRL(A); break;

    case 0x40: BIT(B, 0); break;
    case 0x41: BIT(C, 0); break;
    case 0x42: BIT(D, 0); break;
    case 0x43: BIT(E, 0); break;
    case 0x44: BIT(H, 0); break;
    case 0x45: BIT(L, 0); break;
    case 0x46: BIT(HLMem, 0); break;
    case 0x47: BIT(A, 0); break;
    case 0x48: BIT(B, 1); break;
    case 0x49: BIT(C, 1); break;
    case 0x4A: BIT(D, 1); break;
    case 0x4B: BIT(E, 1); break;
    case 0x4C: BIT(H, 1); break;
    case 0x4D: BIT(L, 1); break;
    case 0x4E: BIT(HLMem, 1); break;
    case 0x4F: BIT(A, 1); break;

    case 0x50: BIT(B, 2); break;
    case 0x51: BIT(C, 2); break;
    case 0x52: BIT(D, 2); break;
    case 0x53: BIT(E, 2); break;
    case 0x54: BIT(H, 2); break;
    case 0x55: BIT(L, 2); break;
    case 0x56: BIT(HLMem, 2); break;
    case 0x57: BIT(A, 2); break;
    case 0x58: BIT(B, 3); break;
    case 0x59: BIT(C, 3); break;
    case 0x5A: BIT(D, 3); break;
    case 0x5B: BIT(E, 3); break;
    case 0x5C: BIT(H, 3); break;
    case 0x5D: BIT(L, 3); break;
    case 0x5E: BIT(HLMem, 3); break;
    case 0x5F: BIT(A, 3); break;

    case 0x60: BIT(B, 4); break;
    case 0x61: BIT(C, 4); break;
    case 0x62: BIT(D, 4); break;
    case 0x63: BIT(E, 4); break;
    case 0x64: BIT(H, 4); break;
    case 0x65: BIT(L, 4); break;
    case 0x66: BIT(HLMem, 4); break;
    case 0x67: BIT(A, 4); break;
    case 0x68: BIT(B, 5); break;
    case 0x69: BIT(C, 5); break;
    case 0x6A: BIT(D, 5); break;
    case 0x6B: BIT(E, 5); break;
    case 0x6C: BIT(H, 5); break;
    case 0x6D: BIT(L, 5); break;
    case 0x6E: BIT(HLMem, 5); break;
    case 0x6F: BIT(A, 5); break;

    case 0x70: BIT(B, 6); break;
    case 0x71: BIT(C, 6); break;
    case 0x72: BIT(D, 6); break;
    case 0x73: BIT(E, 6); break;
    case 0x74: BIT(H, 6); break;
    case 0x75: BIT(L, 6); break;
    case 0x76: BIT(HLMem, 6); break;
    case 0x77: BIT(A, 6); break;
    case 0x78: BIT(B, 7); break;
    case 0x79: BIT(C, 7); break;
    case 0x7A: BIT(D, 7); break;
    case 0x7B: BIT(E, 7); break;
    case 0x7C: BIT(H, 7); break;
    case 0x7D: BIT(L, 7); break;
    case 0x7E: BIT(HLMem, 7); break;
    case 0x7F: BIT(A, 7); break;

    case 0x80: RES(B, 0); break;
    case 0x81: RES(C, 0); break;
    case 0x82: RES(D, 0); break;
    case 0x83: RES(E, 0); break;
    case 0x84: RES(H, 0); break;
    case 0x85: RES(L, 0); break;
    case 0x86: RES(HLMem, 0); break;
    case 0x87: RES(A, 0); break;
    case 0x88: RES(B, 1); break;
    case 0x89: RES(C, 1); break;
    case 0x8A: RES(D, 1); break;
    case 0x8B: RES(E, 1); break;
    case 0x8C: RES(H, 1); break;
    case 0x8D: RES(L, 1); break;
    case 0x8E: RES(HLMem, 1); break;
    case 0x8F: RES(A, 1); break;

    case 0x90: RES(B, 2); break;
    case 0x91: RES(C, 2); break;
    case 0x92: RES(D, 2); break;
    case 0x93: RES(E, 2); break;
    case 0x94: RES(H, 2); break;
    case 0x95: RES(L, 2); break;
    case 0x96: RES(HLMem, 2); break;
    case 0x97: RES(A, 2); break;
    case 0x98: RES(B, 3); break;
    case 0x99: RES(C, 3); break;
    case 0x9A: RES(D, 3); break;
    case 0x9B: RES(E, 3); break;
    case 0x9C: RES(H, 3); break;
    case 0x9D: RES(L, 3); break;
    case 0x9E: RES(HLMem, 3); break;
    case 0x9F: RES(A, 3); break;

    case 0xA0: RES(B, 4); break;
    case 0xA1: RES(C, 4); break;
    case 0xA2: RES(D, 4); break;
    case 0xA3: RES(E, 4); break;
    case 0xA4: RES(H, 4); break;
    case 0xA5: RES(L, 4); break;
    case 0xA6: RES(HLMem, 4); break;
    case 0xA7: RES(A, 4); break;
    case 0xA8: RES(B, 5); break;
    case 0xA9: RES(C, 5); break;
    case 0xAA: RES(D, 5); break;
    case 0xAB: RES(E, 5); break;
    case 0xAC: RES(H, 5); break;
    case 0xAD: RES(L, 5); break;
    case 0xAE: RES(HLMem, 5); break;
    case 0xAF: RES(A, 5); break;

    case 0xB0: RES(B, 6); break;
    case 0xB1: RES(C, 6); break;
    case 0xB2: RES(D, 6); break;
    case 0xB3: RES(E, 6); break;
    case 0xB4: RES(H, 6); break;
    case 0xB5: RES(L, 6); break;
    case 0xB6: RES(HLMem, 6); break;
    case 0xB7: RES(A, 6); break;
    case 0xB8: RES(B, 7); break;
    case 0xB9: RES(C, 7); break;
    case 0xBA: RES(D, 7); break;
    case 0xBB: RES(E, 7); break;
    case 0xBC: RES(H, 7); break;
    case 0xBD: RES(L, 7); break;
    case 0xBE: RES(HLMem, 7); break;
    case 0xBF: RES(A, 7); break;

    case 0xC0: SET(B, 0); break;
    case 0xC1: SET(C, 0); break;
    case 0xC2: SET(D, 0); break;
    case 0xC3: SET(E, 0); break;
    case 0xC4: SET(H, 0); break;
    case 0xC5: SET(L, 0); break;
    case 0xC6: SET(HLMem, 0); break;
    case 0xC7: SET(A, 0); break;
    case 0xC8: SET(B, 1); break;
    case 0xC9: SET(C, 1); break;
    case 0xCA: SET(D, 1); break;
    case 0xCB: SET(E, 1); break;
    case 0xCC: SET(H, 1); break;
    case 0xCD: SET(L, 1); break;
    case 0xCE: SET(HLMem, 1); break;
    case 0xCF: SET(A, 1); break;

    case 0xD0: SET(B, 2); break;
    case 0xD1: SET(C, 2); break;
    case 0xD2: SET(D, 2); break;
    case 0xD3: SET(E, 2); break;
    case 0xD4: SET(H, 2); break;
    case 0xD5: SET(L, 2); break;
    case 0xD6: SET(HLMem, 2); break;
    case 0xD7: SET(A, 2); break;
    case 0xD8: SET(B, 3); break;
    case 0xD9: SET(C, 3); break;
    case 0xDA: SET(D, 3); break;
    case 0xDB: SET(E, 3); break;
    case 0xDC: SET(H, 3); break;
    case 0xDD: SET(L, 3); break;
    case 0xDE: SET(HLMem, 3); break;
    case 0xDF: SET(A, 3); break;

    case 0xE0: SET(B, 4); break;
    case 0xE1: SET(C, 4); break;
    case 0xE2: SET(D, 4); break;
    case 0xE3: SET(E, 4); break;
    case 0xE4: SET(H, 4); break;
    case 0xE5: SET(L, 4); break;
    case 0xE6: SET(HLMem, 4); break;
    case 0xE7: SET(A, 4); break;
    case 0xE8: SET(B, 5); break;
    case 0xE9: SET(C, 5); break;
    case 0xEA: SET(D, 5); break;
    case 0xEB: SET(E, 5); break;
    case 0xEC: SET(H, 5); break;
    case 0xED: SET(L, 5); break;
    case 0xEE: SET(HLMem, 5); break;
    case 0xEF: SET(A, 5); break;

    case 0xF0: SET(B, 6); break;
    case 0xF1: SET(C, 6); break;
    case 0xF2: SET(D, 6); break;
    case 0xF3: SET(E, 6); break;
    case 0xF4: SET(H, 6); break;
    case 0xF5: SET(L, 6); break;
    case 0xF6: SET(HLMem, 6); break;
    case 0xF7: SET(A, 6); break;
    case 0xF8: SET(B, 7); break;
    case 0xF9: SET(C, 7); break;
    case 0xFA: SET(D, 7); break;
    case 0xFB: SET(E, 7); break;
    case 0xFC: SET(H, 7); break;
    case 0xFD: SET(L, 7); break;
    case 0xFE: SET(HLMem, 7); break;
    case 0xFF: SET(A, 7); break;
    }

    std::unreachable();
}
