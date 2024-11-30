//! Implementation of Game Boy device

const CPU = @import("cpu.zig").CPU;
const MMU = @import("mmu.zig").MMU;

pub const GameBoy = struct {
    cpu: CPU,
    mmu: MMU,

    fn init(self: *GameBoy) void {
        self.mmu.init(self);
    }
};
