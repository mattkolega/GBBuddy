//! Implementation of Game Boy CPU (Sharp SM83) instructions

const bitutils = @import("bitutils.zig");
const CPU = @import("cpu.zig").CPU;

// ---
// 8-bit Arithmetic and Logic Instructions
// ---

/// Adds value and carry bit to accumulator
pub fn ADC(cpu: *CPU, value: u8) void {
    const originalValue = cpu.A;
    const addResult = @addWithOverflow(cpu.A, (value + cpu.carry));
    cpu.A = addResult[0];

    // Set flags
    if (cpu.A == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 0;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry8(originalValue, (value + cpu.carry), '+'));
    cpu.carry = addResult[1];
}

/// Adds value to accumulator
pub fn ADD8(cpu: *CPU, value: u8) void {
    const originalValue = cpu.A;
    const addResult = @addWithOverflow(cpu.A, value);
    cpu.A = addResult[0];

    // Set flags
    if (cpu.A == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 0;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry8(originalValue, value, '+'));
    cpu.carry = addResult[1];
}

/// Bitwise AND of accumulator and value
pub fn AND(cpu: *CPU, value: u8) void {
    cpu.A &= value;

    // Set flags
    if (cpu.A == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 0;
    cpu.halfCarry = 1;
    cpu.carry = 0;
}

/// Compares accumulator and value
pub fn CP(cpu: *CPU, value: u8) void {
    const originalValue = cpu.A;
    const subResult = @subWithOverflow(cpu.A, value);

    // Set flags
    if (subResult[0] == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 1;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry8(originalValue, value, '-'));
    cpu.carry = subResult[1];
}

/// Decrements value
pub fn DEC8(cpu: *CPU, value: *u8) void {
    const originalValue = value;
    value -%= 1;

    if (value == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 1;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry8(originalValue, 1, '-'));
}

/// Increments value
pub fn INC8(cpu: *CPU, value: *u8) void {
    const originalValue = value;
    value +%= 1;

    if (value == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 0;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry8(originalValue, 1, '+'));
}

/// Bitwise OR of accumulator and value
pub fn OR(cpu: *CPU, value: u8) void {
    cpu.A |= value;

    // Set flags
    if (cpu.A == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 0;
    cpu.halfCarry = 0;
    cpu.carry = 0;
}

/// Subtracts value and carry bit from accumulator
pub fn SBC(cpu: *CPU, value: u8) void {
    const originalValue = cpu.A;
    const subResult = @subWithOverflow(cpu.A, (value + cpu.carry));
    cpu.A = subResult[0];

    // Set flags
    if (cpu.A == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 1;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry8(originalValue, (value + cpu.carry), '-'));
    cpu.carry = subResult[1];
}

/// Subtracts value from accumulator
pub fn SUB(cpu: *CPU, value: u8) void {
    const originalValue = cpu.A;
    const subResult = @subWithOverflow(cpu.A, value);
    cpu.A = subResult[0];

    // Set flags
    if (cpu.A == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 1;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry8(originalValue, value, '-'));
    cpu.carry = subResult[1];
}

/// Bitwise XOR of accumulator and value
pub fn XOR(cpu: *CPU, value: u8) void {
    cpu.A ^= value;

    // Set flags
    if (cpu.A == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 0;
    cpu.halfCarry = 0;
    cpu.carry = 0;
}

// ---
// 16-bit Arithmetic Instructions
// ---

pub fn ADD16(cpu: *CPU, value: u16) void {
    const originalValue = cpu.A;
    const addResult = @addWithOverflow(cpu.A, value);
    cpu.A = addResult[0];

    // Set flags
    cpu.subtract = 0;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry16(originalValue, value, '+'));
    cpu.carry = addResult[1];
}

pub fn DEC16(value: *u16) void {
    value -%= 1;
}

pub fn INC16(value: *u16) void {
    value +%= 1;
}

// ---
// Miscellaneous Instructions
// ---

/// Complements carry flag
pub fn CCF(cpu: *CPU) void {
    cpu.subtract = 0;
    cpu.halfCarry = 0;
    cpu.carry = ~cpu.carry;
}

/// Complements accumulator
pub fn CPL(cpu: *CPU) void {
    cpu.A = ~cpu.A;

    // Set flags
    cpu.subtract = 0;
    cpu.halfCarry = 0;
}

/// Gets BCD representation of accumulator
pub fn DAA(cpu: *CPU) void {
    var offset: u8 = 0;

    if (cpu.subtract == 0 and (cpu.a & 0xF) > 0x9 or cpu.halfCarry == 1) {
        offset |= 0x06;
    }

    if (cpu.subtract == 0 and cpu.a > 0x99 or cpu.carry == 1) {
        offset |= 0x60;
        cpu.carry = 1;
    }

    if (cpu.subtract == 0) cpu.a +% offset else cpu.a -% offset;

    // Set flags
    if (cpu.A == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.halfCarry = 0;
}

/// Disables interrupts
pub fn DI(cpu: *CPU) void {
    cpu.ime = 0;
}

/// Enables interrupts
pub fn EI() void {
    // TODO: Add delayed setting of IME flag
}

/// Enters CPU low-power mode
pub fn HALT() void {
    // TODO: Implement behaviour
}

/// Performs no operation
pub fn NOP() void {
    return;
}

/// Sets carry flag
pub fn SCF(cpu: *CPU) void {
    cpu.subtract = 0;
    cpu.halfCarry = 0;
    cpu.carry = 1;
}

pub fn STOP() void {
    // TODO: Implement behaviour
}
