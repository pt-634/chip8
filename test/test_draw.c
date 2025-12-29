#include "chip8.h"
#include "test.h"

int test_draw() {
    Chip_Context chip;
    int keypad[16] = {0};

    // Sprite
    unsigned char sprite[] = { 0b11100110, 0b11111111 };

    // --------------------------------------------------
    // 1. Byte spillover
    // --------------------------------------------------
    chip8_reset(&chip);
    chip.I = 0x300;
    chip.mem[0x300] = sprite[0];

    chip.V[0] = 12; // x // halfway through a byte
    chip.V[1] = 10; // y // row 10

    chip.pc = 0x200;
    chip.mem[0x200] = 0xD0; // DRW V0, V1, 1
    chip.mem[0x201] = 0x11;

    chip8_step(&chip, keypad);
    
    // 1110 0110
    ASSERT(chip.screen_buffer[10][1] == 0b00001110, "Byte spillover DRW failed (left byte)"); // Vx / 8 = 1
    ASSERT(chip.screen_buffer[10][2] == 0b01100000, "Byte spillover DRW failed (right byte)");
    ASSERT(chip.V[0xF] == 0, "VF should be 0 when no collision");

    // --------------------------------------------------
    // 2. Horizontal wrap screen
    // --------------------------------------------------
    chip8_reset(&chip);
    chip.I = 0x300;
    chip.mem[0x300] = sprite[0];

    chip.V[0] = 62; // near right edge
    chip.V[1] = 5; // row 5

    chip.pc = 0x200;
    chip.mem[0x200] = 0xD0; // DRW V0, V1, 1
    chip.mem[0x201] = 0x11;

    chip8_step(&chip, keypad);

    // 11 100110
    ASSERT(chip.screen_buffer[5][7] == 0b00000011, "Horizontal wrap failed (right edge byte)"); // 62 / 8 = 7
    ASSERT(chip.screen_buffer[5][0] == 0b10011000, "Horizontal wrap failed (left edge byte)");

    // --------------------------------------------------
    // 3. Vertical wrap screen
    // --------------------------------------------------
    chip8_reset(&chip);
    chip.I = 0x300;
    chip.mem[0x300] = sprite[0];
    chip.mem[0x301] = sprite[1];

    chip.V[0] = 40; // no spillover
    chip.V[1] = 31; // bottom row

    chip.pc = 0x200;
    chip.mem[0x200] = 0xD0; // DRW V0, V1, 2
    chip.mem[0x201] = 0x12;

    chip8_step(&chip, keypad);

    // 11100110
    // 11111111
    ASSERT(chip.screen_buffer[31][5] == 0b11100110, "Vertical wrap failed (bottom edge byte)"); // 40/8=5
    ASSERT(chip.screen_buffer[0][5] == 0b11111111, "Vertical wrap failed (top edge byte)");

    // --------------------------------------------------
    // 4. Collision detection (pixel erased)
    // --------------------------------------------------
    chip8_reset(&chip);
    chip.I = 0x300;
    chip.mem[0x300] = sprite[1];

    chip.V[0] = 0;
    chip.V[1] = 0;

    // Pre-fill pixel to force collision
    chip.screen_buffer[0][0] = 0b00010000;

    chip.pc = 0x200;
    chip.mem[0x200] = 0xD0; // DRW V0, V1, 1
    chip.mem[0x201] = 0x11;

    chip8_step(&chip, keypad);

    // 11111111
    ASSERT(chip.screen_buffer[0][0] == 0b11101111, "Pixel should be erased on collision");
    ASSERT(chip.V[0xF] == 1, "VF should be set on collision");

    return 0;
}

// useful for manual/visual tests
void chip8_print_screen(const Chip_Context *chip) {
    for (int y = 0; y < 32; y++) {
        for (int byte = 0; byte < 8; byte++) {
            unsigned char b = chip->screen_buffer[y][byte];

            // print bits MSB → LSB (left → right)
            for (int bit = 7; bit >= 0; bit--) {
                putchar((b & (1 << bit)) ? '1' : '0');
            }
        }
        putchar('\n');
    }
}



