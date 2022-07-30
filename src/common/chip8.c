#include "chip8.h"

#include "platform.h"

#include <SDL.h> // SDL_SCANCODE

#include <stdio.h>
#include <string.h>

void init_chip8(struct chip8 *state)
{
    state->cpu.pc = 0x200; // Program should be loaded in at 0x200 since OG hardware stored emulator from 0x000 to 0x1FF
    state->cpu.i = 0;
    state->cpu.delay = 0;
    state->cpu.sound = 0;
    memset(state->cpu.v, 0, 16);
    memset(state->memory, 0, MEMORY_SIZE);
    memset(state->screen, 0, DISPLAY_SIZE);
    memset(state->stack, 0, STACK_SIZE);
    state->sp = state->stack; // Set stack pointer to the beginning of the stack
    state->cycles = 0;
    state->halt = 0;
    state->await_input = 0;
    state->input_register = 0;
}

void print_cpu(struct chip8 *state)
{
    printf("Program counter: %#06x\n", state->cpu.pc);
    printf("I register: %#06x\n", state->cpu.i);
    printf("Delay register: %#04x\n", state->cpu.delay);
    printf("Sound register: %#04x\n", state->cpu.sound);

    u8 *v = state->cpu.v;
    printf(
        "V0: %#04x    V1: %#04x    V2: %#04x    V3: %#04x\n"
        "V4: %#04x    V5: %#04x    V6: %#04x    V7: %#04x\n"
        "V8: %#04x    V9: %#04x    VA: %#04x    VB: %#04x\n"
        "VC: %#04x    VD: %#04x    VE: %#04x    VF: %#04x\n",
        v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8],
        v[9], v[0xA], v[0xB], v[0xC], v[0xD], v[0xE], v[0xF]
    );
}

u8 save_state(struct chip8 *state)
{
    FILE *file;
    if (!pf_mkdir("states"))
    {
        printf("./states directory already exists\n");
    }
    if (fopen_s(&file, "states/state.ch8", "w") != 0)
    {
        printf("Failed to open states/state.ch8\n");
        return 0;
    }

    // Registers
    fputc((state->cpu.pc >> 8) & 0xFF, file); // PC higher
    fputc(state->cpu.pc & 0xFF, file); // PC lower

    fputc((state->cpu.i >> 8) & 0xFF, file); // i higher
    fputc(state->cpu.i & 0xFF, file); // i lower

    fputc(state->cpu.delay, file); // delay
    fputc(state->cpu.sound, file); // sound

    for (int i = 0; i < 16; i++)
    {
        fputc(state->cpu.v[i], file); // v[0x0] - v[0xF] in order
    }

    // Memory
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        fputc(state->memory[i], file);
    }

    // Screen
    for (int i = 0; i < DISPLAY_SIZE; i++)
    {
        fputc(state->screen[i], file);
    }

    // Stack
    for (int i = 0; i < STACK_SIZE; i++)
    {
        fputc(state->stack[i], file);
    }

    size_t sp = (size_t)state->sp;
    for (int byte = 0; byte < sizeof(size_t); byte++) // sp high-low
    {
        fputc((int)(sp >> (8 * byte)), file);
    }

    // Variables

    for (int byte = 0; byte < 8; byte++) // Cycles high-low
    {
        fputc((int)(state->cycles >> (8 * byte)), file);
    }

    fputc((int)state->halt, file);
    fputc((int)state->await_input, file);
    fputc((int)state->input_register, file);

    fclose(file);
    return 1;
}

void print_screen(struct chip8 *state)
{
    for (int j = 0; j < DISPLAY_HEIGHT; j++)
    {
        for (int i = 0; i < DISPLAY_WIDTH; i++)
        {
            printf("%d", state->screen[j * DISPLAY_WIDTH + i]);
        }
        printf("\n");
    }
    printf("\n");
}

void set_pixel(struct chip8 *state, int width, int height)
{
    state->screen[height * DISPLAY_WIDTH + width] = (u8)1;
}

void clear_pixel(struct chip8 *state, int width, int height)
{
    state->screen[height * DISPLAY_WIDTH + width] = (u8)0;
}

u8 toggle_pixel(struct chip8 *state, int width, int height)
{
    int i = height * DISPLAY_WIDTH + width;
    if (state->screen[i] == (u8)0)
    {
        state->screen[i] = (u8)1;
        return (u8)1;
    }
    else if (state->screen[i] == (u8)1)
    {
        state->screen[i] = (u8)0;
        return (u8)0;
    }
    else
    {
        printf("Trying to toggle pixel (%d, %d) but it isn't set to 1 or 0", width, height);
        return (u8)0;
    }
}

u8 scancode_to_key(SDL_Scancode scancode)
{
    switch(scancode)
    {
        case SDL_SCANCODE_1: return 0x1;
        case SDL_SCANCODE_2: return 0x2;
        case SDL_SCANCODE_3: return 0x3;
        case SDL_SCANCODE_4: return 0xC;

        case SDL_SCANCODE_Q: return 0x4;
        case SDL_SCANCODE_W: return 0x5;
        case SDL_SCANCODE_E: return 0x6;
        case SDL_SCANCODE_R: return 0xD;

        case SDL_SCANCODE_A: return 0x7;
        case SDL_SCANCODE_S: return 0x8;
        case SDL_SCANCODE_D: return 0x9;
        case SDL_SCANCODE_F: return 0xE;

        case SDL_SCANCODE_Z: return 0xA;
        case SDL_SCANCODE_X: return 0x0;
        case SDL_SCANCODE_C: return 0xB;
        case SDL_SCANCODE_V: return 0xF;
    }
    return (u8)-1;
}

void print_memory(struct chip8 *state, int offset, int count, int vals_per_line)
{
    for (int i = offset; i < offset + count; i++)
    {
        if ((i - offset) == 0)
        {
            printf("%#05x: %#04x ", i, state->memory[i]);
        }
        else if ((i - offset) % vals_per_line == 0)
        {
            printf("\n%#05x: %#04x ", i, state->memory[i]);
        }
        else
        {
            printf("%#04x ",state->memory[i]);
        }
    }
    printf("\n");
}

void print_stack(struct chip8 *state, int offset, int count)
{
    printf("Current stack pointer is at offset %zu\n", (size_t)(state->sp-state->stack));
    for (int i = offset; i < offset + count; i++)
    {
        printf("%d: %" PRIu32 "\n", i, state->stack[i]);
    }
}

void push_stack(struct chip8 *state, u8 byte)
{
    if (state->sp > state->stack + STACK_SIZE - 1)
    {
        printf("Trying to push to a full stack, uh oh!\n");
        return;
    }
    *state->sp = byte;
    state->sp++;
}

u8 pop_stack(struct chip8 *state)
{
    if (state->sp <= state->stack)
    {
        printf("Trying to pop from an empty stack, uh oh!\n");
        return 0;
    }
    state->sp--;
    return *state->sp;
}