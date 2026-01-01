## Chip8 Interpreter
- an emulator for a Chip8 interpreter written in C
- contains a chip interface, UI components, and a complete test suite 

#### Requirements
- [SDL2](https://wiki.libsdl.org/SDL2/FrontPage)
- Linux/Mac (see Notes below for details) 

#### Usage
**precompiled binary:**
- run `./ch8_emu roms/tetris.ch8` (`ch8_emu` was compiled with `gcc -O2 src/main.c src/chip8.c -o ch8_emu -lSDL2`)
**build from source:**
- compile src and link SDL2 (e.g `gcc src/main.c src/chip8.c -o build/ch8_emu -lSDL2`)
- run with `./build/ch8_emu roms/tetris.ch8` (or any ch8 rom of your choice)

#### Notes
- (as of 12/31/2025) currently missing sound support
- (as of 12/31/2025) no support for Windows due to `usleep` usage, no current plans to implement a workaround

#### Links
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#keyboard) used as primary source for Chip-8 architecture details
- [Binary Art Generator](https://binarycodeconverter.com/binary-art-generator/) used to generate marylin monroe bitmap 
- [marilyn monroe source photo](https://william-cuccio.pixels.com/featured/marilyn-monroe-black-and-white-pop-art-william-cuccio-aka-wcsmack.html)

