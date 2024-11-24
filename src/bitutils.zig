//! Bit manipulation utilities

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
