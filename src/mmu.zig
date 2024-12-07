//! Game Boy memory management unit

const GameBoy = @import("gameboy.zig").GameBoy;
const log = @import("logger.zig");

pub const MMU = struct {
    ram: [1024 * 64]u8,  // Temporary 64KB will gradually be reduced to actual RAM size as proper memory mapping is introduced

    gb: *GameBoy,

    pub fn init(self: *MMU, gb: *GameBoy) void {
        self.gb = gb;
    }

    pub fn memoryRead(self: *MMU, address: u16) u8 {
        switch (address) {
            0x0000...0xFFFF => {
                return self.ram[address];
            },
        }
    }

    pub fn memoryWrite(self: *MMU, address: u16, value: u8) void {
        switch (address) {
            0x0000...0xFFFF => {
                self.ram[address] = value;
            },
        }
    }
};
