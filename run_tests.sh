#!/bin/sh

# Create build directory if it doesn't exist
mkdir -p build

# Compile source files
gcc -c src/chip8.c -o build/chip8.o
gcc -c test/test_reset.c -o build/test_reset.o -I src
gcc -c test/test_load_rom.c -o build/test_load_rom.o -I src
gcc -c test/test_step.c -o build/test_step.o -I src
gcc -c test/test_draw.c -o build/test_draw.o -I src

# Link into executable
gcc build/chip8.o build/test_reset.o -o build/test_reset
gcc build/chip8.o build/test_load_rom.o -o build/test_load_rom
gcc build/chip8.o build/test_step.o -o build/test_step
gcc build/chip8.o build/test_draw.o -o build/test_draw

# Run the test
./build/test_reset
./build/test_load_rom
./build/test_step
./build/test_draw

