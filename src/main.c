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

int emulate(const char *rom_path, const char *font_path, u8 debug);
int assemble(const char *path);

int main(int argc, char *argv[])
{
    // Parse command line args
    if (argc > 1)
    {
        if (strcmp(argv[1], "emulate") == 0)
        {
            if (argc == 3)
            {
                const char *rom_path = argv[2];
                return emulate(rom_path, "fonts/default.font", 0);
            }
            else if (argc == 4)
            {
                const char *rom_path = argv[2];
                const char *font_path = argv[3];
                return emulate(rom_path, font_path, 0);
            }
            else
            {
                printf("Usage: chip8 emulate <rom> <font:optional>\n");
                return 1;
            }
        }
        else if (strcmp(argv[1], "emulate-debug") == 0)
        {
            if (argc == 3)
            {
                const char *rom_path = argv[2];
                return emulate(rom_path, "fonts/default.font", 1);
            }
            else if (argc == 4)
            {
                const char *rom_path = argv[2];
                const char *font_path = argv[3];
                return emulate(rom_path, font_path, 1);
            }
            else
            {
                printf("Usage: chip8 emulate-debug <rom> <font:optional>\n");
                return 1;
            }
        }
        else if (strcmp(argv[1], "assemble") == 0)
        {
            if (argc == 3)
            {
                const char *assemble_path = argv[2];
                return assemble(assemble_path);
            }
            else
            {
                printf("Usage: chip8 assemble <filename>\n");
            }
        }
    }
    printf("Usage:\n\tEmulator: chip8 emulate <filename> <font:optional>\n\tEmulator with debug: chip8 emulate-debug <filename> <font:optional>\n\tAssembler: chip8 assemble <filename>");
    return 1;
}

int emulate(const char *rom_path, const char *font_path, u8 debug)
{
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
            if (debug)
            {
                if (!debug_instruction(&state, &instruction))
                {
                    state.halt = 1;
                }
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
    }

    shutdown_graphics();
    return 0;
}

int assemble(const char *path)
{
    printf("Cannot assemble \"%s\", feature not implemented yet\n", path);
}