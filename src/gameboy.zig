//! Implementation of Game Boy device

const cpu = @import("cpu.zig");
const MMU = @import("mmu.zig").MMU;

pub const GameBoy = struct {
    cpu: cpu.CPU,
    mmu: MMU,

    pub fn init(self: *GameBoy) void {
        self.cpu.init(self);
        self.mmu.init(self);
    }

    pub fn run(self: *GameBoy) void {
        _ = cpu.fetchAndExecute(&self.cpu);
    }
};
