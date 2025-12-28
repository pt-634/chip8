#ifndef CHIP8_H
#define CHIP8_H

typedef struct Chip_Context {
    // Memory
    // - 4KB of memory
    // - 0x000 (0) to 0xFFF (4095)
    // - 0x000 to 0x1FF reserved for original interpreter
    // - Big Endian (high byte at mem[pc], low byte at mem[pc+1])
    unsigned char mem[4096];

    // Variable Registers
    // - 16 general purpose 8-bit regs, referred to as Vx (0 <= x <= F)
    // - VF reserved as flag for certain instructions
    unsigned char V[16];

    // I register
    // - 16 bits, only bottom 12 bits used
    unsigned short I;

    // Special Purpose Registers
    // - delay and sound timer, both 8-bit
    // - automatically decremented at rate 60Hz if nonzero
    // - if ST > 0, Chip-8 buzzer will sound
    unsigned char delay;
    unsigned char sound;

    // Pseudo-Registers
    // - program counter (16-bit) and stack pointer (8-bit)
    // - not directly accessible by programs
    // - pc stores currently executing address
    // - sp points at the next free spot in the stack
    unsigned short pc;
    unsigned char sp; // (TODO) fix descriptions of instructions that reference this

    // Stack
    // - array of 16 16-bit values
    // - used to store the address interpreter should return to after subroutine
    // - an implication is that Chip-8 allows for up to 16 levels of nested subroutines
    unsigned short stack[16];

    // Display
    // - 64x32-pixel monochrome display
    // - (0,0) is top left, (63, 31) is bottom right
    unsigned char screen_buffer[32][8];

} Chip_Context;

/**
 * Hard reset. Wipes RAM/ROM and loads fonts
 */
void chip8_reset(Chip_Context *chip8);

/**
 * Loads ROM into memory and sets pc to the start of it
 * Exits with an error if ROM too large to be loaded
 */
void chip8_load_rom(Chip_Context *chip8, const unsigned char *bytes, unsigned short len);

/**
 * Processes the next instruction in memory and updates the state accordingly
 * Keypad keys are 1 when pressed (in the "down" position).
 */
void chip8_step(Chip_Context *chip8, const int keypad[16]); // todo: change the keypad to bool[]

/**
 * Advances the chip8 timers by one tick
 */
void chip8_tick_timers(Chip_Context *chip8);

#endif
