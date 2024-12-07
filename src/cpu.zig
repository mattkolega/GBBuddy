//! Implementation of Game Boy CPU (Sharp SM83)

const bitutils = @import("bitutils.zig");
const GameBoy = @import("GameBoy.zig").GameBoy;

pub const CPU = packed struct {
    // Registers
    a: u8,
    b: u8,
    c: u8,
    d: u8,
    e: u8,
    f: u8,
    h: u8,
    l: u8,
    sp: u16,  // Stack pointer
    pc: u16,  // Program counter

    ime: u1,  // Interrupt master enable flag

    gb: *GameBoy,

    fn init(self: *CPU, gb: *GameBoy) void {
        self.gb = gb;
    }

    // Wrappers for MMU memory access
    fn memoryRead(self: *CPU, address: u16) void {
        self.gb.mmu.memoryRead(address);
    }
    fn memoryWrite(self: *CPU, address: u16, value: u8) void {
        self.gb.mmu.memoryWrite(address, value);
    }

    // 16-bit register helpers
    fn getAF(self: *CPU) u16 {
        return self.a << 8 | self.b;
    }
    fn setAF(self: *CPU, value: u16) void {
        self.a = @truncate(value >> 8);
        self.f = @truncate(value);
    }
    fn getBC(self: *CPU) u16 {
        return self.b << 8 | self.c;
    }
    fn setBC(self: *CPU, value: u16) void {
        self.b = @truncate(value >> 8);
        self.c = @truncate(value);
    }
    fn getDE(self: *CPU) u16 {
        return self.d << 8 | self.e;
    }
    fn setDE(self: *CPU, value: u16) void {
        self.d = @truncate(value >> 8);
        self.e = @truncate(value);
    }
    fn getHL(self: *CPU) u16 {
        return self.h << 8 | self.l;
    }
    fn setHL(self: *CPU, value: u16) void {
        self.h = @truncate(value >> 8);
        self.l = @truncate(value);
    }

    // Flag helpers
    fn getZero(self: *CPU) u1 {
        return bitutils.getBitFromByte(self.f, 7);
    }
    fn setZero(self: *CPU, value: u1) void {
        self.f = bitutils.setBitInByte(self.f, 7, value);
    }
    fn getSubtract(self: *CPU) u1 {
        return bitutils.getBitFromByte(self.f, 6);
    }
    fn setSubtract(self: *CPU, value: u1) void {
        self.f = bitutils.setBitInByte(self.f, 6, value);
    }
    fn getHalfCarry(self: *CPU) u1 {
        return bitutils.getBitFromByte(self.f, 5);
    }
    fn setHalfCarry(self: *CPU, value: u1) void {
        self.f = bitutils.setBitInByte(self.f, 5, value);
    }
    fn getCarry(self: *CPU) u1 {
        return bitutils.getBitFromByte(self.f, 4);
    }
    fn setCarry(self: *CPU, value: u1) void {
        self.f = bitutils.setBitInByte(self.f, 4, value);
    }

    // Stack helpers
    fn pushToStack16(self: *CPU, value: u16) void {
        self.sp -%= 1;
        self.memoryWrite(self.sp, @truncate(value >> 8));
        self.sp -%= 1;
        self.memoryWrite(self.sp, @truncate(value));
    }
    fn popStack16(self: *CPU) u16 {
        const lo = self.memoryRead(self.sp);
        self.sp +%= 1;
        const hi = self.memoryRead(self.sp);
        self.sp +%= 1;
        return (hi << 8) | lo;
    }
};
