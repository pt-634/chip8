#include "chip8.h"


void clear_display(unsigned char screen_buffer[32][8]) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            screen_buffer[i][j] = 0;
        }
    }
}


unsigned char FONTSET[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


// something i was wondering about that probably isnt that important is if a reset should reset a running rom or if it should
// zero out everything like how i have it doing now
void chip8_reset(Chip_Context *chip8) {
    // Memory
    // - zero it out
    // - store the fontset in mem[0x050..0x09F] (as for why its not just started at 0x0... god knows... he might not even know neither real talk smh)
    // - the rest of the bottom of the memory doesnt really matter but keep in mind it should remain untouched in the future (?)
    // - maybe just to be funny i put the corresponding code of the original interpreter there lmaoooo. later tho
    for (int i = 0; i < 4096; i++) {
        chip8->mem[i] = 0;
    }

    for (int i = 0; i < 80; i++) {
        chip8->mem[i + 0x050] = FONTSET[i];
    }

    // zero out all the different registers
    for (int i = 0; i < 16; i++) {
        chip8->V[i] = 0;
    }
    chip8->I = 0;
    chip8->delay = 0;
    chip8->sound = 0;
    chip8->pc = 0; // or should it just be 0x200 (?) ok ill do that when i load the rom for now
    chip8->sp = 0;

    // zero out the stack (to help w debugging but will probably remove for speed later though itll probably be negligible anyway)
    for (int i = 0; i < 16; i++) {
        chip8->stack[i] = 0;
    }

    // black screen
    // for (int i = 0; i < 32; i++) {
    //     for (int j = 0; j < 8; j++) {
    //         chip8->screen_buffer[i][j] = 0;
    //     }
    // }
    clear_display(chip8->screen_buffer);
}

int chip8_load_rom(Chip_Context *chip8, const unsigned char *bytes, unsigned short len) {

    // maybe call a chip reset here?
    chip8_reset(chip8);

    // Most Chip-8 programs start at location 0x200 (512), but some begin at 0x600 (1536).
    // Programs beginning at 0x600 are intended for the ETI 660 computer.
    // - with that being said, i'll just load stuff in starting at 0x200

    if (len > 4096 - 0x200) {
        return 1;
    }

    for (int i = 0; i < len; i++) {
        chip8->mem[i + 0x200] = bytes[i];
    }

    chip8->pc = 0x200;

    return 0;
}




typedef struct Decoded_Inst {
    unsigned int op; // A 4-bit value, the upper 4 bits of the high byte of the instruction
    unsigned int x; // A 4-bit value, the lower 4 bits of the high byte of the instruction
    unsigned int y; // A 4-bit value, the upper 4 bits of the low byte of the instruction
    unsigned int kk; // An 8-bit value, the lowest 8 bits of the instruction
    unsigned int nnn; // A 12-bit value, the lowest 12 bits of the instruction
    unsigned int n; // A 4-bit value, the lowest 4 bits of the instruction
} Decoded_Inst;

// (TODO) later but at some point put in an error return type for an unrecognized instruction?
// we can discuss that behavior later, no promises have to be made for a ROM that breaks the contract i think
// but it can help with debugging if im writing my own rom for whatever reason
// maybe that part belongs in execute anyways
Decoded_Inst decode(unsigned short inst) {
    Decoded_Inst decoded_inst;
    decoded_inst.op     = (inst & 0xF000) >> 12;    // 0b1111_0000_0000_0000
    decoded_inst.x      = (inst & 0x0F00) >> 8;     // 0b0000_1111_0000_0000
    decoded_inst.y      = (inst & 0x00F0) >> 4;     // 0b0000_0000_1111_0000
    decoded_inst.kk     = (inst & 0x00FF);          // 0b0000_0000_1111_1111
    decoded_inst.nnn    = (inst & 0x0FFF);          // 0b0000_1111_1111_1111
    decoded_inst.n      = (inst & 0x000F);          // 0b0000_0000_0000_1111
    return decoded_inst;
}

// (TODO) implement
unsigned char rng() {
    return 0xFF; // this thing is just gonna get anded w kk and be kk for now
}

// display a sprite on the screen
// returns 1 if any pixels are erased as a result, 0 otherwise
// behavior undefined for n >= 16
int display_sprite(unsigned char screen_buffer[32][8], unsigned char * sprite, unsigned char x_coord, unsigned char y_coord, unsigned int n) {
    // (TODO) describe implementation here
    // location (x,y), where y indicates the row, and x indicates the column bit
    int col_byte = x_coord / 8; // the byte containing the column bit x
    int col_byte_spillover = (col_byte + 1) % 8; // the byte to the right of col_byte (unless it wraps around)
    int col_shift = x_coord % 8; // how far to shift the sprite to the right from being col_byte aligned
    int col_shift_left = 8 - col_shift;

    int erased = 0;

    for (int i = 0; i < n; i++) {
        int row = (i + y_coord) % 32; // in order to wrap around
        unsigned char original_byte = screen_buffer[row][col_byte];
        unsigned char original_byte_spillover = screen_buffer[row][col_byte_spillover];
        unsigned short original = ((unsigned short)original_byte << 8) | (unsigned short)original_byte_spillover;

        unsigned char sprite_byte = sprite[i];
        unsigned short sprite_mask = (unsigned short)sprite_byte << col_shift_left;

        unsigned short masked_bytes = original ^ sprite_mask;
        unsigned char new_byte = 0xFF & (masked_bytes >> 8);
        unsigned char new_byte_spillover = 0xFF & masked_bytes;
        screen_buffer[row][col_byte] = new_byte;
        screen_buffer[row][col_byte_spillover] = new_byte_spillover;

        // (TODO) explain how this works
        if ((original & sprite_mask) != 0) erased = 1;
    }

    // (TODO) explain why this works
    return erased;
}

// // (TODO) delete at some point when im ready to let go of my old code lmao
// // returns 1 if any pixels are erased as a result, 0 otherwise
// int display_sprite_deprecated(unsigned char screen_buffer[32][8], unsigned char * sprite, unsigned char x_coord, unsigned char y_coord, unsigned int n) {
//     // sidenote n should technically have a max value of 16
//     // location (x,y), where y indicates the row, and x indicates the column bit
//     int col_byte = x_coord / 8; // the byte containing the column bit x
//     int col_byte_spillover = (col_byte + 1) % 8; // the byte to the right of col_byte (unless it wraps around)
//     int col_shift = x_coord % 8; // how far to shift the sprite to the right from being byte aligned
//     int col_shift_spillover = 8 - col_shift; // how far to shift sprite to the left from being spillover byte aligned

//     for (int i = 0; i < n; i++) {
//         int row = (i + y_coord) % 32; // in order to wrap around
//         unsigned char original_byte = screen_buffer[row][col_byte];
//         unsigned char original_byte_spillover = screen_buffer[row][col_byte_spillover];
//         unsigned char sprite_byte = sprite[i];
//         unsigned char sprite_mask = sprite_byte >> col_shift;
//         unsigned char sprite_mask_spillover = sprite_byte << col_shift_spillover;
//         unsigned char new_byte = original_byte ^ sprite_mask;
//         unsigned char new_byte_spillover = original_byte_spillover ^ sprite_mask_spillover;
//         screen_buffer[row][col_byte] = new_byte;
//         screen_buffer[row][col_byte_spillover] = new_byte_spillover;
//     }

//     // (TODO) determine if a pixel was erased
// }

void chip8_step(Chip_Context *chip8, const int keypad[16]) {
    // incrementing pc should occur last (if needed). +=2 by default
    // for ease im gonna use a redirect_pc flag thats either 0 or 1 to indicate if i add 2 or not
    int redirect_pc = 0; // (TODO) make a bool, if 0 we add 2 otherwise no


    // FETCH
    unsigned short inst = (chip8->mem[chip8->pc] << 8) | chip8->mem[chip8->pc + 1]; // leftmost will be highest byte


    // DECODE
    Decoded_Inst decoded_inst = decode(inst); // worth writing for debugging purposes


    // EXECUTE
    switch (decoded_inst.op) {
        case 0x0: {
            if (inst == 0x00E0) {
                // 00E0 - CLS
                // Clear the display.
                clear_display(chip8->screen_buffer);
            }
            else if (inst == 0x00EE)
            {
                // 00EE - RET
                // Return from a subroutine
                // The interpreter sets the program counter to the address at the top of the stack,
                // then subtracts 1 from the stack pointer.
                // (note that sp doesnt point to the top of the stack in this implementation)
                if (chip8->sp <= 0) {
                    // (TODO) enter error state (for now i just skip)
                }
                else {
                    chip8->sp -= 1;
                    chip8->pc = chip8->stack[chip8->sp];
                    redirect_pc = 1;
                }
            }
            else {
                // 0nnn - SYS addr
                // Jump to a machine code routine at nnn.
                // This instruction is only used on the old computers on which Chip-8 was originally implemented.
                // It is ignored by modern interpreters.
                // (TODO) for now i wont implement but maybe for accuracy i implement later
            }
            break;
        }
        case 0x1: {
            // 1nnn - JP addr
            // Jump to location nnn.
            // The interpreter sets the program counter to nnn.
            chip8->pc = decoded_inst.nnn;
            redirect_pc = 1;
            break;
        }
        case 0x2: {
            // 2nnn - CALL addr
            // Call subroutine at nnn.
            // The interpreter increments the stack pointer, then puts the next PC on the top of the stack.
            // The PC is then set to nnn.
            // (note that sp doesnt point to the top of the stack in this implementation)
            if (chip8->sp >= 16) {
                // (TODO) enter error state (for now i just skip)
            }
            else {
                chip8->stack[chip8->sp] = chip8->pc + 2;
                chip8->sp += 1;
                chip8->pc = decoded_inst.nnn;
                redirect_pc = 1;
            }
            break;
        }
        case 0x3: {
            // 3xkk - SE Vx, byte
            // Skip next instruction if Vx = kk.
            // The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
            if (chip8->V[decoded_inst.x] == decoded_inst.kk) {
                chip8->pc += 2; // dont consider a skip a redirect so it can actually skip this instruction
            }
            break;
        }
        case 0x4: {
            // 4xkk - SNE Vx, byte
            // Skip next instruction if Vx != kk.
            // The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
            if (chip8->V[decoded_inst.x] != decoded_inst.kk) {
                chip8->pc += 2; // dont consider a skip a redirect so it can actually skip this instruction
            }
            break;
        }
        case 0x5: {
            // currently the only valid skip instruction ends with a 0. adding an exit for invalid 5xyn where n != 0
            if (decoded_inst.n != 0) break;

            // 5xy0 - SE Vx, Vy
            // Skip next instruction if Vx = Vy.
            // The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
            if (chip8->V[decoded_inst.x] == chip8->V[decoded_inst.y]) {
                chip8->pc += 2; // dont consider a skip a redirect so it can actually skip this instruction
            }
            break;
        }
        case 0x6: {
            // 6xkk - LD Vx, byte
            // Set Vx = kk.
            // The interpreter puts the value kk into register Vx.
            chip8->V[decoded_inst.x] = decoded_inst.kk;
            break;
        }
        case 0x7: {
            // 7xkk - ADD Vx, byte
            // Set Vx = Vx + kk.
            // Adds the value kk to the value of register Vx, then stores the result in Vx.
            chip8->V[decoded_inst.x] += decoded_inst.kk;
            break;
        }
        case 0x8: {
            // there are a few of these which only differ by the nibble
            switch (decoded_inst.n) {
                case (0x0): {
                    // 8xy0 - LD Vx, Vy
                    // Set Vx = Vy.
                    // Stores the value of register Vy in register Vx.
                    chip8->V[decoded_inst.x] = chip8->V[decoded_inst.y];
                    break;
                }
                case (0x1): {
                    // 8xy1 - OR Vx, Vy
                    // Set Vx = Vx OR Vy.
                    // Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
                    chip8->V[decoded_inst.x] |= chip8->V[decoded_inst.y];
                    break;
                }
                case (0x2): {
                    // 8xy2 - AND Vx, Vy
                    // Set Vx = Vx AND Vy.
                    // Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
                    chip8->V[decoded_inst.x] &= chip8->V[decoded_inst.y];
                    break;
                }
                case (0x3): {
                    // 8xy3 - XOR Vx, Vy
                    // Set Vx = Vx XOR Vy.
                    // Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
                    chip8->V[decoded_inst.x] ^= chip8->V[decoded_inst.y];
                    break;
                }
                case (0x4): {
                    // 8xy4 - ADD Vx, Vy
                    // Set Vx = Vx + Vy, set VF = carry.
                    // The values of Vx and Vy are added together.
                    // If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
                    // Only the lowest 8 bits of the result are kept, and stored in Vx.
                    int res = (int)chip8->V[decoded_inst.x] + (int)chip8->V[decoded_inst.y];
                    chip8->V[0xF] = 0x1 & (res >> 8);
                    chip8->V[decoded_inst.x] = (unsigned char)(0xFF & res); // idk if the unsigned char conversion here matters
                    break;
                }
                case (0x5): {
                    // 8xy5 - SUB Vx, Vy
                    // Set Vx = Vx - Vy, set VF = NOT borrow.
                    // If Vx > Vy, then VF is set to 1, otherwise 0.
                    // Then Vy is subtracted from Vx, and the results stored in Vx.
                    int res = (int)chip8->V[decoded_inst.x] - (int)chip8->V[decoded_inst.y];
                    chip8->V[0xF] = (res > 0) ? 1 : 0;
                    chip8->V[decoded_inst.x] = (unsigned char)(0xFF & res); // idk if the unsigned char conversion here matters
                    break;
                }
                case (0x6): {
                    // 8xy6 - SHR Vx {, Vy}
                    // Set Vx = Vx SHR 1.
                    // If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0.
                    // Then Vx is divided by 2.
                    chip8->V[0xF] = 0x1 & chip8->V[decoded_inst.x];
                    chip8->V[decoded_inst.x] >>= 1;
                    break;
                }
                case (0x7): {
                    // 8xy7 - SUBN Vx, Vy
                    // Set Vx = Vy - Vx, set VF = NOT borrow.
                    // If Vy > Vx, then VF is set to 1, otherwise 0.
                    // Then Vx is subtracted from Vy, and the results stored in Vx.
                    int res = (int)chip8->V[decoded_inst.y] - (int)chip8->V[decoded_inst.x];
                    chip8->V[0xF] = (res > 0) ? 1 : 0;
                    chip8->V[decoded_inst.x] = (unsigned char)(0xFF & res); // idk if the unsigned char conversion here matters
                    break;
                }
                case (0xE): {
                    // 8xyE - SHL Vx {, Vy}
                    // Set Vx = Vx SHL 1.
                    // If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
                    // Then Vx is multiplied by 2.
                    chip8->V[0xF] = 0x1 & (chip8->V[decoded_inst.x] >> 7);
                    chip8->V[decoded_inst.x] <<= 1;
                    break;
                }
                default: break;
            }
            break;
        }
        case 0x9: {
            // currently the only valid skip instruction ends with a 0. adding an exit for invalid 9xyn where n != 0
            if (decoded_inst.n != 0) break;

            // 9xy0 - SNE Vx, Vy
            // Skip next instruction if Vx != Vy.
            // The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
            if (chip8->V[decoded_inst.x] != chip8->V[decoded_inst.y]) {
                chip8->pc += 2; // dont consider a skip a redirect so it can actually skip this instruction
            }
            break;
        }
        case 0xA: {
            // Annn - LD I, addr
            // Set I = nnn.
            // The value of register I is set to nnn.
            chip8->I = decoded_inst.nnn;
            break;
        }
        case 0xB: {
            // Bnnn - JP V0, addr
            // Jump to location nnn + V0.
            // The program counter is set to nnn plus the value of V0.
            chip8->pc = decoded_inst.nnn + chip8->V[0];
            redirect_pc = 1;
            break;
        }
        case 0xC: {
            // Cxkk - RND Vx, byte
            // Set Vx = random byte AND kk.
            // The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk.
            // The results are stored in Vx.
            chip8->V[decoded_inst.x] = rng() & decoded_inst.kk; // idk if explicit conversion of kk from int to char matters
            break;
        }
        case 0xD: {
            // Dxyn - DRW Vx, Vy, nibble
            // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            // The interpreter reads n bytes from memory, starting at the address stored in I.
            // These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
            // Sprites are XORed onto the existing screen.
            // If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
            // If the sprite is positioned so part of it is outside the coordinates of the display,
            // it wraps around to the opposite side of the screen.
            chip8->V[0xF] = display_sprite(
                chip8->screen_buffer,
                &(chip8->mem[chip8->I]),
                chip8->V[decoded_inst.x],
                chip8->V[decoded_inst.y],
                decoded_inst.n
            );
            break;
        }
        case 0xE: {
            // there are two, which differ by the kk byte
            switch (decoded_inst.kk) {
                case 0x9E: {
                    // Ex9E - SKP Vx
                    // Skip next instruction if key with the value of Vx is pressed.
                    // Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position,
                    // PC is increased by 2.
                    unsigned int value = chip8->V[decoded_inst.x];
                    if (value > 0xF) break; // not sure if this should or shouldnt skip the next instruction
                    if (keypad[value] == 1) {
                        chip8->pc += 2; // dont consider a skip a redirect so it can actually skip this instruction
                    }
                    break;
                }
                case 0xA1: {
                    // ExA1 - SKNP Vx
                    // Skip next instruction if key with the value of Vx is not pressed.
                    // Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position,
                    // PC is increased by 2.
                    unsigned int value = chip8->V[decoded_inst.x];
                    if (value > 0xF) break; // not sure if this should or shouldnt skip the next instruction
                    if (keypad[value] != 1) {
                        chip8->pc += 2; // dont consider a skip a redirect so it can actually skip this instruction
                    }
                    break;
                }
                default: break;
            }
            break;
        }
        case 0xF: {
            // there are a few, which differ by the kk byte
            switch (decoded_inst.kk) {
                case 0x07: {
                    // Fx07 - LD Vx, DT
                    // Set Vx = delay timer value.
                    // The value of DT is placed into Vx.
                    chip8->V[decoded_inst.x] = chip8->delay;
                    break;
                }
                case 0x0A: {
                    // Fx0A - LD Vx, K
                    // Wait for a key press, store the value of the key in Vx.
                    // All execution stops until a key is pressed, then the value of that key is stored in Vx.
                    // note: this interpreter prioritizes the lowest value key
                    redirect_pc = 1; // if no key is found to be pressed, we stop the pc from proceeding
                    for (int key = 0; key < 16; key++) {
                        if (keypad[key] == 1) {
                            chip8->V[decoded_inst.x] = key;
                            redirect_pc = 0;
                            break;
                        }
                    }
                    break;
                }
                case 0x15: {
                    // Fx15 - LD DT, Vx
                    // Set delay timer = Vx.
                    // DT is set equal to the value of Vx.
                    chip8->delay = chip8->V[decoded_inst.x];
                    break;
                }
                case 0x18: {
                    // Fx18 - LD ST, Vx
                    // Set sound timer = Vx.
                    // ST is set equal to the value of Vx.
                    chip8->sound = chip8->V[decoded_inst.x];
                    break;
                }
                case 0x1E: {
                    // Fx1E - ADD I, Vx
                    // Set I = I + Vx.
                    // The values of I and Vx are added, and the results are stored in I.
                    chip8->I = 0x0FFF & (chip8->I + chip8->V[decoded_inst.x]);
                    break;
                }
                case 0x29: {
                    // Fx29 - LD F, Vx
                    // Set I = location of sprite for digit Vx.
                    // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
                    chip8->I = 0x0FFF & (0x050 + (5 * chip8->V[decoded_inst.x]));
                    // The 5 comes from the fact that each hexadecimal sprite is 5 bytes long
                    // the 0x050 comes from where they start being stored
                    break;
                }
                case 0x33: {
                    // Fx33 - LD B, Vx
                    // Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    // The interpreter takes the decimal value of Vx,
                    // and places the hundreds digit in memory at location in I,
                    // the tens digit at location I+1, and the ones digit at location I+2.
                    unsigned char value = chip8->V[decoded_inst.x];
                    // method is division, will optimize later (assuming this is even correct)
                    unsigned char ones = value % 10;
                    unsigned char tens = (value / 10) % 10;
                    unsigned char hundreds = (value / 100) % 10;
                    chip8->mem[chip8->I] = hundreds;
                    chip8->mem[chip8->I + 1] = tens;
                    chip8->mem[chip8->I + 2] = ones;
                    break;
                }
                case 0x55: {
                    // Fx55 - LD [I], Vx
                    // Store registers V0 through Vx in memory starting at location I.
                    // The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
                    for (int i = 0; i <= decoded_inst.x; i++) {
                        chip8->mem[chip8->I + i] = chip8->V[i];
                    }
                    break;
                }
                case 0x65: {
                    // Fx65 - LD Vx, [I]
                    // Read registers V0 through Vx from memory starting at location I.
                    // The interpreter reads values from memory starting at location I into registers V0 through Vx.
                    for (int i = 0; i <= decoded_inst.x; i++) {
                        chip8->V[i] = chip8->mem[chip8->I + i];
                    }
                    break;
                }
                default: break;
            }
            break;
        }
        default: break; // currently just ignoring fake instructions (on every default case)
    }
    if (!redirect_pc) chip8->pc += 2;
    // (TODO) maybe differentiate between valid and invalid instructions to help with debugging ?
}


void chip8_tick_timers(Chip_Context *chip8) {
    if (chip8->delay > 0) chip8->delay--;
    if (chip8->sound > 0) chip8->sound--;
}
