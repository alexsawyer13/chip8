// https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

#include "common/types.h"
#include "common/instructions.h"
#include "common/chip8.h"
#include "common/graphics.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct chip8 state;

struct args
{
    const char *rom_path;
    const char *font_path;
    u8 debug;
};

int emulate(struct args *args);

int main(int argc, char *argv[])
{
    struct args args = {0};
    // Parse command line args
    if (argc >= 2)
    {
        for (int i = 1; i < argc; i++)
        {
            const char *str = argv[i];
            if (str[0] == '-')
            {
                // Parse a flag
                char flag = str[1];
                switch(flag)
                {
                case 'f':
                    if (args.font_path == NULL)
                    {
                        args.font_path = str + 2;
                    }
                    else
                    {
                        printf("-f flag defined twice\n");
                        return 1;
                    }
                    break;
                case 'd':
                    if (args.debug == 0)
                    {
                        args.debug = 1;
                    }
                    else
                    {
                        printf("-d flag defined twice\n");
                        return 1;
                    }
                    break;
                default:
                    printf("Unknown flag: %c\n", flag);
                }
            }
            else
            {
                if (args.rom_path == NULL)
                {
                    args.rom_path = str;
                }
                else
                {
                    printf("Multiple rom paths specified\n");
                    return 1;
                }
            }
        }

        if (args.rom_path == NULL)
        {
            printf("No rom path specified\n");
            return 1;
        }

        if (args.font_path == NULL)
        {
            printf("No font path specified, choosing default\n");
            args.font_path = "fonts/default.font";
        }

        printf("Rom path: %s\nFont path: %s\nDebug mode: %d\n", args.rom_path, args.font_path, (int)args.debug);
        printf("\n");
        // return 0;
        return emulate(&args);
    }

    printf("Usage: chip8 <rom_path>\n\t-f\"<font_path>\"\n\t-d enable debugging\n");
    return 1;
}

int emulate(struct args *args)
{
    // Init CPU
    init_chip8(&state);

    // Load rom into memory at location 0x200
    printf("Loading rom: \"%s\"\n", args->rom_path);

    FILE *rom_file = fopen(args->rom_path, "rb");
    
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
    printf("Loading font: %s\n", args->font_path);

    FILE *font_file = fopen(args->font_path, "rb");

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
            if (args->debug)
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