// https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

#include "types.h"
#include "instructions.h"
#include "chip8.h"
#include "graphics.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct chip8 state;

int main(int argc, char *argv[])
{
    // Parse command line args
    if (argc != 2)
    {
        printf("Usage: chip8 <filename>\n");
        return 0;
    }
    const char *rom_path = argv[1];

    // Init CPU
    init_chip8(&state);

    // Load rom into memory at location 0x200
    printf("Loading rom: \"%s\"\n", rom_path);

    FILE *rom_file = fopen(rom_path, "rb");
    
    fseek(rom_file, 0, SEEK_END);
    int rom_size = (int)ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET);
    
    if (rom_size - 512 > MEMORY_SIZE)
    {
        printf("Rom size is %d bytes which is too large to fit in memory\n", rom_size);
        return 1;
    }

    printf("Rom size is %d bytes, reading into memory\n\n", rom_size);

    fread(&state.memory[0x200], 1, rom_size, rom_file);
    fclose(rom_file);

    // print_memory(0x200, 160, 16);

    // Setup font
    const char *font_path = "fonts/default.font";
    printf("Loading font: %s\n", font_path);

    FILE *font_file = fopen(font_path, "rb");

    fseek(font_file, 0, SEEK_END);
    int font_size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);

    if (font_size < 80)
    {
        printf("Font file size is %d bytes, it should be 80 bytes\n", font_size);
        return 1;
    }
    else if (font_size > 80)
    {
        printf("Font file size is %d bytes, taking first 80 bytes\n", font_size);
    }
    printf("\n");

    fread(&state.memory[0x50], 1, 80, font_file);
    fclose(font_file);

    // print_memory(0x50, 80, 5);

    // Init graphics
    init_graphics();

    // Start emulation
    u8 loop = 1;
    u16 instruction_bytes;
    struct instruction instruction;
    while (loop)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                loop = 0;
                break;
            }
        }
        if (!loop) break;

        // Emulate
        if (!state.halt)
        {
            fetch_instruction(&state, &instruction_bytes);
            decode_instruction(instruction_bytes, &instruction);
            if (!execute_instruction(&state, &instruction))
            {
                state.halt = 1;
            }
            if (!debug_instruction(&state, &instruction))
            {
                state.halt = 1;
            }
            render_screen(&state);
            state.cycles++;

            // print_cpu(&state);

            // Hardcoded to stop after 100 cycles
            if (state.cycles > 100)
            {
                state.halt = 1;
            }
        }


        // if (instruction_bytes == 0x1228) return 0; // TEMP
        // if (instruction_bytes == 0xFFFF) return 0; // TEMP
    }

    // SDL_Delay(3000);
    shutdown_graphics();
    return 0;
}