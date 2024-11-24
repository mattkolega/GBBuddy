//! Implementation of Game Boy CPU (Sharp SM83)

pub const CPU = packed struct {
    // Registers
    A: u8,
    B: u8,
    C: u8,
    D: u8,
    E: u8,
    H: u8,
    L: u8,
    SP: u16,  // Stack pointer
    PC: u16,  // Program counter

    // Flags
    zero: u1,
    subtract: u1,
    halfCarry: u1,
    carry: u1,

    ime: u1,  // Interrupt master enable flag
};
