#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 10

#define CPU_HZ   540
#define TIMER_HZ 60
#define INSTRS_PER_TIMER (CPU_HZ / TIMER_HZ)  // 9

// colors (color = 0x@@$$&&??, where @@ is transparency, $$ is red, && is green, ?? is blue)
// default pair
#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000
#define DEFAULT WHITE : BLACK
// mixed greens
#define PEA_GREEN 0xFF9BBC0F
#define DARK_OLIVE 0xFF0F380F
#define MIXED_GREENS PEA_GREEN : DARK_OLIVE
// cyberpunk
#define ELECTRIC_CYAN 0xFF00FFFF
#define PURPLE 0xFF6F00FF
#define CYBERPUNK ELECTRIC_CYAN : PURPLE
// corrosion
#define TOXIC_TEAL 0xFF27B481
#define BURNT_ORANGE 0xFFED453D
#define CORROSION TOXIC_TEAL : BURNT_ORANGE


// file size stuff (need this for le load_rom)
long get_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);
    return filesize;
}

// consider changing to a lookup table to speed this up
void convert_pixels(unsigned int pixel_buff[SCREEN_HEIGHT][SCREEN_WIDTH], unsigned char screen_buff[SCREEN_HEIGHT][SCREEN_WIDTH/8]) {
    for (int y = 0; y < 32; y++) {
        for (int x_byte = 0; x_byte < 8; x_byte++) {
            unsigned char byte = screen_buff[y][x_byte];
            for (int bit = 0; bit < 8; bit++) {
                int x = x_byte * 8 + (7 - bit); // 7-bit because highest bit is leftmost
                pixel_buff[y][x] = (byte & (1 << bit)) ? CYBERPUNK; // DEFAULT is default
            }
        }
    }
}

// actually blit the thing to the screen and refresh it
void refresh(SDL_Window *window, SDL_Surface *surface) {
    SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    SDL_BlitScaled(surface, NULL, window_surface, NULL);
    SDL_UpdateWindowSurface(window);
}

// debugging
void chip8_dump_state(const Chip_Context *chip) {
    printf("================================ CHIP-8 STATE ================================\n");

    // PC + current instruction
    unsigned short pc = chip->pc;
    unsigned short inst = (chip->mem[pc] << 8) | chip->mem[pc + 1];

    printf("PC   : 0x%03X\n", pc);
    printf("INST : 0x%04X\n", inst);

    // Registers
    printf("\nREGISTERS:\n");
    for (int i = 0; i < 16; i++) {
        printf("V%X: %02X  ", i, chip->V[i]);
        if (i == 7) printf("\n");  // split into two rows
    }
    printf("\n");

    // Special registers
    printf("\nI     : 0x%03X\n", chip->I);
    printf("DT    : %u\n", chip->delay);
    printf("ST    : %u\n", chip->sound);

    // Stack
    printf("\nSTACK (sp = %u):\n", chip->sp);
    if (chip->sp == 0) {
        printf("  <empty>\n");
    } else {
        for (int i = 0; i < chip->sp; i++) {
            printf("  [%02d] 0x%03X\n", i, chip->stack[i]);
        }
    }

    printf("===============================================================================\n\n");
}


int main(int argc, char **argv) {

    // grab rom file
    if (argc != 2) {
        printf("Usage: ./start romname\n");
        exit(1);
    }
    const char *romname = argv[1];
    FILE *romfile = fopen(romname, "rb");
    if (romfile == NULL) {
        printf("Error: unable to open file %s\n", romname);
        exit(1);
    }
    long romsize = get_size(romfile);
    unsigned char *rom = malloc(romsize);
    if (rom == NULL) {
        printf("Error: no memory\n");
        exit(1);
    }
    fread(rom, 1, romsize, romfile);
    fclose(romfile);

    // initialize chip context and load rom
    Chip_Context chip;
    int keypad[16] = {0};
    chip8_reset(&chip);
    chip8_load_rom(&chip, rom, romsize);

    // initialize window
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        // i dont concretely get what these flags are for but not doing SDL_Init can cause problems apparently
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Window *game_window = SDL_CreateWindow(romname, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH*SCALE, SCREEN_HEIGHT*SCALE, 0);
    unsigned int sdl_pixels[SCREEN_HEIGHT][SCREEN_WIDTH]; // needed to change the structure a bit to account for how SDL treats pixels
    SDL_Surface *surface_buff = SDL_CreateRGBSurfaceFrom(
        sdl_pixels,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        32,                        // 32 bits per pixel
        SCREEN_WIDTH * sizeof(unsigned int), // bytes per row
        0x00FF0000,
        0x0000FF00,
        0x000000FF,
        0xFF000000
    );

    // initialize timer stuff
    int instrs_until_timer = INSTRS_PER_TIMER;

    // initialize audio stuff
    //SDL_AudioDeviceID audio_dev;

    int running = 1;
    SDL_Event e;
    while (running) {

        // debugging
        //chip8_dump_state(&chip);
        //SDL_Delay(4); // just did this to slow the game down and debug issues

        // events (window closing / keypresses / rom resetting)
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                int pressed = (e.type == SDL_KEYDOWN);
                SDL_Keycode key = e.key.keysym.sym;
                // chip8 keypad is a 4x4 grid of keys. going to map the structure to fit the average keyboard decently
                switch (key) {
                    case SDLK_1: keypad[0x1] = pressed; break;
                    case SDLK_2: keypad[0x2] = pressed; break;
                    case SDLK_3: keypad[0x3] = pressed; break;
                    case SDLK_4: keypad[0xC] = pressed; break;

                    case SDLK_q: keypad[0x4] = pressed; break;
                    case SDLK_w: keypad[0x5] = pressed; break;
                    case SDLK_e: keypad[0x6] = pressed; break;
                    case SDLK_r: keypad[0xD] = pressed; break;

                    case SDLK_a: keypad[0x7] = pressed; break;
                    case SDLK_s: keypad[0x8] = pressed; break;
                    case SDLK_d: keypad[0x9] = pressed; break;
                    case SDLK_f: keypad[0xE] = pressed; break;

                    case SDLK_z: keypad[0xA] = pressed; break;
                    case SDLK_x: keypad[0x0] = pressed; break;
                    case SDLK_c: keypad[0xB] = pressed; break;
                    case SDLK_v: keypad[0xF] = pressed; break;
                }
            }
        }


        // step game
        chip8_step(&chip, keypad);

        // timer stuff (60Hz timers + 540Hz processor -> 1 timer cycle per 9 inst cycles)
        if (instrs_until_timer == 0) {
            instrs_until_timer = INSTRS_PER_TIMER;
            chip8_tick_timers(&chip);
        }
        instrs_until_timer--;

        // update sound

        // update display
        convert_pixels(sdl_pixels, chip.screen_buffer);
        refresh(game_window, surface_buff);

        // sleep to simulate roughly accurate frequencies (and to avoid CPU hogging)
        usleep(1852);  // 1852 microseconds ≈ (1/540)s (POSIX only i believe, wont work on Windows)
        
    }

    free(rom);
    SDL_Quit();

    return 0;
}

