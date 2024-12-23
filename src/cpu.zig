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
        return bitutils.concatBytes(lo, hi);
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
                    const value = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
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
                    const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
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
                    instructions.STOP();
                    return 2;
                },
                0x1 => {
                    const value = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                    cpu.pc +%= 2;
                    instructions.LD_r16(cpu, value, "DE");
                    return 3;
                },
                0x2 => {
                    instructions.LD_n16_A(cpu, cpu.getDE());
                    return 2;
                },
                0x3 => {
                    instructions.INC16(cpu, "DE");
                    return 2;
                },
                0x4 => {
                    instructions.INC8(cpu, &cpu.d);
                    return 1;
                },
                0x5 => {
                    instructions.DEC8(cpu, &cpu.d);
                    return 1;
                },
                0x6 => {
                    instructions.LD_r8(&cpu.d, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0x7 => {
                    instructions.RLA(cpu);
                    return 1;
                },
                0x8 => {
                    instructions.JR(cpu, @bitCast(cpu.memoryRead(cpu.pc)));
                    cpu.pc +%= 1;
                    return 3;
                },
                0x9 => {
                    instructions.ADD16(cpu, cpu.getDE());
                    return 2;
                },
                0xA => {
                    instructions.LD_r8(&cpu.a, cpu.memoryRead(cpu.getDE()));
                    return 2;
                },
                0xB => {
                    instructions.DEC16(cpu, "DE");
                    return 2;
                },
                0xC => {
                    instructions.INC8(cpu, &cpu.e);
                    return 1;
                },
                0xD => {
                    instructions.DEC8(cpu, &cpu.e);
                    return 1;
                },
                0xE => {
                    instructions.LD_r8(&cpu.e, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0xF => {
                    instructions.RRA(cpu);
                    return 1;
                },
            }
        },
        0x2 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    if (cpu.getZero() == 0) {
                        instructions.JR(cpu, @bitCast(cpu.memoryRead(cpu.pc)));
                        cpu.pc +%= 1;
                        return 3;
                    } else {
                        cpu.pc +%= 1;
                        return 2;
                    }
                },
                0x1 => {
                    const value = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                    cpu.pc +%= 2;
                    instructions.LD_r16(cpu, value, "HL");
                    return 3;
                },
                0x2 => {
                    instructions.LD_HLI_A(cpu);
                    return 2;
                },
                0x3 => {
                    instructions.INC16(cpu, "HL");
                    return 2;
                },
                0x4 => {
                    instructions.INC8(cpu, &cpu.h);
                    return 1;
                },
                0x5 => {
                    instructions.DEC8(cpu, &cpu.h);
                    return 1;
                },
                0x6 => {
                    instructions.LD_r8(&cpu.h, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0x7 => {
                    instructions.DAA(cpu);
                    return 1;
                },
                0x8 => {
                    if (cpu.getZero() == 1) {
                        instructions.JR(cpu, @bitCast(cpu.memoryRead(cpu.pc)));
                        cpu.pc +%= 1;
                        return 3;
                    } else {
                        cpu.pc +%= 1;
                        return 2;
                    }
                },
                0x9 => {
                    instructions.ADD16(cpu, cpu.getHL());
                    return 2;
                },
                0xA => {
                    instructions.LD_A_HLI(cpu);
                    return 2;
                },
                0xB => {
                    instructions.DEC16(cpu, "HL");
                    return 2;
                },
                0xC => {
                    instructions.INC8(cpu, &cpu.l);
                    return 1;
                },
                0xD => {
                    instructions.DEC8(cpu, &cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.LD_r8(&cpu.l, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0xF => {
                    instructions.CPL(cpu);
                    return 1;
                },
            }
        },
        0x3 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    if (cpu.getCarry() == 0) {
                        instructions.JR(cpu, @bitCast(cpu.memoryRead(cpu.pc)));
                        cpu.pc +%= 1;
                        return 3;
                    } else {
                        cpu.pc +%= 1;
                        return 2;
                    }
                },
                0x1 => {
                    const value = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                    cpu.pc +%= 2;
                    instructions.LD_SP_n16(cpu, value);
                    return 3;
                },
                0x2 => {
                    instructions.LD_HLD_A(cpu);
                    return 2;
                },
                0x3 => {
                    instructions.INC16(cpu, "SP");
                    return 2;
                },
                0x4 => {
                    instructions.INC16(cpu, "HL");
                    return 3;
                },
                0x5 => {
                    instructions.DEC16(cpu, "HL");
                    return 3;
                },
                0x6 => {
                    instructions.LD_HL(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 3;
                },
                0x7 => {
                    instructions.SCF(cpu);
                    return 1;
                },
                0x8 => {
                    if (cpu.getCarry() == 1) {
                        instructions.JR(cpu, @bitCast(cpu.memoryRead(cpu.pc)));
                        cpu.pc +%= 1;
                        return 3;
                    } else {
                        cpu.pc +%= 1;
                        return 2;
                    }
                },
                0x9 => {
                    instructions.ADD_HL_SP(cpu);
                    return 2;
                },
                0xA => {
                    instructions.LD_A_HLD(cpu);
                    return 2;
                },
                0xB => {
                    instructions.DEC16(cpu, "SP");
                    return 2;
                },
                0xC => {
                    instructions.INC8(cpu, &cpu.a);
                    return 1;
                },
                0xD => {
                    instructions.DEC8(cpu, &cpu.a);
                    return 1;
                },
                0xE => {
                    instructions.LD_A_n16(cpu, cpu.a);
                    return 2;
                },
                0xF => {
                    instructions.CCF(cpu);
                    return 1;
                },
            }
        },
        0x4 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.LD_r8(&cpu.b, cpu.b);
                    return 1;
                },
                0x1 => {
                    instructions.LD_r8(&cpu.b, cpu.c);
                    return 1;
                },
                0x2 => {
                    instructions.LD_r8(&cpu.b, cpu.d);
                    return 1;
                },
                0x3 => {
                    instructions.LD_r8(&cpu.b, cpu.e);
                    return 1;
                },
                0x4 => {
                    instructions.LD_r8(&cpu.b, cpu.h);
                    return 1;
                },
                0x5 => {
                    instructions.LD_r8(&cpu.b, cpu.l);
                    return 1;
                },
                0x6 => {
                    instructions.LD_r8(&cpu.b, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0x7 => {
                    instructions.LD_r8(&cpu.b, cpu.a);
                    return 1;
                },
                0x8 => {
                    instructions.LD_r8(&cpu.c, cpu.b);
                    return 1;
                },
                0x9 => {
                    instructions.LD_r8(&cpu.c, cpu.c);
                    return 1;
                },
                0xA => {
                    instructions.LD_r8(&cpu.c, cpu.d);
                    return 1;
                },
                0xB => {
                    instructions.LD_r8(&cpu.c, cpu.e);
                    return 1;
                },
                0xC => {
                    instructions.LD_r8(&cpu.c, cpu.h);
                    return 1;
                },
                0xD => {
                    instructions.LD_r8(&cpu.c, cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.LD_r8(&cpu.c, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0xF => {
                    instructions.LD_r8(&cpu.c, cpu.a);
                    return 1;
                },
            }
        },
        0x5 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.LD_r8(&cpu.d, cpu.b);
                    return 1;
                },
                0x1 => {
                    instructions.LD_r8(&cpu.d, cpu.c);
                    return 1;
                },
                0x2 => {
                    instructions.LD_r8(&cpu.d, cpu.d);
                    return 1;
                },
                0x3 => {
                    instructions.LD_r8(&cpu.d, cpu.e);
                    return 1;
                },
                0x4 => {
                    instructions.LD_r8(&cpu.d, cpu.h);
                    return 1;
                },
                0x5 => {
                    instructions.LD_r8(&cpu.d, cpu.l);
                    return 1;
                },
                0x6 => {
                    instructions.LD_r8(&cpu.d, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0x7 => {
                    instructions.LD_r8(&cpu.d, cpu.a);
                    return 1;
                },
                0x8 => {
                    instructions.LD_r8(&cpu.e, cpu.b);
                    return 1;
                },
                0x9 => {
                    instructions.LD_r8(&cpu.e, cpu.c);
                    return 1;
                },
                0xA => {
                    instructions.LD_r8(&cpu.e, cpu.d);
                    return 1;
                },
                0xB => {
                    instructions.LD_r8(&cpu.e, cpu.e);
                    return 1;
                },
                0xC => {
                    instructions.LD_r8(&cpu.e, cpu.h);
                    return 1;
                },
                0xD => {
                    instructions.LD_r8(&cpu.e, cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.LD_r8(&cpu.e, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0xF => {
                    instructions.LD_r8(&cpu.e, cpu.a);
                    return 1;
                },
            }
        },
        0x6 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.LD_r8(&cpu.h, cpu.b);
                    return 1;
                },
                0x1 => {
                    instructions.LD_r8(&cpu.h, cpu.c);
                    return 1;
                },
                0x2 => {
                    instructions.LD_r8(&cpu.h, cpu.d);
                    return 1;
                },
                0x3 => {
                    instructions.LD_r8(&cpu.h, cpu.e);
                    return 1;
                },
                0x4 => {
                    instructions.LD_r8(&cpu.h, cpu.h);
                    return 1;
                },
                0x5 => {
                    instructions.LD_r8(&cpu.h, cpu.l);
                    return 1;
                },
                0x6 => {
                    instructions.LD_r8(&cpu.h, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0x7 => {
                    instructions.LD_r8(&cpu.h, cpu.a);
                    return 1;
                },
                0x8 => {
                    instructions.LD_r8(&cpu.l, cpu.b);
                    return 1;
                },
                0x9 => {
                    instructions.LD_r8(&cpu.l, cpu.c);
                    return 1;
                },
                0xA => {
                    instructions.LD_r8(&cpu.l, cpu.d);
                    return 1;
                },
                0xB => {
                    instructions.LD_r8(&cpu.l, cpu.e);
                    return 1;
                },
                0xC => {
                    instructions.LD_r8(&cpu.l, cpu.h);
                    return 1;
                },
                0xD => {
                    instructions.LD_r8(&cpu.l, cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.LD_r8(&cpu.l, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0xF => {
                    instructions.LD_r8(&cpu.l, cpu.a);
                    return 1;
                },
            }
        },
        0x7 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.LD_HL(cpu, cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.LD_HL(cpu, cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.LD_HL(cpu, cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.LD_HL(cpu, cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.LD_HL(cpu, cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.LD_HL(cpu, cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.HALT();
                    return 1;
                },
                0x7 => {
                    instructions.LD_HL(cpu, cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.LD_r8(&cpu.a, cpu.b);
                    return 1;
                },
                0x9 => {
                    instructions.LD_r8(&cpu.a, cpu.c);
                    return 1;
                },
                0xA => {
                    instructions.LD_r8(&cpu.a, cpu.d);
                    return 1;
                },
                0xB => {
                    instructions.LD_r8(&cpu.a, cpu.e);
                    return 1;
                },
                0xC => {
                    instructions.LD_r8(&cpu.a, cpu.h);
                    return 1;
                },
                0xD => {
                    instructions.LD_r8(&cpu.a, cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.LD_r8(&cpu.a, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0xF => {
                    instructions.LD_r8(&cpu.a, cpu.a);
                    return 1;
                },
            }
        },
        0x8 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.ADD8(cpu, cpu.b);
                    return 1;
                },
                0x1 => {
                    instructions.ADD8(cpu, cpu.c);
                    return 1;
                },
                0x2 => {
                    instructions.ADD8(cpu, cpu.d);
                    return 1;
                },
                0x3 => {
                    instructions.ADD8(cpu, cpu.e);
                    return 1;
                },
                0x4 => {
                    instructions.ADD8(cpu, cpu.h);
                    return 1;
                },
                0x5 => {
                    instructions.ADD8(cpu, cpu.l);
                    return 1;
                },
                0x6 => {
                    instructions.ADD8(cpu, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0x7 => {
                    instructions.ADD8(cpu, cpu.a);
                    return 1;
                },
                0x8 => {
                    instructions.ADC(cpu, cpu.b);
                    return 1;
                },
                0x9 => {
                    instructions.ADC(cpu, cpu.c);
                    return 1;
                },
                0xA => {
                    instructions.ADC(cpu, cpu.d);
                    return 1;
                },
                0xB => {
                    instructions.ADC(cpu, cpu.e);
                    return 1;
                },
                0xC => {
                    instructions.ADC(cpu, cpu.h);
                    return 1;
                },
                0xD => {
                    instructions.ADC(cpu, cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.ADC(cpu, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0xF => {
                    instructions.ADC(cpu, cpu.a);
                    return 1;
                },
            }
        },
        0x9 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.SUB(cpu, cpu.b);
                    return 1;
                },
                0x1 => {
                    instructions.SUB(cpu, cpu.c);
                    return 1;
                },
                0x2 => {
                    instructions.SUB(cpu, cpu.d);
                    return 1;
                },
                0x3 => {
                    instructions.SUB(cpu, cpu.e);
                    return 1;
                },
                0x4 => {
                    instructions.SUB(cpu, cpu.h);
                    return 1;
                },
                0x5 => {
                    instructions.SUB(cpu, cpu.l);
                    return 1;
                },
                0x6 => {
                    instructions.SUB(cpu, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0x7 => {
                    instructions.SUB(cpu, cpu.a);
                    return 1;
                },
                0x8 => {
                    instructions.SBC(cpu, cpu.b);
                    return 1;
                },
                0x9 => {
                    instructions.SBC(cpu, cpu.c);
                    return 1;
                },
                0xA => {
                    instructions.SBC(cpu, cpu.d);
                    return 1;
                },
                0xB => {
                    instructions.SBC(cpu, cpu.e);
                    return 1;
                },
                0xC => {
                    instructions.SBC(cpu, cpu.h);
                    return 1;
                },
                0xD => {
                    instructions.SBC(cpu, cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.SBC(cpu, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0xF => {
                    instructions.SBC(cpu, cpu.a);
                    return 1;
                },
            }
        },
        0xA => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.AND(cpu, cpu.b);
                    return 1;
                },
                0x1 => {
                    instructions.AND(cpu, cpu.c);
                    return 1;
                },
                0x2 => {
                    instructions.AND(cpu, cpu.d);
                    return 1;
                },
                0x3 => {
                    instructions.AND(cpu, cpu.e);
                    return 1;
                },
                0x4 => {
                    instructions.AND(cpu, cpu.h);
                    return 1;
                },
                0x5 => {
                    instructions.AND(cpu, cpu.l);
                    return 1;
                },
                0x6 => {
                    instructions.AND(cpu, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0x7 => {
                    instructions.AND(cpu, cpu.a);
                    return 1;
                },
                0x8 => {
                    instructions.XOR(cpu, cpu.b);
                    return 1;
                },
                0x9 => {
                    instructions.XOR(cpu, cpu.c);
                    return 1;
                },
                0xA => {
                    instructions.XOR(cpu, cpu.d);
                    return 1;
                },
                0xB => {
                    instructions.XOR(cpu, cpu.e);
                    return 1;
                },
                0xC => {
                    instructions.XOR(cpu, cpu.h);
                    return 1;
                },
                0xD => {
                    instructions.XOR(cpu, cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.XOR(cpu, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0xF => {
                    instructions.XOR(cpu, cpu.a);
                    return 1;
                },
            }
        },
        0xB => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.OR(cpu, cpu.b);
                    return 1;
                },
                0x1 => {
                    instructions.OR(cpu, cpu.c);
                    return 1;
                },
                0x2 => {
                    instructions.OR(cpu, cpu.d);
                    return 1;
                },
                0x3 => {
                    instructions.OR(cpu, cpu.e);
                    return 1;
                },
                0x4 => {
                    instructions.OR(cpu, cpu.h);
                    return 1;
                },
                0x5 => {
                    instructions.OR(cpu, cpu.l);
                    return 1;
                },
                0x6 => {
                    instructions.OR(cpu, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0x7 => {
                    instructions.OR(cpu, cpu.a);
                    return 1;
                },
                0x8 => {
                    instructions.CP(cpu, cpu.b);
                    return 1;
                },
                0x9 => {
                    instructions.CP(cpu, cpu.c);
                    return 1;
                },
                0xA => {
                    instructions.CP(cpu, cpu.d);
                    return 1;
                },
                0xB => {
                    instructions.CP(cpu, cpu.e);
                    return 1;
                },
                0xC => {
                    instructions.CP(cpu, cpu.h);
                    return 1;
                },
                0xD => {
                    instructions.CP(cpu, cpu.l);
                    return 1;
                },
                0xE => {
                    instructions.CP(cpu, cpu.memoryRead(cpu.getHL()));
                    return 2;
                },
                0xF => {
                    instructions.CP(cpu, cpu.a);
                    return 1;
                },
            }
        },
        0xC => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    if (cpu.getZero() == 0) {
                        instructions.RET(cpu);
                        return 5;
                    } else {
                        return 2;
                    }
                },
                0x1 => {
                    instructions.POP(cpu, "BC");
                    return 3;
                },
                0x2 => {
                    if (cpu.getZero() == 0) {
                        const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                        cpu.pc +%= 2;
                        instructions.JP(cpu, address);
                        return 4;
                    } else {
                        cpu.pc +%= 2;
                        return 3;
                    }

                },
                0x3 => {
                    const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                    cpu.pc +%= 2;
                    instructions.JP(cpu, address);
                    return 4;
                },
                0x4 => {
                    if (cpu.getZero() == 0) {
                        const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                        cpu.pc +%= 2;
                        instructions.CALL(cpu, address);
                        return 6;
                    } else {
                        cpu.pc +%= 2;
                        return 3;
                    }
                },
                0x5 => {
                    instructions.PUSH(cpu, "BC");
                    return 4;
                },
                0x6 => {
                    instructions.ADD8(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0x7 => {
                    instructions.RST(cpu, 0x00);
                    return 4;
                },
                0x8 => {
                    if (cpu.getZero() == 1) {
                        instructions.RET(cpu);
                        return 5;
                    } else {
                        return 2;
                    }
                },
                0x9 => {
                    instructions.RET(cpu);
                    return 4;
                },
                0xA => {
                    if (cpu.getZero() == 1) {
                        const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                        cpu.pc +%= 2;
                        instructions.JP(cpu, address);
                        return 4;
                    } else {
                        cpu.pc +%= 2;
                        return 3;
                    }
                },
                0xB => {
                    return fetchAndExecuteCB(cpu);
                },
                0xC => {
                    if (cpu.getZero() == 1) {
                        const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                        cpu.pc +%= 2;
                        instructions.CALL(cpu, address);
                        return 6;
                    } else {
                        cpu.pc +%= 2;
                        return 3;
                    }
                },
                0xD => {
                    const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                    cpu.pc +%= 2;
                    instructions.CALL(cpu, address);
                    return 6;
                },
                0xE => {
                    instructions.ADC(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0xF => {
                    instructions.RST(cpu, 0x08);
                    return 4;
                },
            }
        },
        0xD => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    if (cpu.getCarry() == 0) {
                        instructions.RET(cpu);
                        return 5;
                    } else {
                        return 2;
                    }
                },
                0x1 => {
                    instructions.POP(cpu, "DE");
                    return 3;
                },
                0x2 => {
                    if (cpu.getCarry() == 0) {
                        const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                        cpu.pc +%= 2;
                        instructions.JP(cpu, address);
                        return 4;
                    } else {
                        cpu.pc +%= 2;
                        return 3;
                    }

                },
                0x3 => {
                    return 0;
                },
                0x4 => {
                    if (cpu.getCarry() == 0) {
                        const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                        cpu.pc +%= 2;
                        instructions.CALL(cpu, address);
                        return 6;
                    } else {
                        cpu.pc +%= 2;
                        return 3;
                    }
                },
                0x5 => {
                    instructions.PUSH(cpu, "DE");
                    return 4;
                },
                0x6 => {
                    instructions.SUB(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0x7 => {
                    instructions.RST(cpu, 0x10);
                    return 4;
                },
                0x8 => {
                    if (cpu.getCarry() == 1) {
                        instructions.RET(cpu);
                        return 5;
                    } else {
                        return 2;
                    }
                },
                0x9 => {
                    instructions.RETI(cpu);
                    return 4;
                },
                0xA => {
                    if (cpu.getCarry() == 1) {
                        const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                        cpu.pc +%= 2;
                        instructions.JP(cpu, address);
                        return 4;
                    } else {
                        cpu.pc +%= 2;
                        return 3;
                    }
                },
                0xB => {
                    return 0;
                },
                0xC => {
                    if (cpu.getCarry() == 1) {
                        const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                        cpu.pc +%= 2;
                        instructions.CALL(cpu, address);
                        return 6;
                    } else {
                        cpu.pc +%= 2;
                        return 3;
                    }
                },
                0xD => {
                    return 0;
                },
                0xE => {
                    instructions.SBC(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0xF => {
                    instructions.RST(cpu, 0x18);
                    return 4;
                },
            }
        },
        0xE => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.LDH_n16_A(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 3;
                },
                0x1 => {
                    instructions.POP(cpu, "HL");
                    return 3;
                },
                0x2 => {
                    instructions.LDH_C_A(cpu);
                    return 2;
                },
                0x3 => {
                    return 0;
                },
                0x4 => {
                    return 0;
                },
                0x5 => {
                    instructions.PUSH(cpu, "HL");
                    return 4;
                },
                0x6 => {
                    instructions.AND(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0x7 => {
                    instructions.RST(cpu, 0x20);
                    return 4;
                },
                0x8 => {
                    instructions.ADD_SP_e8(cpu, @bitCast(cpu.memoryRead(cpu.pc)));
                    cpu.pc +%= 1;
                    return 4;
                },
                0x9 => {
                    instructions.JP(cpu, cpu.getHL());
                    return 1;
                },
                0xA => {
                    const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                    cpu.pc +%= 2;
                    instructions.LD_n16_A(cpu, address);
                    return 4;
                },
                0xB => {
                    return 0;
                },
                0xC => {
                    return 0;
                },
                0xD => {
                    return 0;
                },
                0xE => {
                    instructions.XOR(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0xF => {
                    instructions.RST(cpu, 0x28);
                    return 4;
                },
            }
        },
        0xF => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.LDH_A_n16(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 3;
                },
                0x1 => {
                    instructions.POP(cpu, "AF");
                    return 3;
                },
                0x2 => {
                    instructions.LDH_A_C(cpu);
                    return 2;
                },
                0x3 => {
                    instructions.DI(cpu);
                    return 1;
                },
                0x4 => {
                    return 0;
                },
                0x5 => {
                    instructions.PUSH(cpu, "AF");
                    return 4;
                },
                0x6 => {
                    instructions.OR(cpu, cpu.memoryRead(cpu.pc));
                    cpu.pc +%= 1;
                    return 2;
                },
                0x7 => {
                    instructions.RST(cpu, 0x30);
                    return 4;
                },
                0x8 => {
                    instructions.LD_HL_SP(cpu, @bitCast(cpu.memoryRead(cpu.pc)));
                    cpu.pc +%= 1;
                    return 3;
                },
                0x9 => {
                    instructions.LD_SP_HL(cpu);
                    return 2;
                },
                0xA => {
                    const address = bitutils.concatBytes(cpu.memoryRead(cpu.pc), cpu.memoryRead(cpu.pc+%1));
                    cpu.pc +%= 2;
                    instructions.LD_A_n16(cpu, address);
                    return 4;
                },
                0xB => {
                    instructions.EI();
                    return 1;
                },
                0xC => {
                    return 0;
                },
                0xD => {
                    return 0;
                },
                0xE => {
                    instructions.CP(cpu, cpu.memoryRead(cpu.pc));
                    return 2;
                },
                0xF => {
                    instructions.RST(cpu, 0x38);
                    return 4;
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
                    instructions.RLC_r8(cpu, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.RLC_r8(cpu, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.RLC_r8(cpu, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.RLC_r8(cpu, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.RLC_r8(cpu, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.RLC_r8(cpu, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.RLC_HL(cpu);
                    return 4;
                },
                0x7 => {
                    instructions.RLC_r8(cpu, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.RRC_r8(cpu, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.RRC_r8(cpu, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.RRC_r8(cpu, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.RRC_r8(cpu, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.RRC_r8(cpu, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.RRC_r8(cpu, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.RRC_HL(cpu);
                    return 4;
                },
                0xF => {
                    instructions.RRC_r8(cpu, &cpu.a);
                    return 2;
                },
            }
        },
        0x1 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.RL_r8(cpu, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.RL_r8(cpu, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.RL_r8(cpu, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.RL_r8(cpu, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.RL_r8(cpu, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.RL_r8(cpu, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.RL_HL(cpu);
                    return 4;
                },
                0x7 => {
                    instructions.RL_r8(cpu, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.RR_r8(cpu, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.RR_r8(cpu, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.RR_r8(cpu, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.RR_r8(cpu, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.RR_r8(cpu, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.RR_r8(cpu, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.RR_HL(cpu);
                    return 4;
                },
                0xF => {
                    instructions.RR_r8(cpu, &cpu.a);
                    return 2;
                },
            }
        },
        0x2 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.SLA_r8(cpu, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.SLA_r8(cpu, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.SLA_r8(cpu, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.SLA_r8(cpu, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.SLA_r8(cpu, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.SLA_r8(cpu, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.SLA_HL(cpu);
                    return 4;
                },
                0x7 => {
                    instructions.SLA_r8(cpu, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.SRA_r8(cpu, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.SRA_r8(cpu, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.SRA_r8(cpu, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.SRA_r8(cpu, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.SRA_r8(cpu, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.SRA_r8(cpu, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.SRA_HL(cpu);
                    return 4;
                },
                0xF => {
                    instructions.SRA_r8(cpu, &cpu.a);
                    return 2;
                },
            }
        },
        0x3 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.SWAP_r8(cpu, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.SWAP_r8(cpu, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.SWAP_r8(cpu, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.SWAP_r8(cpu, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.SWAP_r8(cpu, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.SWAP_r8(cpu, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.SWAP_HL(cpu);
                    return 4;
                },
                0x7 => {
                    instructions.SWAP_r8(cpu, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.SRL_r8(cpu, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.SRL_r8(cpu, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.SRL_r8(cpu, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.SRL_r8(cpu, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.SRL_r8(cpu, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.SRL_r8(cpu, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.SRL_HL(cpu);
                    return 4;
                },
                0xF => {
                    instructions.SRL_r8(cpu, &cpu.a);
                    return 2;
                },
            }
        },
        0x4 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.BIT(cpu, 0, cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.BIT(cpu, 0, cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.BIT(cpu, 0, cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.BIT(cpu, 0, cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.BIT(cpu, 0, cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.BIT(cpu, 0, cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.BIT(cpu, 0, cpu.memoryRead(cpu.getHL()));
                    return 3;
                },
                0x7 => {
                    instructions.BIT(cpu, 0, cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.BIT(cpu, 1, cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.BIT(cpu, 1, cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.BIT(cpu, 1, cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.BIT(cpu, 1, cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.BIT(cpu, 1, cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.BIT(cpu, 1, cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.BIT(cpu, 1, cpu.memoryRead(cpu.getHL()));
                    return 3;
                },
                0xF => {
                    instructions.BIT(cpu, 1, cpu.a);
                    return 2;
                },
            }
        },
        0x5 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.BIT(cpu, 2, cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.BIT(cpu, 2, cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.BIT(cpu, 2, cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.BIT(cpu, 2, cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.BIT(cpu, 2, cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.BIT(cpu, 2, cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.BIT(cpu, 2, cpu.memoryRead(cpu.getHL()));
                    return 3;
                },
                0x7 => {
                    instructions.BIT(cpu, 2, cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.BIT(cpu, 3, cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.BIT(cpu, 3, cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.BIT(cpu, 3, cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.BIT(cpu, 3, cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.BIT(cpu, 3, cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.BIT(cpu, 3, cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.BIT(cpu, 3, cpu.memoryRead(cpu.getHL()));
                    return 3;
                },
                0xF => {
                    instructions.BIT(cpu, 3, cpu.a);
                    return 2;
                },
            }
        },
        0x6 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.BIT(cpu, 4, cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.BIT(cpu, 4, cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.BIT(cpu, 4, cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.BIT(cpu, 4, cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.BIT(cpu, 4, cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.BIT(cpu, 4, cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.BIT(cpu, 4, cpu.memoryRead(cpu.getHL()));
                    return 3;
                },
                0x7 => {
                    instructions.BIT(cpu, 4, cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.BIT(cpu, 5, cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.BIT(cpu, 5, cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.BIT(cpu, 5, cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.BIT(cpu, 5, cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.BIT(cpu, 5, cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.BIT(cpu, 5, cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.BIT(cpu, 5, cpu.memoryRead(cpu.getHL()));
                    return 3;
                },
                0xF => {
                    instructions.BIT(cpu, 5, cpu.a);
                    return 2;
                },
            }
        },
        0x7 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.BIT(cpu, 6, cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.BIT(cpu, 6, cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.BIT(cpu, 6, cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.BIT(cpu, 6, cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.BIT(cpu, 6, cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.BIT(cpu, 6, cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.BIT(cpu, 6, cpu.memoryRead(cpu.getHL()));
                    return 3;
                },
                0x7 => {
                    instructions.BIT(cpu, 6, cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.BIT(cpu, 7, cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.BIT(cpu, 7, cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.BIT(cpu, 7, cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.BIT(cpu, 7, cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.BIT(cpu, 7, cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.BIT(cpu, 7, cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.BIT(cpu, 7, cpu.memoryRead(cpu.getHL()));
                    return 3;
                },
                0xF => {
                    instructions.BIT(cpu, 7, cpu.a);
                    return 2;
                },
            }
        },
        0x8 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.RES_r8(0, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.RES_r8(0, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.RES_r8(0, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.RES_r8(0, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.RES_r8(0, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.RES_r8(0, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.RES_HL(cpu, 0);
                    return 4;
                },
                0x7 => {
                    instructions.RES_r8(0, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.RES_r8(1, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.RES_r8(1, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.RES_r8(1, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.RES_r8(1, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.RES_r8(1, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.RES_r8(1, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.RES_HL(cpu, 1);
                    return 4;
                },
                0xF => {
                    instructions.RES_r8(1, &cpu.a);
                    return 2;
                },
            }
        },
        0x9 => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.RES_r8(2, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.RES_r8(2, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.RES_r8(2, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.RES_r8(2, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.RES_r8(2, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.RES_r8(2, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.RES_HL(cpu, 2);
                    return 4;
                },
                0x7 => {
                    instructions.RES_r8(2, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.RES_r8(3, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.RES_r8(3, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.RES_r8(3, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.RES_r8(3, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.RES_r8(3, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.RES_r8(3, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.RES_HL(cpu, 3);
                    return 4;
                },
                0xF => {
                    instructions.RES_r8(3, &cpu.a);
                    return 2;
                },
            }
        },
        0xA => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.RES_r8(4, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.RES_r8(4, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.RES_r8(4, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.RES_r8(4, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.RES_r8(4, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.RES_r8(4, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.RES_HL(cpu, 4);
                    return 4;
                },
                0x7 => {
                    instructions.RES_r8(4, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.RES_r8(5, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.RES_r8(5, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.RES_r8(5, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.RES_r8(5, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.RES_r8(5, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.RES_r8(5, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.RES_HL(cpu, 5);
                    return 4;
                },
                0xF => {
                    instructions.RES_r8(5, &cpu.a);
                    return 2;
                },
            }
        },
        0xB => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.RES_r8(6, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.RES_r8(6, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.RES_r8(6, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.RES_r8(6, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.RES_r8(6, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.RES_r8(6, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.RES_HL(cpu, 6);
                    return 4;
                },
                0x7 => {
                    instructions.RES_r8(6, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.RES_r8(7, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.RES_r8(7, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.RES_r8(7, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.RES_r8(7, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.RES_r8(7, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.RES_r8(7, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.RES_HL(cpu, 7);
                    return 4;
                },
                0xF => {
                    instructions.RES_r8(7, &cpu.a);
                    return 2;
                },
            }
        },
        0xC => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.SET_r8(0, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.SET_r8(0, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.SET_r8(0, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.SET_r8(0, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.SET_r8(0, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.SET_r8(0, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.SET_HL(cpu, 0);
                    return 4;
                },
                0x7 => {
                    instructions.SET_r8(0, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.SET_r8(1, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.SET_r8(1, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.SET_r8(1, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.SET_r8(1, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.SET_r8(1, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.SET_r8(1, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.SET_HL(cpu, 1);
                    return 4;
                },
                0xF => {
                    instructions.SET_r8(1, &cpu.a);
                    return 2;
                },
            }
        },
        0xD => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.SET_r8(2, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.SET_r8(2, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.SET_r8(2, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.SET_r8(2, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.SET_r8(2, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.SET_r8(2, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.SET_HL(cpu, 2);
                    return 4;
                },
                0x7 => {
                    instructions.SET_r8(2, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.SET_r8(3, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.SET_r8(3, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.SET_r8(3, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.SET_r8(3, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.SET_r8(3, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.SET_r8(3, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.SET_HL(cpu, 3);
                    return 4;
                },
                0xF => {
                    instructions.SET_r8(3, &cpu.a);
                    return 2;
                },
            }
        },
        0xE => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.SET_r8(4, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.SET_r8(4, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.SET_r8(4, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.SET_r8(4, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.SET_r8(4, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.SET_r8(4, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.SET_HL(cpu, 4);
                    return 4;
                },
                0x7 => {
                    instructions.SET_r8(4, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.SET_r8(5, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.SET_r8(5, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.SET_r8(5, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.SET_r8(5, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.SET_r8(5, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.SET_r8(5, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.SET_HL(cpu, 5);
                    return 4;
                },
                0xF => {
                    instructions.SET_r8(5, &cpu.a);
                    return 2;
                },
            }
        },
        0xF => {
            switch (bitutils.getSecondNibble(opcode)) {
                0x0 => {
                    instructions.SET_r8(6, &cpu.b);
                    return 2;
                },
                0x1 => {
                    instructions.SET_r8(6, &cpu.c);
                    return 2;
                },
                0x2 => {
                    instructions.SET_r8(6, &cpu.d);
                    return 2;
                },
                0x3 => {
                    instructions.SET_r8(6, &cpu.e);
                    return 2;
                },
                0x4 => {
                    instructions.SET_r8(6, &cpu.h);
                    return 2;
                },
                0x5 => {
                    instructions.SET_r8(6, &cpu.l);
                    return 2;
                },
                0x6 => {
                    instructions.SET_HL(cpu, 6);
                    return 4;
                },
                0x7 => {
                    instructions.SET_r8(6, &cpu.a);
                    return 2;
                },
                0x8 => {
                    instructions.SET_r8(7, &cpu.b);
                    return 2;
                },
                0x9 => {
                    instructions.SET_r8(7, &cpu.c);
                    return 2;
                },
                0xA => {
                    instructions.SET_r8(7, &cpu.d);
                    return 2;
                },
                0xB => {
                    instructions.SET_r8(7, &cpu.e);
                    return 2;
                },
                0xC => {
                    instructions.SET_r8(7, &cpu.h);
                    return 2;
                },
                0xD => {
                    instructions.SET_r8(7, &cpu.l);
                    return 2;
                },
                0xE => {
                    instructions.SET_HL(cpu, 7);
                    return 4;
                },
                0xF => {
                    instructions.SET_r8(7, &cpu.a);
                    return 2;
                },
            }
        },
    }
}
