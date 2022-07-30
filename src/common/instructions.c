#include "instructions.h"

#include "chip8.h"

#include <stdio.h>

void fetch_instruction(struct chip8 *state, u16 *instruction)
{
    u8 higher = state->memory[state->cpu.pc];
    u8 lower = state->memory[state->cpu.pc + 1];
    *instruction = ((u16)higher << 8) + (u16)lower;
    state->cpu.pc += 2;
}

void decode_instruction(u16 instruction_bytes, struct instruction *instruction)
{
    instruction->instruction = instruction_bytes;
    instruction->i   = (instruction_bytes >> 12) & 0xF;
    instruction->x   = (instruction_bytes >> 8)  & 0xF;
    instruction->y   = (instruction_bytes >> 4)  & 0xF;
    instruction->N   = (instruction_bytes >> 0)  & 0xF;
    instruction->NN  = (instruction_bytes) & 0xFF;
    instruction->NNN = (instruction_bytes) & 0xFFF;
}

u8 execute_instruction(struct chip8 *state, struct instruction *instruction)
{
    switch(instruction->i)
    {
    case 0x0:
        if (instruction->NNN == 0x000)
            in_halt(state);
        else if (instruction->NNN == 0x0E0)
            in_clear_screen(state);
        else if (instruction->NNN == 0x0EE)
            in_end_subroutine(state);
        else
        {
            printf("Unknown host machine instruction: %#06x\n", instruction->instruction);
            return 1; // These instructions are external machine instructions on host device, don't fail if it happens
        }
        break;
    case 0x1:
        in_jump(state, instruction->NNN);
        break;
    case 0x2:
        in_start_subroutine(state, instruction->NNN);
        break;
    case 0x3:
        in_skip_vx_eq_nn(state, instruction->x, instruction->NN);
        break;
    case 0x4:
        in_skip_vx_neq_nn(state, instruction->x, instruction->NN);
        break;
    case 0x5:
        if (instruction->N == 0)
        {
            in_skip_vx_eq_vy(state, instruction->x, instruction->y);
        }
        else
        {
            printf("Unknown instruction: %#06x\n", instruction->instruction);
            return 0;   
        }
        break;
    case 0x6:
        in_set_vx(state, instruction->x, instruction->NN);
        break;
    case 0x7:
        in_add_vx(state, instruction->x, instruction->NN);
        break;
    case 0x9:
        if (instruction->N == 0)
        {
            in_skip_vx_neq_vy(state, instruction->x, instruction->y);
        }
        else
        {
            printf("Unknown instruction: %#06x\n", instruction->instruction);
            return 0;   
        }
        break;
    case 0xA:
        in_set_i(state, instruction->NNN);
        break;
    case 0xD:
        in_display(state, instruction->x, instruction->y, instruction->N);
        break;
    case 0xF:
        switch(instruction->NN)
        {
        case 0x1E:
            in_add_i(state, instruction->x);
            break;
        default:
            printf("Unknown instruction: %#06x\n", instruction->instruction);
            return 0;
        }
        break;
    default:
        printf("Unknown instruction: %#06x\n", instruction->instruction);
        return 0;
    }
    return 1;
}

u8 debug_instruction(struct chip8 *state, struct instruction *instruction)
{
    printf("%#06x %#06x: ", state->cpu.pc, instruction->instruction);
    switch(instruction->i)
    {
    case 0x0:
        if (instruction->NNN == 0x000)
            printf("Halting\n");
        else if (instruction->NNN == 0x0E0)
            printf("Clearing screen\n");
        else if (instruction->NNN == 0x0EE)
            printf("Returning from subroutine\n");
        else
        {
            printf("Host machine instruction, not implemented\n"); 
            return 1; // These instructions are external machine instructions on host device, don't fail if it happens
        }
        break;
    case 0x1:
        printf("Jumping to address %#x\n", instruction->NNN);
        break;
    case 0x2:
        printf("Starting subroutine at address %#x\n", instruction->NNN);
        break;
    case 0x3:
        printf("Skipping instruction if v[%x] == %x", instruction->x, instruction->NN);
        if (state->cpu.v[instruction->x] == instruction->NN)
            printf("    (skipping)");
        else
            printf("    (not skipping)");
        printf("\n");
        break;
    case 0x4:
        printf("Skipping instruction if v[%x] != %x", instruction->x, instruction->NN);
        if (state->cpu.v[instruction->x] != instruction->NN)
            printf("    (skipping)");
        else
            printf("    (not skipping)");
        printf("\n");
        break;
    case 0x5:
        printf("Skipping instruction if v[%x] == v[%x]", instruction->x, instruction->y);
        if (state->cpu.v[instruction->x] == state->cpu.v[instruction->y])
            printf("    (skipping)");
        else
            printf("    (not skipping)");
        printf("\n");
        break;
    case 0x6:
        printf("Setting v%x to %#x\n", instruction->x, instruction->NN);
        break;
    case 0x7:
        printf("Adding %#x to v%x\n", instruction->NN, instruction->x);
        break;
    case 0x9:
        printf("Skipping instruction if v[%x] != v[%x]", instruction->x, instruction->y);
        if (state->cpu.v[instruction->x] != state->cpu.v[instruction->y])
            printf("    (skipping)");
        else
            printf("    (not skipping)");
        printf("\n");
        break;
    case 0xA:
        printf("Setting i to %#x\n", instruction->NNN);
        break;
    case 0xD:
        printf("Displaying character %#x at position (%d, %d) of height %d at\n", state->cpu.i, (int)state->cpu.v[instruction->x], (int)state->cpu.v[instruction->y], instruction->N);
        break;
    case 0xF:
        switch(instruction->NN)
        {
        case 0x1E:
            printf("Adding value of v[%x] to i register\n", instruction->x);
            break;
        default:
            printf("No debug string set");
            return 0;
        }
        break;
    default:
        printf("No debug string set");
        return 0;
    }
    return 1;
}

