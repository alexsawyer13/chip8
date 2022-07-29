#include "chip8.h"

#include <stdio.h>

void init_chip8(struct chip8 *state)
{
    state->sp = state->stack; // Set stack pointer to the beginning of the stack
    state->cpu.pc = 0x200; // Program should be loaded in at 0x200 since OG hardware stored emulator from 0x000 to 0x1FF
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