void test_monroe() {
    Chip_Context chip;
    int keypad[16] = {0};

    chip8_reset(&chip);

    // -------------------------
    // 16 chunks (8x15) + coords
    // from the MIDDLE 30 rows (i.e., rows 4..33 of the 38-row input)
    // -------------------------
    int x1 = 0,  y1 = 0;
    unsigned char chunk1[15]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE};

    int x2 = 8,  y2 = 0;
    unsigned char chunk2[15]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xF8, 0xF8, 0xF8, 0xE0, 0x88, 0x80, 0x80, 0x00, 0x00};

    int x3 = 16, y3 = 0;
    unsigned char chunk3[15]  = {0xFF, 0xFF, 0xF8, 0xF0, 0xE0, 0x02, 0x10, 0x08, 0x18, 0x21, 0x80, 0x03, 0x41, 0xF9, 0xFF};

    int x4 = 24, y4 = 0;
    unsigned char chunk4[15]  = {0xFE, 0xC0, 0x04, 0x20, 0x0F, 0x1F, 0x3F, 0x20, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x3E, 0xC7};

    int x5 = 32, y5 = 0;
    unsigned char chunk5[15]  = {0x03, 0x21, 0x00, 0x9E, 0x85, 0xA0, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0E};

    int x6 = 40, y6 = 0;
    unsigned char chunk6[15]  = {0xFF, 0xFF, 0x1F, 0x03, 0x00, 0x80, 0xC0, 0x00, 0x00, 0x02, 0x00, 0x00, 0x61, 0x0B, 0x6B};

    int x7 = 48, y7 = 0;
    unsigned char chunk7[15]  = {0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x0F, 0x17, 0x07, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xF2, 0xE7};

    int x8 = 56, y8 = 0;
    unsigned char chunk8[15]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF};

    int x9 = 0,  y9 = 15;
    unsigned char chunk9[15]  = {0xFC, 0xFC, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    int x10 = 8,  y10 = 15;
    unsigned char chunk10[15] = {0x04, 0x50, 0x1F, 0x9F, 0x07, 0x87, 0xFF, 0xE7, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    int x11 = 16, y11 = 15;
    unsigned char chunk11[15] = {0x7F, 0x3E, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    int x12 = 24, y12 = 15;
    unsigned char chunk12[15] = {0xF5, 0xF0, 0x00, 0x00, 0x03, 0x80, 0x80, 0x8F, 0xC3, 0xE0, 0xE0, 0xF8, 0xFF, 0xFF, 0xFF};

    int x13 = 32, y13 = 15;
    unsigned char chunk13[15] = {0x06, 0x03, 0x00, 0x00, 0xF0, 0x00, 0x00, 0xFC, 0x08, 0xC0, 0x00, 0x03, 0xFF, 0xFF, 0xFF};

    int x14 = 40, y14 = 15;
    unsigned char chunk14[15] = {0xFB, 0x41, 0x03, 0x01, 0x04, 0x01, 0x06, 0x0F, 0x0B, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    int x15 = 48, y15 = 15;
    unsigned char chunk15[15] = {0x25, 0x64, 0xC0, 0xC0, 0xA0, 0x41, 0x25, 0xC0, 0xC1, 0xE7, 0xFF, 0x9F, 0x3F, 0xFF, 0xFF};

    int x16 = 56, y16 = 15;
    unsigned char chunk16[15] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // -------------------------
    // Load all chunk sprites into memory contiguously
    // (each sprite is 15 bytes tall)
    // -------------------------
    const int base = 0x300;

    unsigned char *chunks[16] = {
        chunk1, chunk2, chunk3, chunk4, chunk5, chunk6, chunk7, chunk8,
        chunk9, chunk10, chunk11, chunk12, chunk13, chunk14, chunk15, chunk16
    };

    int xs[16] = {x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16};
    int ys[16] = {y1,y2,y3,y4,y5,y6,y7,y8,y9,y10,y11,y12,y13,y14,y15,y16};

    for (int i = 0; i < 16; i++) {
        for (int r = 0; r < 15; r++) {
            chip.mem[base + i*15 + r] = chunks[i][r];
        }
    }

    // DRW V0, V1, 0xF (15 rows)
    chip.mem[0x200] = 0xD0;
    chip.mem[0x201] = 0x1F;

    // -------------------------
    // Draw each chunk at its (x,y)
    // -------------------------
    for (int i = 0; i < 16; i++) {
        chip.I = (unsigned short)(base + i*15);
        chip.V[0] = (unsigned char)xs[i];
        chip.V[1] = (unsigned char)ys[i];

        chip.pc = 0x200;
        chip8_step(&chip, keypad);
    }

    printf("Manual DRW test (Monroe middle-30-rows via 16 sprites of 8x15):\n");
    chip8_print_screen(&chip);

    /*
     *
     *
     * Reference
     *
        1111111111111111111111111111111111111111111111111111111111111111
        1111111111111111111111111111111111111111111111111111111111111111
        1111111111111111111111111111111111111111111111111111111111111111
        1111111111111111111111111111111111111111111111111111111111111111
        1111111111111111111111111111111000000011111111111111111111111111
        1111111111111111111111111100000000100001111111111111111111111111
        1111111111111111111110000000010000000000000111111111111111111111
        1111111111111111111100000010000010011110000000111111111111111111
        1111111111111111111000000000111110000101000000000111111111111111
        1111111111111100000000100001111110100000100000000000111111111111
        1111111111111000000100000011111111111110110000000001011111111111
        1111111111111000000010000010000000000000000000000000011111111111
        1111111111111000000110000000000000000000000000000000000011111111
        1111111111100000001000010000000000000000000000100000000011111111
        1111111110001000100000000000000000000000000000000000000011111111
        1111111110000000000000111000000000000000000000000000000011111111
        1111111110000000010000011100000000000000011000011110000001111111
        1111111100000000111110010011111000000110000010111111001011111111
        1111111000000000111111111100011100001110011010111110011111111111
        1111110000000100011111111111010100000110111110110010010111111111
        1111110001010000001111101111000000000011010000010110010011111111
        1111110000011111111111100000000000000000000000111100000011111111
        1111111010011111111111100000000000000000000000011100000011111111
        1111111100000111111111110000001111110000000001001010000011111111
        1111111110000111111111111000000000000000000000010100000111111111
        1111111111111111111111111000000000000000000001100010010111111111
        1111111111100111111111111000111111111100000011111100000011111111
        1111111111111110111111111100001100001000000010111100000111111111
        1111111111111111111111111110000011000000000111111110011111111111
        1111111111111111111111111110000000000000111111111111111111111111
        1111111111111111111111111111100000000011111111111001111111111111
        1111111111111111111111111111111111111111111111110011111111111111
        1111111111111111111111111111111111111111111111111111111111111111
        1111111111111111111111111111111111111111111111111111111111111111
        1111111111111111111111111111111111111111111111111111111111111111
        1111111111111111111111111111111111111111111111111111111111111111
        1111111111111111111111111111111111111111111111111111111101010111
        1111111111111111111111111111111111111111111111111111111111111111
     *
     *
     */

}




int main() {
    printf("testing draw... ");
    if (test_draw() == 0) {
        printf("PASSED\n");

        printf("\n\n");
        test_monroe();
        printf("\n\n");

        return 0;
    } else {
        return 1;
    }
}