void in_clear_screen(struct chip8 *state)
{
    // printf("Clear screen\n");
    memset(state->screen, 0, DISPLAY_SIZE);
}

void in_jump(struct chip8 *state, u16 address)
{
    // printf("Jumping to address %x\n", address);
    state->cpu.pc = address;
}

void in_set_vx(struct chip8 *state, u8 reg, u8 value)
{
    // printf("Setting v%x to %x\n", reg, value);
    state->cpu.v[reg] = value;
}

void in_add_vx(struct chip8 *state, u8 reg, u8 value)
{
    // printf("Adding %x to v%x\n", value, reg);
    state->cpu.v[reg] += value;
}

void in_set_i(struct chip8 *state, u16 address)
{
    // printf("Setting i to %x\n", address);
    state->cpu.i = address;
}

void in_display(struct chip8 *state, u8 xreg, u8 yreg, u8 height)
{
    u8 x = state->cpu.v[xreg] % DISPLAY_WIDTH;
    u8 y = state->cpu.v[yreg] % DISPLAY_HEIGHT;
    state->cpu.v[0xF] = 0;

    for (int row = 0; row < height; row++)
    {
        u8 row_data = state->memory[state->cpu.i + row];
        for (int bit = 0; bit < 8; bit++)
        {
            u8 should_toggle = (row_data >> (7 - bit)) & 0x1;
            if (should_toggle && x + bit < DISPLAY_WIDTH && y + row < DISPLAY_HEIGHT)
            {
                u8 pixel = toggle_pixel(state, x + bit, y + row);
                if (pixel == 0)
                    state->cpu.v[0xF] = 1;
            }
        }
    }
}

void in_start_subroutine(struct chip8 *state, u16 address)
{
    u8 lower = state->cpu.pc & 0xFF;
    u8 higher = (state->cpu.pc >> 8) & 0xFF;

    push_stack(state, higher);
    push_stack(state, lower);

    state->cpu.pc = address;
}

void in_end_subroutine(struct chip8 *state)
{
    u8 lower = pop_stack(state);
    u8 higher = pop_stack(state);
    state->cpu.pc = ((u16)higher << 8) + (u16)lower;
}

void in_halt(struct chip8 *state)
{
    state->halt = 1;
}

void in_add_i(struct chip8 *state, u8 xreg)
{
    state->cpu.i += state->cpu.v[xreg];
    if (state->cpu.i >= 0x1000)
    {
        state->cpu.v[0xF] = 1;
    }
}

void in_skip_vx_eq_nn(struct chip8 *state, u8 xreg, u8 nn)
{
    if (state->cpu.v[xreg] == nn)
    {
        state->cpu.pc += 2;
    }
}

void in_skip_vx_neq_nn(struct chip8 *state, u8 xreg, u8 nn)
{
    if (state->cpu.v[xreg] != nn)
    {
        state->cpu.pc += 2;
    }
}

void in_skip_vx_eq_vy(struct chip8 *state, u8 xreg, u8 yreg)
{
    if (state->cpu.v[xreg] == state->cpu.v[yreg])
    {
        state->cpu.pc += 2;
    }
}

void in_skip_vx_neq_vy(struct chip8 *state, u8 xreg, u8 yreg)
{
    if (state->cpu.v[xreg] != state->cpu.v[yreg])
    {
        state->cpu.pc += 2;
    }
}