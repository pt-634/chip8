#include "chip8.h"
#include "test.h"

int test_load_rom() {
    Chip_Context chip;

    // assumes correct reset functionality
    chip8_reset(&chip);

    // dummy rom
    unsigned char rom[] = {0xAA, 0xBB, 0xCC, 0xDD};
    chip8_load_rom(&chip, rom, 4);
   
    ASSERT(chip.pc == 0x200, "pc not set to 0x200 on rom load");
    ASSERT(chip.mem[0x200] == 0xAA, "expected 0xAA at 0x200");
    ASSERT(chip.mem[0x201] == 0xBB, "expected 0xBB at 0x201");
    ASSERT(chip.mem[0x202] == 0xCC, "expected 0xCC at 0x202");
    ASSERT(chip.mem[0x203] == 0xDD, "expected 0xDD at 0x203");

    // invalid rom size test (max length is 0xFFF - 0x200)
    ASSERT(chip8_load_rom(&chip, rom, 0xFFF) == 1, "expected error on loading rom");

    return 0; // 0 means test passed
}

int main() {
    printf("testing load rom... ");
    if (test_load_rom() == 0) {
        printf("PASSED\n");
        return 0;
    } else {
        return 1;
    }
}
