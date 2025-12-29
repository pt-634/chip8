#include "chip8.h"
#include "test.h"

int test_reset() {
    Chip_Context chip;

    // write to certain spots in mem, check if overwritten
    chip.mem[0] = 0xFF;
    chip.mem[0x050] = 0x13;
    chip.V[0] = 0xFF;
    chip.I = 0xFFFF;
    chip.delay = 5;
    chip.sound = 5;
    chip.pc = 0x300;
    chip.sp = 5;
    chip.stack[0] = 0xAAAA;
    chip.screen_buffer[0][0] = 0xFF;

    chip8_reset(&chip);

    ASSERT(chip.mem[0] == 0, "chip memory not zeroed out");
    ASSERT(chip.mem[0x050] == 0xF0, "fontset not written correctly");
    ASSERT(chip.V[0] == 0, "registers not zeroed out correctly");
    ASSERT(chip.I == 0, "register I not zeroed out correctly");
    ASSERT(chip.delay == 0, "delay not zeroed correctly");
    ASSERT(chip.sound == 0, "sound not zeroed correctly");
    ASSERT(chip.pc == 0, "pc not reset correctly");
    ASSERT(chip.sp == 0, "sp not reset correctly");
    ASSERT(chip.screen_buffer[0][0] == 0, "screen buffer not wiped");

    return 0; // 0 means test passed
}

int main() {
    printf("testing reset... ");
    if (test_reset() == 0) {
        printf("PASSED\n");
        return 0;
    } else {
        return 1;
    }
}
