//! Game Boy memory management unit

const GameBoy = @import("gameboy.zig").GameBoy;
const log = @import("logger.zig");

pub const MMU = struct {
    ram: u8[1024 * 64],  // Temporary 64KB will gradually be reduced to actual RAM size as proper memory mapping is introduced

    gb: *GameBoy,

    fn init(self: *MMU, gb: *GameBoy) void {
        self.gb = gb;
    }

    fn memoryRead(self: *MMU, address: u16) u8 {
        switch (address) {
            0x0000...0xFFFF => {
                return self.ram[address];
            },
            else => {
                log.err("Illegal memory read. Invalid memory address: ${X:0>4}", .{ address });
                return 0x0;
            },
        }
    }

    fn memoryWrite(self: *MMU, address: u16, value: u8) void {
        switch (address) {
            0x0000...0xFFFF => {
                self.ram[address] = value;
            },
            else => {
                log.err("Illegal memory write. Invalid memory address: ${X:0>4}", .{ address });
            },
        }
    }
};
