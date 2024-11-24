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
pub fn DEC(cpu: *CPU, value: *u8) void {
    const originalValue = value;
    value -%= 1;

    if (value == 0) cpu.zero = 1 else cpu.zero = 0;
    cpu.subtract = 1;
    cpu.halfCarry = @intFromBool(bitutils.checkHalfCarry8(originalValue, 1, '-'));
}

/// Increments value
pub fn INC(cpu: *CPU, value: *u8) void {
    const originalValue = value;
    value -%= 1;

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
pub fn SUB8(cpu: *CPU, value: u8) void {
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
