#include "chip8.h"
#include "test.h"

int test_step() {
    Chip_Context chip;
    int keypad[16] = {0};

    // reset chip
    chip8_reset(&chip);

    // -----------------------
    // 00E0 - CLS
    // -----------------------
    chip.pc = 0x200;
    chip.mem[0x200] = 0x00;
    chip.mem[0x201] = 0xE0;
    chip.screen_buffer[0][0] = 0xFF; // pixel on
    chip8_step(&chip, keypad);
    ASSERT(chip.screen_buffer[0][0] == 0, "CLS failed");
    ASSERT(chip.pc == 0x202, "PC not incremented after CLS");

    // -----------------------
    // 00EE - RET
    // -----------------------
    chip.pc = 0x202;
    chip.sp = 1;
    chip.stack[0] = 0x300;
    chip.mem[0x202] = 0x00;
    chip.mem[0x203] = 0xEE;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x300, "RET failed");
    ASSERT(chip.sp == 0, "SP not decremented on RET");

    // -----------------------
    // 1nnn - JP addr
    // -----------------------
    chip.pc = 0x200;
    chip.mem[0x200] = 0x12; // JP 0x2C0
    chip.mem[0x201] = 0xC0; 
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x2C0, "JP 0x2C0 failed");

    // -----------------------
    // 2nnn - CALL addr
    // -----------------------
    chip.pc = 0x200;
    chip.sp = 0;
    chip.mem[0x200] = 0x23; // CALL 0x300
    chip.mem[0x201] = 0x00;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x300, "CALL failed");
    ASSERT(chip.stack[0] == 0x202, "CALL did not push PC+2 to stack");
    ASSERT(chip.sp == 1, "CALL did not increment SP");

    // -----------------------
    // 3xkk - SE Vx, byte
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 0x42;
    chip.mem[0x200] = 0x31; // SE V1, 0x42
    chip.mem[0x201] = 0x42;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x204, "SE Vx, byte failed to skip when equal");

    chip.pc = 0x200;
    chip.V[1] = 0x41;
    chip.mem[0x200] = 0x31; // SE V1, 0x42
    chip.mem[0x201] = 0x42;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x202, "SE Vx, byte incorrectly skipped when not equal");

    // -----------------------
    // 4xkk - SNE Vx, byte
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 0x41;
    chip.mem[0x200] = 0x41; // SNE V1, 0x42
    chip.mem[0x201] = 0x42;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x204, "SNE Vx, byte failed to skip when not equal");

    chip.pc = 0x200;
    chip.V[1] = 0x42;
    chip.mem[0x200] = 0x41; // SNE V1, 0x42
    chip.mem[0x201] = 0x42;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x202, "SNE Vx, byte incorrectly skipped when equal");

    // -----------------------
    // 5xy0 - SE Vx, Vy
    // -----------------------
    chip.pc = 0x200;
    chip.V[2] = 0x55;
    chip.V[3] = 0x55;
    chip.mem[0x200] = 0x52; // 5 2 3 0: SE V2, V3
    chip.mem[0x201] = 0x30;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x204, "SE Vx, Vy failed to skip when equal");

    chip.pc = 0x200;
    chip.V[2] = 0x54;
    chip.V[3] = 0x55;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x202, "SE Vx, Vy incorrectly skipped when not equal");

    // -----------------------
    // 6xkk - LD Vx, byte
    // -----------------------
    chip.pc = 0x200;
    chip.mem[0x200] = 0x6A; // LD VA, 0x99
    chip.mem[0x201] = 0x99;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[0xA] == 0x99, "LD Vx, byte failed");
    ASSERT(chip.pc == 0x202, "PC not incremented after LD Vx, byte");

    // -----------------------
    // 7xkk - ADD Vx, byte
    // -----------------------
    chip.pc = 0x200;
    chip.V[0xA] = 0x10;
    chip.mem[0x200] = 0x7A; // ADD VA, 0x05
    chip.mem[0x201] = 0x05;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[0xA] == 0x15, "ADD Vx, byte failed");
    ASSERT(chip.pc == 0x202, "PC not incremented after ADD Vx, byte");

    // -----------------------
    // 8xy0 - LD Vx, Vy
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 0x42;
    chip.V[0xE]= 0x24;
    chip.mem[0x200] = 0x81; // 8 1 E 0: LD V1, VE
    chip.mem[0x201] = 0xE0;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0x24, "LD Vx, Vy failed");
    ASSERT(chip.pc == 0x202, "PC not incremented after LD Vx, Vy");

    // -----------------------
    // 8xy1 - OR Vx, Vy
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 0b1010;
    chip.V[2] = 0b1100;
    chip.mem[0x200] = 0x81; // 8 1 2 1: OR V1, V2
    chip.mem[0x201] = 0x21;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0b1110, "OR Vx, Vy failed");

    // -----------------------
    // 8xy2 - AND Vx, Vy
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 0b1010;
    chip.V[2] = 0b1100;
    chip.mem[0x200] = 0x81; // 8 1 2 2: AND V1, V2
    chip.mem[0x201] = 0x22;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0b1000, "AND Vx, Vy failed");

    // -----------------------
    // 8xy3 - XOR Vx, Vy
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 0b1010;
    chip.V[2] = 0b1100;
    chip.mem[0x200] = 0x81; // 8 1 2 3: XOR V1, V2
    chip.mem[0x201] = 0x23;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0b0110, "XOR Vx, Vy failed");

    // -----------------------
    // 8xy4 - ADD Vx, Vy (with carry)
    // -----------------------
    // no carry
    chip.pc = 0x200;
    chip.V[1] = 10;
    chip.V[2] = 20;
    chip.mem[0x200] = 0x81; // ADD V1, V2
    chip.mem[0x201] = 0x24;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 30, "ADD Vx, Vy failed (no carry)");
    ASSERT(chip.V[0xF] == 0, "VF should be 0 when no carry");

    // with carry
    chip.pc = 0x200;
    chip.V[1] = 200;
    chip.V[2] = 100;
    chip.mem[0x200] = 0x81;
    chip.mem[0x201] = 0x24;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == ((200+100)&0xFF), "ADD Vx, Vy failed (carry)"); // (200+100) & 0xFF = 300 % 256 = 44
    ASSERT(chip.V[0xF] == 1, "VF should be 1 when carry occurs");

    // -----------------------
    // 8xy5 - SUB Vx, Vy
    // -----------------------
    // no borrow
    chip.pc = 0x200;
    chip.V[1] = 0x03;
    chip.V[2] = 0x01;
    chip.mem[0x200] = 0x81; // SUB V1, V2
    chip.mem[0x201] = 0x25;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0x02, "SUB Vx, Vy failed (no borrow)");
    ASSERT(chip.V[0xF] == 1, "VF should be 1 when no borrow");

    // borrow
    chip.pc = 0x200;
    chip.V[1] = 0x01;
    chip.V[2] = 0x03;
    chip.mem[0x200] = 0x81;
    chip.mem[0x201] = 0x25;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0xFE, "SUB Vx, Vy failed (borrow)");
    ASSERT(chip.V[0xF] == 0, "VF should be 0 when borrow occurs");

    // -----------------------
    // 8xy6 - SHR Vx
    // -----------------------
    // lsb 1
    chip.pc = 0x200;
    chip.V[1] = 0b10000011;
    chip.mem[0x200] = 0x81;
    chip.mem[0x201] = 0x26;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0b01000001, "SHR Vx failed (lsb 1)");
    ASSERT(chip.V[0xF] == 1, "VF should contain LSB before shift (1)");
    
    // lsb 0
    chip.pc = 0x200;
    chip.V[1] = 0b00000010;
    chip.mem[0x200] = 0x81;
    chip.mem[0x201] = 0x26;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0b00000001, "SHR Vx failed (lsb 0)");
    ASSERT(chip.V[0xF] == 0, "VF should contain LSB before shift (0)");

    // -----------------------
    // 8xy7 - SUBN Vx, Vy
    // -----------------------
    // no borrow
    chip.pc = 0x200;
    chip.V[1] = 0x01;
    chip.V[2] = 0x03;
    chip.mem[0x200] = 0x81; // SUBN V1, V2
    chip.mem[0x201] = 0x27;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0x02, "SUBN failed (no borrow)");
    ASSERT(chip.V[0xF] == 1, "VF should be 1 when no borrow");

    // borrow
    chip.pc = 0x200;
    chip.V[1] = 0x03;
    chip.V[2] = 0x01;
    chip.mem[0x200] = 0x81;
    chip.mem[0x201] = 0x27;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0xFE, "SUBN failed (borrow)");
    ASSERT(chip.V[0xF] == 0, "VF should be 0 when borrow occurs");

    // -----------------------
    // 8xyE - SHL Vx
    // -----------------------
    // msb 1
    chip.pc = 0x200;
    chip.V[1] = 0b10000001;
    chip.mem[0x200] = 0x81;
    chip.mem[0x201] = 0x2E;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0b00000010, "SHL Vx failed (msb 1)");
    ASSERT(chip.V[0xF] == 1, "VF should contain MSB before shift (1)");
    
    // msb 0
    chip.pc = 0x200;
    chip.V[1] = 0b01000001;
    chip.mem[0x200] = 0x81;
    chip.mem[0x201] = 0x2E;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 0b10000010, "SHL Vx failed (msb 0)");
    ASSERT(chip.V[0xF] == 0, "VF should contain MSB before shift (0)");

    // -----------------------
    // 9xy0 - SNE Vx, Vy
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 1;
    chip.V[2] = 2;
    chip.mem[0x200] = 0x91;
    chip.mem[0x201] = 0x20;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x204, "SNE Vx, Vy failed to skip when not equal");

    chip.pc = 0x200;
    chip.V[1] = 2;
    chip.V[2] = 2;
    chip.mem[0x200] = 0x91;
    chip.mem[0x201] = 0x20;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x202, "SNE Vx, Vy incorrectly skipped when equal");

    // -----------------------
    // Annn - LD I, addr
    // -----------------------
    chip.pc = 0x200;
    chip.mem[0x200] = 0xAB;
    chip.mem[0x201] = 0xBB;
    chip8_step(&chip, keypad);
    ASSERT(chip.I == 0x0BBB, "LD I, addr failed");

    // -----------------------
    // Bnnn - JP V0, addr
    // -----------------------
    chip.pc = 0x200;
    chip.V[0] = 0x10;
    chip.mem[0x200] = 0xB2;
    chip.mem[0x201] = 0x00;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x210, "JP V0, addr failed");

    // -----------------------
    // Cxkk - RND Vx, byte
    // -----------------------
    chip.pc = 0x200;
    chip.mem[0x200] = 0xC1;
    chip.mem[0x201] = 0x0F;
    chip8_step(&chip, keypad);
    //ASSERT(chip.V[1] == 0x0F, "RND Vx, byte failed (TODO - update test)");

    // -----------------------
    // Ex9E - SKP Vx
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 5;
    keypad[5] = 1;
    chip.mem[0x200] = 0xE1;
    chip.mem[0x201] = 0x9E;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x204, "SKP Vx failed (should skip when key pressed)");
    
    chip.pc = 0x200;
    keypad[5] = 0;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x202, "SKP Vx failed (should not skip if key not pressed)");

    // -----------------------
    // ExA1 - SKNP Vx
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 5;
    keypad[5] = 1;
    chip.mem[0x200] = 0xE1;
    chip.mem[0x201] = 0xA1;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x202, "SKNP Vx failed (should not skip when key pressed)");

    chip.pc = 0x200;
    keypad[5] = 0;
    chip8_step(&chip, keypad);
    ASSERT(chip.pc == 0x204, "SKNP Vx failed (should skip when key not pressed)");

    // -----------------------
    // Fx07 - LD Vx, DT
    // -----------------------
    chip.pc = 0x200;
    chip.delay = 42;
    chip.mem[0x200] = 0xF1;
    chip.mem[0x201] = 0x07;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[1] == 42, "LD Vx, DT failed");

    // -----------------------
    // Fx15 - LD DT, Vx
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 60;
    chip.mem[0x200] = 0xF1;
    chip.mem[0x201] = 0x15;
    chip8_step(&chip, keypad);
    ASSERT(chip.delay == 60, "LD DT, Vx failed");

    // -----------------------
    // Fx18 - LD ST, Vx
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 30;
    chip.mem[0x200] = 0xF1;
    chip.mem[0x201] = 0x18;
    chip8_step(&chip, keypad);
    ASSERT(chip.sound == 30, "LD ST, Vx failed");

    // -----------------------
    // Fx1E - ADD I, Vx
    // -----------------------
    chip.pc = 0x200;
    chip.I = 0xFFF;
    chip.V[1] = 0x32;
    chip.mem[0x200] = 0xF1;
    chip.mem[0x201] = 0x1E;
    chip8_step(&chip, keypad);
    ASSERT(chip.I == 0x31, "ADD I, Vx failed");

    // -----------------------
    // Fx29 - LD F, Vx
    // -----------------------
    chip.pc = 0x200;
    chip.V[1] = 5;
    chip.mem[0x200] = 0xF1;
    chip.mem[0x201] = 0x29;
    chip8_step(&chip, keypad);
    ASSERT(chip.I == (0x050 + 5 * 5), "LD F, Vx failed");

    // -----------------------
    // Fx33 - LD B, Vx
    // -----------------------
    chip.pc = 0x200;
    chip.I = 300;
    chip.V[1] = 123;
    chip.mem[0x200] = 0xF1;
    chip.mem[0x201] = 0x33;
    chip8_step(&chip, keypad);
    ASSERT(chip.mem[300] == 1, "BCD hundreds incorrect");
    ASSERT(chip.mem[301] == 2, "BCD tens incorrect");
    ASSERT(chip.mem[302] == 3, "BCD ones incorrect");

    // -----------------------
    // Fx55 - LD [I], Vx
    // -----------------------
    chip.pc = 0x200;
    chip.I = 400;
    chip.V[0] = 1;
    chip.V[1] = 2;
    chip.V[2] = 3;
    chip.mem[0x200] = 0xF2;
    chip.mem[0x201] = 0x55;
    chip8_step(&chip, keypad);
    ASSERT(chip.mem[400] == 1, "LD [I], Vx failed");
    ASSERT(chip.mem[401] == 2, "LD [I], Vx failed");
    ASSERT(chip.mem[402] == 3, "LD [I], Vx failed");

    // -----------------------
    // Fx65 - LD Vx, [I]
    // -----------------------
    chip.pc = 0x200;
    chip.I = 500;
    chip.mem[500] = 7;
    chip.mem[501] = 8;
    chip.mem[502] = 9;
    chip.mem[0x200] = 0xF2;
    chip.mem[0x201] = 0x65;
    chip8_step(&chip, keypad);
    ASSERT(chip.V[0] == 7, "LD Vx, [I] failed");
    ASSERT(chip.V[1] == 8, "LD Vx, [I] failed");
    ASSERT(chip.V[2] == 9, "LD Vx, [I] failed");


    return 0;
}

int main() {
    printf("testing step... ");
    if (test_step() == 0) {
        printf("PASSED\n");
        return 0;
    } else {
        return 1;
    }
}

