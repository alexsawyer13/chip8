#ifndef _CHIP8_H_
#define _CHIP8_H_

#include "types.h"

#include <SDL_scancode.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT)

#define DISPLAY_SCALE 16
#define WINDOW_WIDTH (DISPLAY_WIDTH * DISPLAY_SCALE)
#define WINDOW_HEIGHT (DISPLAY_HEIGHT * DISPLAY_SCALE)

#define MEMORY_SIZE 4096
#define STACK_SIZE 1024

#define NUM_CHIP_KEYS 16

extern int chip8_keys[NUM_CHIP_KEYS];

struct cpu
{
    u16 pc;
    u16 i;

    u8 delay;
    u8 sound;

    u8 v[16];
};

struct chip8
{
    struct cpu cpu;

    u8 memory[MEMORY_SIZE];
    u8 screen[DISPLAY_SIZE];

    u8 stack[STACK_SIZE];
    u8 *sp;

    u64 cycles;
    u8 halt;
    u8 await_input;
    u8 input_register;
};

struct instruction
{
    u16 instruction;
    u8 i;
    u8 x;
    u8 y;
    u8 N;
    u8 NN;
    u16 NNN;
};

// General
void init_chip8(struct chip8 *state);
void print_cpu(struct chip8 *state);
u8 save_state(struct chip8 *state);

// Screen
void print_screen(struct chip8 *state);
void set_pixel(struct chip8 *state, int width, int height);
void clear_pixel(struct chip8 *state, int width, int height);
u8 toggle_pixel(struct chip8 *state, int width, int height); // Returns the state of the pixel

// Keys
u8 get_chip_key();

// Memory
void print_memory(struct chip8 *state, int offset, int count, int vals_per_line);

// Stack
void print_stack(struct chip8 *state, int offset, int count);
void push_stack(struct chip8 *state, u8 byte);
u8 pop_stack(struct chip8 *state);

#endif //_CHIP8_H_