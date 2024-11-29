//! Bit manipulation utility functions

const log = @import("logger.zig");

/// Check if half carry occurred in 8-bit arithmetic
pub fn checkHalfCarry8(operand1: u8, operand2: u8, comptime operation: u8) bool {
    if (operation == '+') {
        return (((operand1 & 0xF) +% (operand2 & 0xF)) & 0x10) == 0x10;
    } else if (operation == '-'){
        return (((operand1 & 0xF) -% (operand2 & 0xF)) & 0x10) == 0x10;
    } else {
        @panic("Unsupported operation type in checkHalfCarry8");
    }
}

/// Check if half carry occurred in 16-bit arithmetic
pub fn checkHalfCarry16(operand1: u16, operand2: u16, comptime operation: u8) bool {
    if (operation == '+') {
        return (((operand1 & 0xFFF) +% (operand2 & 0xFFF)) & 0x1000) == 0x1000;
    } else if (operation == '-'){
        return (((operand1 & 0xFFF) -% (operand2 & 0xFFF)) & 0x1000) == 0x1000;
    } else {
        @panic("Unsupported operation type in checkHalfCarry16");
    }
}

/// Gets nth bit from an 8-bit value
pub fn getBitFromByte(value: u8, comptime n: u3) u1 {
    return @truncate(value >> n);
}

/// Sets a specific bit to 1 or 0 in a given byte
pub fn setBitInByte(value: u8, comptime n: u3, bit: u1) u8 {
    // Clear the bit then bitwise OR byte with new bit
    return (value & ~(@as(u8, 1) << n)) | (@as(u8, bit) << n);
}

// ---
// Tests
// ---

const expectEqual = @import("std").testing.expectEqual;

test "checkHalfCarry8" {
    try expectEqual(true, checkHalfCarry8(0xF, 0xF, '+'));
    try expectEqual(false, checkHalfCarry8(0x1, 0x1, '+'));
    try expectEqual(true, checkHalfCarry8(0x1, 0xF, '-'));
    try expectEqual(false, checkHalfCarry8(0xF, 0x1, '-'));
}

test "checkHalfCarry16" {
    try expectEqual(true, checkHalfCarry16(0xFFF, 0xFFF, '+'));
    try expectEqual(false, checkHalfCarry16(0x100, 0x100, '+'));
    try expectEqual(true, checkHalfCarry16(0x100, 0xF00, '-'));
    try expectEqual(false, checkHalfCarry16(0xF00, 0x100, '-'));
}

test "getBitFromByte" {
    try expectEqual(0b1, getBitFromByte(0b1011_1001, 7));
    try expectEqual(0b0, getBitFromByte(0b1011_1001, 6));
    try expectEqual(0b1, getBitFromByte(0b1011_1001, 5));
    try expectEqual(0b1, getBitFromByte(0b1011_1001, 4));
    try expectEqual(0b1, getBitFromByte(0b1011_1001, 3));
    try expectEqual(0b0, getBitFromByte(0b1011_1001, 2));
    try expectEqual(0b0, getBitFromByte(0b1011_1001, 1));
    try expectEqual(0b1, getBitFromByte(0b1011_1001, 0));
}

test "setBitInByte" {
    try expectEqual(0b1010_1011, setBitInByte(0b1010_1011, 7, 1));
    try expectEqual(0b0010_1011, setBitInByte(0b1010_1011, 7, 0));
    try expectEqual(0b1010_1111, setBitInByte(0b1010_1011, 2, 1));
    try expectEqual(0b1010_1011, setBitInByte(0b1010_1011, 4, 0));
}
