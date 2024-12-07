//! Implementation of Game Boy CPU (Sharp SM83)

const bitutils = @import("bitutils.zig");
const GameBoy = @import("gameboy.zig").GameBoy;
const instructions = @import("instructions.zig");

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

    pub fn init(self: *CPU, gb: *GameBoy) void {
        self.gb = gb;
    }

    // Wrappers for MMU memory access
    pub fn memoryRead(self: *CPU, address: u16) u8 {
        return self.gb.mmu.memoryRead(address);
    }
    pub fn memoryWrite(self: *CPU, address: u16, value: u8) void {
        self.gb.mmu.memoryWrite(address, value);
    }

    // 16-bit register helpers
    pub fn getAF(self: *CPU) u16 {
        return bitutils.concatBytes(self.b, self.a);
    }
    pub fn setAF(self: *CPU, value: u16) void {
        self.a = @truncate(value >> 8);
        self.f = @truncate(value);
    }
    pub fn getBC(self: *CPU) u16 {
        return bitutils.concatBytes(self.c, self.b);
    }
    pub fn setBC(self: *CPU, value: u16) void {
        self.b = @truncate(value >> 8);
        self.c = @truncate(value);
    }
    pub fn getDE(self: *CPU) u16 {
        return bitutils.concatBytes(self.e, self.d);
    }
    pub fn setDE(self: *CPU, value: u16) void {
        self.d = @truncate(value >> 8);
        self.e = @truncate(value);
    }
    pub fn getHL(self: *CPU) u16 {
        return bitutils.concatBytes(self.l, self.h);
    }
    pub fn setHL(self: *CPU, value: u16) void {
        self.h = @truncate(value >> 8);
        self.l = @truncate(value);
    }

    // Flag helpers
    pub fn getZero(self: *CPU) u1 {
        return bitutils.getBitFromByte(self.f, 7);
    }
    pub fn setZero(self: *CPU, value: u1) void {
        self.f = bitutils.setBitInByte(self.f, 7, value);
    }
    pub fn getSubtract(self: *CPU) u1 {
        return bitutils.getBitFromByte(self.f, 6);
    }
    pub fn setSubtract(self: *CPU, value: u1) void {
        self.f = bitutils.setBitInByte(self.f, 6, value);
    }
    pub fn getHalfCarry(self: *CPU) u1 {
        return bitutils.getBitFromByte(self.f, 5);
    }
    pub fn setHalfCarry(self: *CPU, value: u1) void {
        self.f = bitutils.setBitInByte(self.f, 5, value);
    }
    pub fn getCarry(self: *CPU) u1 {
        return bitutils.getBitFromByte(self.f, 4);
    }
    pub fn setCarry(self: *CPU, value: u1) void {
        self.f = bitutils.setBitInByte(self.f, 4, value);
    }

    // Stack helpers
    pub fn pushToStack16(self: *CPU, value: u16) void {
        self.sp -%= 1;
        self.memoryWrite(self.sp, @truncate(value >> 8));
        self.sp -%= 1;
        self.memoryWrite(self.sp, @truncate(value));
    }
    pub fn popStack16(self: *CPU) u16 {
        const lo = self.memoryRead(self.sp);
        self.sp +%= 1;
        const hi = self.memoryRead(self.sp);
        self.sp +%= 1;
        return (hi << 8) | lo;
    }
};

/// Executes a single opcode. Returns number of cycles
pub fn fetchAndExecute(cpu: *CPU) usize {
    const opcode = cpu.memoryRead(cpu.pc);
    cpu.pc +%= 1;

    switch (bitutils.getFirstNibble(opcode)) {
        0x0 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.NOP();
                    return 1;
                },
                0x1 => {
                    const value = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+1));
                    cpu.pc +%= 2;
                    instructions.LD_r16(cpu, value, "BC");
                    return 3;
                },
                0x2 => {
                    instructions.LD_n16_A(cpu, cpu.getBC());
                    return 2;
                },
                0x3 => {
                    instructions.INC16(cpu, "BC");
                    return 2;
                },
                0x4 => {
                    instructions.INC8(cpu, &cpu.b);
                    return 1;
                },
                0x5 => {
                    instructions.DEC8(cpu, &cpu.b);
                    return 1;
                },
                0x6 => {
                    instructions.LD_r8(&cpu.b, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0x7 => {
                    instructions.RLCA(cpu);
                    return 1;
                },
                0x8 => {
                    const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+1));
                    cpu.pc +%= 2;
                    instructions.LD_n16_SP(cpu, address);
                    return 5;
                },
                0x9 => {
                    instructions.ADD16(cpu, cpu.getBC());
                    return 2;
                },
                0xA => {
                    instructions.LD_r8(&cpu.a, cpu.memoryRead(cpu.getBC()));
                    return 2;
                },
                0xB => {
                    instructions.DEC16(cpu, "BC");
                    return 2;
                },
                0xC => {
                    instructions.INC8(cpu, &cpu.c);
                    return 1;
                },
                0xD => {
                    instructions.DEC8(cpu, &cpu.c);
                    return 1;
                },
                0xE => {
                    instructions.LD_r8(&cpu.c, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0xF => {
                    instructions.RRCA(cpu);
                    return 1;
                },
            }
        },
        0x1 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x2 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x3 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x4 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x5 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x6 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x7 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x8 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x9 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xA => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xB => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xC => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xD => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xE => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xF => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
    }
    return 0;
}

/// Executes a single opcode which has been prefixed with $CB. Returns number of cycles
fn fetchAndExecuteCB(cpu: *CPU) usize {
    const opcode = cpu.memoryRead(cpu.pc);
    cpu.pc += 1;

    switch (bitutils.getFirstNibble(opcode)) {
        0x0 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x1 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x2 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x3 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x4 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x5 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x6 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x7 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x8 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0x9 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xA => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xB => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xC => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xD => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xE => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
        0xF => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {

                },
                0x1 => {

                },
                0x2 => {

                },
                0x3 => {

                },
                0x4 => {

                },
                0x5 => {

                },
                0x6 => {

                },
                0x7 => {

                },
                0x8 => {

                },
                0x9 => {

                },
                0xA => {

                },
                0xB => {

                },
                0xC => {

                },
                0xD => {

                },
                0xE => {

                },
                0xF => {

                },
            }
        },
    }
}
