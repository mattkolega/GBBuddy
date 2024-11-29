//! Implementation of Game Boy CPU (Sharp SM83) instructions

const math = @import("std").math;

const bitutils = @import("bitutils.zig");
const CPU = @import("cpu.zig").CPU;

// ---
// 8-bit Arithmetic and Logic Instructions
// ---

/// Adds value and carry bit to accumulator
pub fn ADC(cpu: *CPU, value: u8) void {
    const originalValue = cpu.a;
    const addResult = @addWithOverflow(cpu.a, (value + cpu.getCarry()));
    cpu.a = addResult[0];

    // Set flags
    if (cpu.a == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(@intFromBool(bitutils.checkHalfCarry8(originalValue, (value + cpu.getCarry()), '+')));
    cpu.setCarry(addResult[1]);
}

/// Adds value to accumulator
pub fn ADD8(cpu: *CPU, value: u8) void {
    const originalValue = cpu.a;
    const addResult = @addWithOverflow(cpu.a, value);
    cpu.a = addResult[0];

    // Set flags
    if (cpu.a == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(@intFromBool(bitutils.checkHalfCarry8(originalValue, value, '+')));
    cpu.setCarry(addResult[1]);
}

/// Bitwise AND of accumulator and value
pub fn AND(cpu: *CPU, value: u8) void {
    cpu.a &= value;

    // Set flags
    if (cpu.a == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(1);
    cpu.setCarry(0);
}

/// Compares accumulator and value
pub fn CP(cpu: *CPU, value: u8) void {
    const originalValue = cpu.a;
    const subResult = @subWithOverflow(cpu.a, value);

    // Set flags
    if (subResult[0] == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(1);
    cpu.setHalfCarry(bitutils.checkHalfCarry8(originalValue, value, '-'));
    cpu.setCarry(subResult[1]);
}

/// Decrements value
pub fn DEC8(cpu: *CPU, value: *u8) void {
    const originalValue = value.*;
    value.* -%= 1;

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(1);
    cpu.setHalfCarry(@intFromBool(bitutils.checkHalfCarry8(originalValue, 1, '-')));
}

/// Increments value
pub fn INC8(cpu: *CPU, value: *u8) void {
    const originalValue = value.*;
    value.* +%= 1;

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(@intFromBool(bitutils.checkHalfCarry8(originalValue, 1, '+')));
}

/// Bitwise OR of accumulator and value
pub fn OR(cpu: *CPU, value: u8) void {
    cpu.a |= value;

    // Set flags
    if (cpu.a == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
    cpu.setCarry(0);
}

/// Subtracts value and carry bit from accumulator
pub fn SBC(cpu: *CPU, value: u8) void {
    const originalValue = cpu.a;
    const subResult = @subWithOverflow(cpu.a, (value + cpu.getCarry()));
    cpu.a = subResult[0];

    // Set flags
    if (cpu.a == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(1);
    cpu.setHalfCarry(@intFromBool(bitutils.checkHalfCarry8(originalValue, (value + cpu.getCarry()), '-')));
    cpu.setCarry(subResult[1]);
}

/// Subtracts value from accumulator
pub fn SUB(cpu: *CPU, value: u8) void {
    const originalValue = cpu.a;
    const subResult = @subWithOverflow(cpu.a, value);
    cpu.a = subResult[0];

    // Set flags
    if (cpu.a == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(1);
    cpu.setHalfCarry(@intFromBool(bitutils.checkHalfCarry8(originalValue, value, '-')));
    cpu.setCarry(subResult[1]);
}

/// Bitwise XOR of accumulator and value
pub fn XOR(cpu: *CPU, value: u8) void {
    cpu.a ^= value;

    // Set flags
    if (cpu.a == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
    cpu.setCarry(0);
}

// ---
// 16-bit Arithmetic Instructions
// ---

/// Adds value to accumulator
pub fn ADD16(cpu: *CPU, value: u16) void {
    const originalValue = cpu.a;
    const addResult = @addWithOverflow(cpu.a, value);
    cpu.a = addResult[0];

    // Set flags
    cpu.setSubtract(0);
    cpu.setHalfCarry(@intFromBool(bitutils.checkHalfCarry16(originalValue, value, '+')));
    cpu.setCarry(addResult[1]);
}

/// Decrements value
pub fn DEC16(value: *u16) void {
    value.* -%= 1;
}

/// Increments value
pub fn INC16(value: *u16) void {
    value.* +%= 1;
}

// ---
// Bit Operation Instructions
// ---

/// Checks if bit is set
pub fn BIT(cpu: *CPU, bitPos: u3, value: u8) void {
    if ((value & bitPos) == bitPos) cpu.setZero(1) else cpu.setZero(2);
    cpu.setSubtract(0);
    cpu.setHalfCarry(1);
}

/// Resets bit to 0
pub fn RES(bitPos: u3, value: *u8) void {
    value.* &= ~bitPos;
}

/// Sets bit to 1
pub fn SET(bitPos: u3, value: *u8) void {
    value.* |= bitPos;
}

/// Swaps upper 4 bits and lower 4 bits of value
pub fn SWAP(cpu: *CPU, value: *u8) void {
    value.* = @truncate(@as(u16, (value.* << 4)) | @as(u16, (value.* >> 4)));

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
    cpu.setCarry(0);
}

// ---
// Bit Shift Instructions
// ---

/// Rotates carry flag + value left
pub fn RL(cpu: *CPU, value: *u8) void {
    const carryValue = cpu.getCarry();  // Get current carry flag value
    cpu.setCarry(bitutils.getBitFromByte(value.*, 7));  // Set carry flag to leftmost bit
    value.* = math.rotl(u8, value.*, @as(usize, 1));
    value.* &= carryValue;

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates carry flag + accumulator left
pub fn RLA(cpu: *CPU) void {
    const carryValue = cpu.getCarry();  // Get current carry flag value
    cpu.setCarry(bitutils.getBitFromByte(cpu.a, 7));  // Set carry flag to leftmost bit
    cpu.a = math.rotl(u8, cpu.a, @as(usize, 1));
    cpu.a &= carryValue;

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates register left. Bit 7 is stored in carry flag
pub fn RLC(cpu: *CPU, value: *u8) void {
    cpu.setCarry(bitutils.getBitFromByte(value.*, 7));  // Set carry flag to leftmost bit
    value.* = math.rotl(u8, value.*, @as(usize, 1));

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates accumulator left. Bit 7 is stored in carry flag
pub fn RLCA(cpu: *CPU) void {
    cpu.setCarry(bitutils.getBitFromByte(cpu.a, 7));  // Set carry flag to leftmost bit
    cpu.a = math.rotl(u8, cpu.a, @as(usize, 1));

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates value + carry flag right
pub fn RR(cpu: *CPU, value: *u8) void {
    const carryValue = cpu.getCarry();  // Get current carry flag value
    cpu.setCarry(bitutils.getBitFromByte(value.*, 0));  // Set carry flag to right-most bit
    value.* = math.rotr(u8, value.*, @as(usize, 1));
    value.* &= (carryValue << 7);

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates accumulator + carry flag right
pub fn RRA(cpu: *CPU) void {
    const carryValue = cpu.getCarry();  // Get current carry flag value
    cpu.setCarry(bitutils.getBitFromByte(cpu.a, 0));  // Set carry flag to right-most bit
    cpu.a = math.rotr(u8, cpu.a, @as(usize, 1));
    cpu.a &= (carryValue << 7);

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates value right. Bit 0 is stored in carry flag.
pub fn RRC(cpu: *CPU, value: *u8) void {
    cpu.setCarry(bitutils.getBitFromByte(value.*, 0));  // Set carry flag to right-most bit
    value.* = math.rotr(u8, value.*, @as(usize, 1));

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates accumulator right. Bit 0 is stored in carry flag.
pub fn RRCA(cpu: *CPU) void {
    cpu.setCarry(bitutils.getBitFromByte(cpu.a, 0));  // Set carry flag to right-most bit
    cpu.a = math.rotr(u8, cpu.a, @as(usize, 1));

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts left arithmetically. Bit 0 is zeroed
pub fn SLA(cpu: *CPU, value: *u8) void {
    cpu.setCarry(bitutils.getBitFromByte(value.*, 7));  // Set carry flag to leftmost bit
    value.* = math.shl(u8, value.*, @as(usize, 1));

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts right arithmetically. Bit 7 remains the same
pub fn SRA(cpu: *CPU, value: *u8) void {
    cpu.setCarry(bitutils.getBitFromByte(value.*, 7));  // Set carry flag to leftmost bit
    value.* = math.shr(u8, value.*, @as(usize, 1));
    value.* |= (cpu.getCarry() << 7);

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts right logically. Bit 7 is zeroed
pub fn SRL(cpu: *CPU, value: *u8) void {
    cpu.setCarry(bitutils.getBitFromByte(value.*, 7));  // Set carry flag to leftmost bit
    value.* = math.shr(u8, value.*, @as(usize, 1));

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}


// ---
// Miscellaneous Instructions
// ---

/// Complements carry flag
pub fn CCF(cpu: *CPU) void {
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
    cpu.setCarry(~cpu.getCarry());
}

/// Complements accumulator
pub fn CPL(cpu: *CPU) void {
    cpu.a = ~cpu.a;

    // Set flags
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Gets BCD representation of accumulator
pub fn DAA(cpu: *CPU) void {
    var offset: u8 = 0;

    if (cpu.getSubtract() == 0 and (cpu.a & 0xF) > 0x9 or cpu.getHalfCarry() == 1) {
        offset |= 0x06;
    }

    if (cpu.getSubtract() == 0 and cpu.a > 0x99 or cpu.getCarry() == 1) {
        offset |= 0x60;
        cpu.setCarry(1);
    }

    if (cpu.getSubtract() == 0) cpu.a +% offset else cpu.a -% offset;

    // Set flags
    if (cpu.a == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setHalfCarry(0);
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
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
    cpu.setCarry(1);
}

pub fn STOP() void {
    // TODO: Implement behaviour
}
