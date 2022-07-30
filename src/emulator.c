// https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

#include "common/types.h"
#include "common/instructions.h"
#include "common/chip8.h"
#include "common/graphics.h"
#include "common/timer.h"

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
    u32 tick_rate;
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
                case 't':
                    if (args.tick_rate == 0)
                    {
                        args.tick_rate = atoi(str + 2);
                    }
                    else
                    {
                        printf("-t flag defined twice\n");
                        return 1;
                    }
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

        if (args.tick_rate == 0)
        {
            printf("No tick rate specified, choosing default\n");
            args.tick_rate = 1000;
        }

        printf("Rom path: %s\nFont path: %s\nDebug mode: %d\nTick rate: %d\n", args.rom_path, args.font_path, (int)args.debug, (int)args.tick_rate);
        printf("\n");
        return emulate(&args);
    }

    printf("Usage: chip8 <rom_path>\n\t-f\"<font_path>\"\n\t-d enable debugging\n\t-t<tps> sets tick rate\n");
    return 1;
}

int emulate(struct args *args)
{
    // Init CPU
    init_chip8(&state);

    // Init graphics
    init_graphics();

    // Load rom into memory at location 0x200
    printf("Loading rom: \"%s\"\n", args->rom_path);

    FILE *rom_file;
    if (fopen_s(&rom_file, args->rom_path, "rb") != 0)
    {
        printf("Failed to open rom file: %s\n", args->rom_path);
        return 1;
    }
    
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

    FILE *font_file;
    if (fopen_s(&font_file, args->font_path, "rb") != 0)
    {
        printf("Failed to open font file: %s\n", args->font_path);
        return 1;
    }

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

    // Timers
    init_timers();
    struct timer timer_60hz;
    struct timer timer_instruction;
    create_timer_us(&timer_60hz, (u64)(1000000.0f/60.0f));
    create_timer_us(&timer_instruction, (u64)(1000000.0f/(float)args->tick_rate));

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
            case SDL_KEYDOWN:
                if (state.await_input)
                {
                    SDL_Scancode scancode = event.key.keysym.scancode;
                    u8 key = scancode_to_key(scancode);
                    if (key != 0xFF)
                    {
                        state.cpu.v[state.input_register] = key;
                        if (args->debug)
                            printf("Saving key %#03x into register v[%x]\n", key, state.input_register);
                        state.await_input = 0;
                    }
                }
                break;
            }
        }
        if (!loop) break;

        // Emulate
        if (!state.halt)
        {
            // TODO: Beep when sound timer > 0

            if (should_tick(&timer_instruction) && !state.await_input)
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

                // Hardcoded to stop after 100 cycles
                if (state.cycles > 1000)
                {
                    state.halt = 1;
                }
            }

            if (should_tick(&timer_60hz))
            {
                if (state.cpu.delay > 0)
                {
                    state.cpu.delay--;
                }
                if (state.cpu.sound > 0)
                {
                    state.cpu.delay--;
                }
            }
        }
    }

    shutdown_graphics();
    return 0;
}