const std = @import("std");

const GameBoy = @import("gameboy.zig").GameBoy;

pub fn main() !void {
    var gb: GameBoy = undefined;
    gb.init();

    while (true) {
        gb.run();
    }
}
