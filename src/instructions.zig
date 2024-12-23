//! Implementation of Game Boy CPU (Sharp SM83) instructions

const math = @import("std").math;
const stringToEnum = @import("std").meta.stringToEnum;

const bitutils = @import("bitutils.zig");
const CPU = @import("cpu.zig").CPU;
const log = @import("logger.zig");

const Register = enum { AF, BC, DE, HL, SP };

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
    cpu.setHalfCarry(bitutils.checkHalfCarry8(originalValue, (value + cpu.getCarry()), '+'));
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
    cpu.setHalfCarry(bitutils.checkHalfCarry8(originalValue, value, '+'));
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
    cpu.setHalfCarry(bitutils.checkHalfCarry8(originalValue, 1, '-'));
}

/// Increments value
pub fn INC8(cpu: *CPU, value: *u8) void {
    const originalValue = value.*;
    value.* +%= 1;

    // Set flags
    if (value.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(bitutils.checkHalfCarry8(originalValue, 1, '+'));
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
    cpu.setHalfCarry(bitutils.checkHalfCarry8(originalValue, (value + cpu.getCarry()), '-'));
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
    cpu.setHalfCarry(bitutils.checkHalfCarry8(originalValue, value, '-'));
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

/// Adds value to HL
pub fn ADD16(cpu: *CPU, value: u16) void {
    const originalValue = cpu.getHL();
    const addResult = @addWithOverflow(originalValue, value);
    cpu.setHL(addResult[0]);

    // Set flags
    cpu.setSubtract(0);
    cpu.setHalfCarry(bitutils.checkHalfCarry16(originalValue, value, '+'));
    cpu.setCarry(addResult[1]);
}

/// Decrements value in 16-bit register
pub fn DEC16(cpu: *CPU, comptime register: []const u8) void {
    const case = stringToEnum(Register, register);
    const reg = case orelse @panic("Invalid register given for DEC16 operation. Must be AF, BC, DE, HL or SP");
    switch (reg) {
        Register.AF => {
            cpu.setAF(cpu.getAF()-%1);
        },
        Register.BC => {
            cpu.setBC(cpu.getBC()-%1);
        },
        Register.DE => {
            cpu.setDE(cpu.getDE()-%1);
        },
        Register.HL => {
            cpu.setHL(cpu.getHL()-%1);
        },
        Register.SP => {
            cpu.sp -%= 1;
        }
    }
}

/// Increments value in 16-bit register
pub fn INC16(cpu: *CPU, comptime register: []const u8) void {
    const case = stringToEnum(Register, register);
    const reg = case orelse @panic("Invalid register given for INC16 operation. Must be AF, BC, DE, HL or SP");
    switch (reg) {
        Register.AF => {
            cpu.setAF(cpu.getAF()+%1);
        },
        Register.BC => {
            cpu.setBC(cpu.getBC()+%1);
        },
        Register.DE => {
            cpu.setDE(cpu.getDE()+%1);
        },
        Register.HL => {
            cpu.setHL(cpu.getHL()+%1);
        },
        Register.SP => {
            cpu.sp +%= 1;
        }
    }
}

// ---
// Bit Operation Instructions
// ---

/// Checks if bit is set
pub fn BIT(cpu: *CPU, bitPos: u3, value: u8) void {
    if ((value & bitPos) == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(1);
}

/// Resets bit in register to 0
pub fn RES_r8(bitPos: u3, reg: *u8) void {
    reg.* &= ~bitPos;
}

/// Resets bit in value at address HL to 0
pub fn RES_HL(cpu: *CPU, bitPos: u3) void {
    const newValue = cpu.memoryRead(cpu.getHL()) & ~bitPos;
    cpu.memoryWrite(cpu.getHL(), newValue);
}

/// Sets bit in register to 1
pub fn SET_r8(bitPos: u3, reg: *u8) void {
    reg.* |= bitPos;
}

/// Sets bit in value at address HL to 1
pub fn SET_HL(cpu: *CPU, bitPos: u3) void {
    const newValue = cpu.memoryRead(cpu.getHL()) | bitPos;
    cpu.memoryWrite(cpu.getHL(), newValue);
}

/// Swaps upper 4 bits and lower 4 bits of register
pub fn SWAP_r8(cpu: *CPU, reg: *u8) void {
    reg.* = bitutils.swapNibbles(reg.*);

    // Set flags
    if (reg.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
    cpu.setCarry(0);
}

/// Swaps upper 4 bits and lower 4 bits of value at address HL
pub fn SWAP_HL(cpu: *CPU) void {
    const newValue = bitutils.swapNibbles(cpu.memoryRead(cpu.getHL()));
    cpu.memoryWrite(cpu.getHL(), newValue);

    // Set flags
    if (newValue == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
    cpu.setCarry(0);
}

// ---
// Bit Shift Instructions
// ---

/// Rotates carry flag + value left
fn RL(cpu: *CPU, value: u8) u8 {
    const carryValue = cpu.getCarry();  // Get current carry flag value
    cpu.setCarry(bitutils.getBitFromByte(value, 7));  // Set carry flag to leftmost bit
    const newValue = math.rotl(u8, value, @as(usize, 1));
    return newValue & carryValue;
}

/// Rotates carry flag + register left
pub fn RL_r8(cpu: *CPU, reg: *u8) void {
    reg.* = RL(cpu, reg.*);

    // Set flags
    if (reg.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates carry flag + value at address HL left
pub fn RL_HL(cpu: *CPU) void {
    const newValue = RL(cpu, cpu.memoryRead(cpu.getHL()));
    cpu.memoryWrite(cpu.getHL(), newValue);

    // Set flags
    if (newValue == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates carry flag + accumulator left
pub fn RLA(cpu: *CPU) void {
    cpu.a = RL(cpu, cpu.a);

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates value left. Bit 7 is stored in carry flag
fn RLC(cpu: *CPU, value: u8) u8 {
    cpu.setCarry(bitutils.getBitFromByte(value, 7));  // Set carry flag to leftmost bit
    return math.rotl(u8, value, @as(usize, 1));
}

/// Rotates register left. Bit 7 is stored in carry flag
pub fn RLC_r8(cpu: *CPU, reg: *u8) void {
    reg.* = RLC(cpu, reg.*);

    // Set flags
    if (reg.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates value at address HL left. Bit 7 is stored in carry flag
pub fn RLC_HL(cpu: *CPU) void {
    const newValue = RLC(cpu, cpu.memoryRead(cpu.getHL()));
    cpu.memoryWrite(cpu.getHL(), newValue);

    // Set flags
    if (newValue == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates accumulator left. Bit 7 is stored in carry flag
pub fn RLCA(cpu: *CPU) void {
    cpu.a = RLC(cpu, cpu.a);

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates value + carry flag right
fn RR(cpu: *CPU, value: u8) u8 {
    const carryValue = cpu.getCarry();  // Get current carry flag value
    cpu.setCarry(bitutils.getBitFromByte(value, 0));  // Set carry flag to right-most bit
    const newValue = math.rotr(u8, value, @as(usize, 1));
    return newValue & (@as(u8, carryValue) << 7);
}

/// Rotates register + carry flag right
pub fn RR_r8(cpu: *CPU, reg: *u8) void {
    reg.* = RR(cpu, reg.*);

    // Set flags
    if (reg.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates value at address HL + carry flag right
pub fn RR_HL(cpu: *CPU) void {
    const newValue = RR(cpu, cpu.memoryRead(cpu.getHL()));
    cpu.memoryWrite(cpu.getHL(), newValue);

    // Set flags
    if (newValue == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates accumulator + carry flag right
pub fn RRA(cpu: *CPU) void {
    cpu.a = RR(cpu, cpu.a);

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates value right. Bit 0 is stored in carry flag.
fn RRC(cpu: *CPU, value: u8) u8 {
    cpu.setCarry(bitutils.getBitFromByte(value, 0));  // Set carry flag to right-most bit
    return math.rotr(u8, value, @as(usize, 1));
}

/// Rotates register right. Bit 0 is stored in carry flag.
pub fn RRC_r8(cpu: *CPU, reg: *u8) void {
    reg.* = RRC(cpu, reg.*);

    // Set flags
    if (reg.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates value at address HL right. Bit 0 is stored in carry flag.
pub fn RRC_HL(cpu: *CPU) void {
    const newValue = RRC(cpu, cpu.memoryRead(cpu.getHL()));
    cpu.memoryWrite(cpu.getHL(), newValue);

    // Set flags
    if (newValue == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Rotates accumulator right. Bit 0 is stored in carry flag.
pub fn RRCA(cpu: *CPU) void {
    cpu.a = RRC(cpu, cpu.a);

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts left arithmetically. Bit 0 is zeroed
fn SLA(cpu: *CPU, value: u8) u8 {
    cpu.setCarry(bitutils.getBitFromByte(value, 7));  // Set carry flag to leftmost bit
    return math.shl(u8, value, @as(usize, 1));
}

/// Shifts register left arithmetically. Bit 0 is zeroed
pub fn SLA_r8(cpu: *CPU, reg: *u8) void {
    reg.* = SLA(cpu, reg.*);

    // Set flags
    if (reg.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts value at address HL left arithmetically. Bit 0 is zeroed
pub fn SLA_HL(cpu: *CPU) void {
    const newValue = SLA(cpu, cpu.memoryRead(cpu.getHL()));
    cpu.memoryWrite(cpu.getHL(), newValue);

    // Set flags
    if (newValue == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts right arithmetically. Bit 7 remains the same
fn SRA(cpu: *CPU, value: u8) u8 {
    cpu.setCarry(bitutils.getBitFromByte(value, 7));  // Set carry flag to leftmost bit
    const newValue = math.shr(u8, value, @as(usize, 1));
    return newValue | (@as(u8, cpu.getCarry()) << 7);
}

/// Shifts register right arithmetically. Bit 7 remains the same
pub fn SRA_r8(cpu: *CPU, reg: *u8) void {
    reg.* = SRA(cpu, reg.*);

    // Set flags
    if (reg.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts value at address HL right arithmetically. Bit 7 remains the same
pub fn SRA_HL(cpu: *CPU) void {
    const newValue = SRA(cpu, cpu.memoryRead(cpu.getHL()));
    cpu.memoryWrite(cpu.getHL(), newValue);

    // Set flags
    if (newValue == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts right logically. Bit 7 is zeroed
fn SRL(cpu: *CPU, value: u8) u8 {
    cpu.setCarry(bitutils.getBitFromByte(value, 7));  // Set carry flag to leftmost bit
    return math.shr(u8, value, @as(usize, 1));
}

/// Shifts register right logically. Bit 7 is zeroed
pub fn SRL_r8(cpu: *CPU, reg: *u8) void {
    reg.* = SRL(cpu, reg.*);

    // Set flags
    if (reg.* == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

/// Shifts value at address HL right logically. Bit 7 is zeroed
pub fn SRL_HL(cpu: *CPU) void {
    const newValue = SRA(cpu, cpu.memoryRead(cpu.getHL()));
    cpu.memoryWrite(cpu.getHL(), newValue);

    // Set flags
    if (newValue == 0) cpu.setZero(1) else cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(0);
}

// ---
// Load Instructions
// ---

/// Loads 8-bit value into 8-bit register
pub fn LD_r8(reg: *u8, value: u8) void {
    reg.* = value;
}

/// Loads 16-bit value into 16-bit register
pub fn LD_r16(cpu: *CPU, value: u16, comptime register: []const u8) void {
    const case = stringToEnum(Register, register);
    const reg = case orelse @panic("Invalid register given for LD operation. Must be AF, BC, DE or HL");
    switch (reg) {
        Register.AF => {
            cpu.setAF(value);
        },
        Register.BC => {
            cpu.setBC(value);
        },
        Register.DE => {
            cpu.setDE(value);
        },
        Register.HL => {
            cpu.setHL(value);
        },
        else => {
            @panic("Unsupported register for LD operation. Must be AF, BC, DE or HL");
        }
    }
}

// Loads 8-bit value into byte pointed to by HL
pub fn LD_HL(cpu: *CPU, value: u8) void {
    cpu.memoryWrite(cpu.getHL(), value);
}

/// Gets value from accumulator and writes to memory with address from a register
pub fn LD_r16_A(cpu: *CPU, address: u16) void {
    cpu.memoryWrite(address, cpu.a);
}

// Gets value from accumulator and writes to memory
pub fn LD_n16_A(cpu: *CPU, address: u16) void {
    cpu.memoryWrite(address, cpu.a);
}

/// Gets value from memory with address from a register and writes to accumulator
pub fn LD_A_r16(cpu: *CPU, address: u16) void {
    cpu.a = cpu.memoryRead(address);
}

/// Gets value from memory and writes to accumulator
pub fn LD_A_n16(cpu: *CPU, address: u16) void {
    cpu.a = cpu.memoryRead(address);
}

/// Gets value from accumulator and writes to memory with address from a register.
/// Address must be between $FF00 and $FFFF
pub fn LDH_n16_A(cpu: *CPU, lowByte: u8) void {
    cpu.memoryWrite(@as(u16, 0xFF00) & lowByte, cpu.a);
}

/// Gets value from accumulator and writes to memory with address $FF00 + offset
pub fn LDH_C_A(cpu: *CPU) void {
    cpu.memoryWrite(@as(u16, 0xFF00) & cpu.c, cpu.a);
}

/// Gets value from memory at an address between $FF00 and $FFFF and writes to accumulator
pub fn LDH_A_n16(cpu: *CPU, lowByte: u8) void {
    cpu.a = cpu.memoryRead(@as(u16, 0xFF00) & lowByte);
}

/// Gets value from memory at address $FF00 + offset and writes to accumulator
pub fn LDH_A_C(cpu: *CPU) void {
    cpu.a = cpu.memoryRead(@as(u16, 0xFF00) & cpu.c);
}

/// Gets value from memory at address stored in HL and writes to accumulator. Increments HL afterwards
pub fn LD_HLI_A(cpu: *CPU) void {
    const HL = cpu.getHL();

    cpu.a = cpu.memoryRead(HL);
    cpu.setHL(HL+1);
}

/// Gets value from memory at address stored in HL and writes to accumulator. Decrements HL afterwards
pub fn LD_HLD_A(cpu: *CPU) void {
    const HL = cpu.getHL();

    cpu.a = cpu.memoryRead(HL);
    cpu.setHL(HL-1);
}

/// Gets value from accumulator and writes to memory at address HL. Increments HL afterwards
pub fn LD_A_HLI(cpu: *CPU) void {
    const HL = cpu.getHL();

    cpu.memoryWrite(HL, cpu.a);
    cpu.setHL(HL+1);
}

/// Gets value from accumulator and writes to memory at address HL. Decrements HL afterwards
pub fn LD_A_HLD(cpu: *CPU) void {
    const HL = cpu.getHL();

    cpu.memoryWrite(HL, cpu.a);
    cpu.setHL(HL-1);
}

// ---
// Jumps and Subroutines
// ---

/// Calls address
pub fn CALL(cpu: *CPU, address: u16) void {
    cpu.pushToStack16(cpu.pc);
    cpu.pc = address;
}

/// Jumps to an address
pub fn JP(cpu: *CPU, address: u16) void {
    cpu.pc = address;
}

/// Performs a relative jump to an address
pub fn JR(cpu: *CPU, offset: i8) void {
    cpu.pc +%= @intCast(offset);
}

/// Returns from subroutine
pub fn RET(cpu: *CPU) void {
    cpu.pc = cpu.popStack16();
}

/// Returns from subroutine and enables interrupts
pub fn RETI(cpu: *CPU) void {
    cpu.pc = cpu.popStack16();
    cpu.ime = 1;
}

/// Calls address vec
pub fn RST(cpu: *CPU, vec: u8) void {
    cpu.pushToStack16(cpu.pc);
    cpu.pc = vec;
}

// ---
// Stack Operation Instructions
// ---

/// Adds SP to HL
pub fn ADD_HL_SP(cpu: *CPU) void {
    const originalValue = cpu.getHL();
    const result = @addWithOverflow(originalValue, cpu.sp);
    cpu.setHL(result[0]);

    // Set flags
    cpu.setSubtract(0);
    cpu.setHalfCarry(bitutils.checkHalfCarry16(originalValue, cpu.sp, '+'));
    cpu.setCarry(result[1]);
}

/// Adds signed 8-bit value to SP
pub fn ADD_SP_e8(cpu: *CPU, value: i8) void {
    const originalValue = cpu.sp;
    const result = @addWithOverflow(cpu.sp, @as(u8, @bitCast(value)));
    cpu.sp = result[0];

    // Set flags
    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(bitutils.checkHalfCarry16(originalValue, @as(u8, @bitCast(value)), '+'));
    cpu.setCarry(result[1]);
}

/// Loads stack pointer with 16-bit value
pub fn LD_SP_n16(cpu: *CPU, value: u16) void {
    cpu.sp = value;
}

/// Writes SP & $FF to address and SP >> 8 to address+1
pub fn LD_n16_SP(cpu: *CPU, address: u16) void {
    cpu.memoryWrite(address, @truncate(cpu.sp));
    cpu.memoryWrite(address + 1, @truncate(cpu.sp >> 8));
}

/// Adds signed 8-bit value to sp and stores result in HL
pub fn LD_HL_SP(cpu: *CPU, value: i8) void {
    const originalValue = cpu.sp;
    const result = @addWithOverflow(originalValue, @as(u8, @bitCast(value)));
    cpu.setHL(result[0]);

    cpu.setZero(0);
    cpu.setSubtract(0);
    cpu.setHalfCarry(bitutils.checkHalfCarry16(cpu.sp, @as(u8, @bitCast(value)), '+'));
    cpu.setCarry(result[1]);
}

/// Loads HL into SP
pub fn LD_SP_HL(cpu: *CPU) void {
    cpu.sp = cpu.getHL();
}

/// Reads 16-bit register from stack
pub fn POP(cpu: *CPU, comptime register: []const u8) void {
    const case = stringToEnum(Register, register);
    const reg = case orelse @panic("Invalid register given for POP operation. Must be AF, BC, DE or HL");
    switch (reg) {
        Register.AF => {
            cpu.setAF(cpu.popStack16());
        },
        Register.BC => {
            cpu.setBC(cpu.popStack16());
        },
        Register.DE => {
            cpu.setDE(cpu.popStack16());
        },
        Register.HL => {
            cpu.setHL(cpu.popStack16());
        },
        else => {
            @panic("Unsupported register for POP operation. Must be AF, BC, DE or HL");
        }
    }
}

/// Writes 16-bit register to stack
pub fn PUSH(cpu: *CPU, comptime register: []const u8) void {
    const case = stringToEnum(Register, register);
    const reg = case orelse @panic("Invalid register given for PUSH operation. Must be AF, BC, DE or HL");
    switch (reg) {
        Register.AF => {
            cpu.pushToStack16(cpu.getAF());
        },
        Register.BC => {
            cpu.pushToStack16(cpu.getBC());
        },
        Register.DE => {
            cpu.pushToStack16(cpu.getDE());
        },
        Register.HL => {
            cpu.pushToStack16(cpu.getHL());
        },
        else => {
            @panic("Unsupported register for PUSH operation. Must be AF, BC, DE or HL");
        }
    }
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

    if (cpu.getSubtract() == 0) cpu.a +%= offset else cpu.a -%= offset;

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
