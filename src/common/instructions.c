#include "instructions.h"

#include "chip8.h"
#include "platform.h"

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
    case 0x8:
        switch(instruction->N)
        {
        case 0x0:
            in_set_vx_vy(state, instruction->x, instruction->y);
            break;
        case 0x1:
            in_or_vx_vy(state, instruction->x, instruction->y);
            break;
        case 0x2:
            in_and_vx_vy(state, instruction->x, instruction->y);
            break;
        case 0x3:
            in_xor_vx_vy(state, instruction->x, instruction->y);
            break;
        case 0x4:
            in_add_vx_vy(state, instruction->x, instruction->y);
            break;
        case 0x5:
            in_sub_vx_vy(state, instruction->x, instruction->y);
            break;
        case 0x6:
            in_shift_right_modern(state, instruction->x, instruction->y);
            break;
        case 0x7:
            in_sub_vy_vx(state, instruction->x, instruction->y);
            break;
        case 0xE:
            in_shift_left_modern(state, instruction->x, instruction->y);
            break;
        default:
            printf("Unknown instruction: %#06x\n", instruction->instruction);
            return 0;
        }
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
    case 0xB:
        in_jump_offset_classic(state, instruction->x, instruction->NNN);
        break;
    case 0xC:
        in_random(state, instruction->x, instruction->NN);
        break;
    case 0xD:
        in_display(state, instruction->x, instruction->y, instruction->N);
        break;
    case 0xE:
        switch(instruction->NN)
        {
        case 0x9E:
            in_skip_vx_pressed(state, instruction->x);
            break;
        case 0xA1:
            in_skip_vx_npressed(state, instruction->x);
            break;
        default:
            printf("Unknown instruction: %#06x\n", instruction->instruction);
            return 0;
        }
        break;
    case 0xF:
        switch(instruction->NN)
        {
        case 0x07:
            in_set_vx_delay(state, instruction->x);
            break;
        case 0x15:
            in_set_delay_vx(state, instruction->x);
            break;
        case 0x18:
            in_set_sound_vx(state, instruction->x);
            break;
        case 0x29:
            in_font_character(state, instruction->x);
            break;
        case 0x33:
            in_bin_to_dec(state, instruction->x);
            break;
        case 0x55:
            in_store_modern(state, instruction->x);
            break;
        case 0x65:
            in_load_modern(state, instruction->x);
            break;
        case 0x1E:
            in_add_i(state, instruction->x);
            break;
        case 0x0A:
            in_get_key(state, instruction->x);
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
        printf("Skipping instruction if v[%x] == %#x", instruction->x, instruction->NN);
        if (state->cpu.v[instruction->x] == instruction->NN)
            printf("    (skipping)");
        else
            printf("    (not skipping)");
        printf("\n");
        break;
    case 0x4:
        printf("Skipping instruction if v[%x] != %#x", instruction->x, instruction->NN);
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
        printf("Setting v[%x] to %#x\n", instruction->x, instruction->NN);
        break;
    case 0x7:
        printf("Adding %#x to v[%x]\n", instruction->NN, instruction->x);
        break;
    case 0x8:
        switch(instruction->N)
        {
        case 0x0:
            printf("Setting v[%x] to v[%x]\n", instruction->x, instruction->y);
            break;
        case 0x1:
            printf("Computing v[%x] |= v[%x]\n", instruction->x, instruction->y);
            break;
        case 0x2:
            printf("Computing v[%x] &= v[%x]\n", instruction->x, instruction->y);
            break;
        case 0x3:
            printf("Computing v[%x] ^= v[%x]\n", instruction->x, instruction->y);
            break;
        case 0x4:
            printf("Computing v[%x] += v[%x]\n", instruction->x, instruction->y);
            break;
        case 0x5:
            printf("Computing v[%x] -= v[%x]\n", instruction->x, instruction->y);
            break;
        case 0x6:
            printf("Bitshifting right v[%x]\n", instruction->x);
            break;
        case 0x7:
            printf("Computing v[%x] = v[%x] - v[%x]\n", instruction->x, instruction->y, instruction->x);
            break;
        case 0xE:
            printf("Bitshifting left v[%x]\n", instruction->x);
            break;
        default:
            printf("No debug string set\n");
            return 0;
        }
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
    case 0xB:
        printf("Jumping to %#x + v[0]", instruction->NNN);
        break;
    case 0xC:
        printf("Setting v[%x] to rand & %#x     (%#x)\n", instruction->x, instruction->NN, state->cpu.v[instruction->x]);
        break;
    case 0xD:
        printf("Displaying character %#x at position (%d, %d) of height %d at\n", state->cpu.i, (int)state->cpu.v[instruction->x], (int)state->cpu.v[instruction->y], instruction->N);
        break;
    case 0xE:
        switch(instruction->NN)
        {
        case 0x9E:
            printf("Skip if v[%x] (%#x) is pressed\n", instruction->x, state->cpu.v[instruction->x]);
            break;
        case 0xA1:
            printf("Skip if v[%x] (%#x) isn't pressed\n", instruction->x, state->cpu.v[instruction->x]);
            break;
        default:
            printf("Unknown instruction: %#06x\n", instruction->instruction);
            return 0;
        }
        break;
    case 0xF:
        switch(instruction->NN)
        {
        case 0x07:
            printf("Setting v[%x] to delay register\n", instruction->x);
            break;
        case 0x15:
            printf("Setting delay register to v[%x]\n", instruction->x);
            break;
        case 0x18:
            printf("Setting sound register to v[%x]\n", instruction->x);
            break;
        case 0x29:
            printf("Setting i to address of character %x\n", (u8)(state->cpu.v[instruction->x] & 0xF));
            break;
        case 0x33:
            printf("Converting v[%x] to decimal at address %#x\n", instruction->x, state->cpu.i);
            break;
        case 0x55:
            printf("Storing registers from v[0] to v[%x] to address %#x\n", instruction->x, state->cpu.i);
            break;
        case 0x65:
            printf("Loading registers from v[0] to v[%x] from address %#x\n", instruction->x, state->cpu.i);
            break;
        case 0x1E:
            printf("Adding value of v[%x] to i register\n", instruction->x);
            break;
        case 0x0A:
            printf("Getting key input into register v[%x]\n", instruction->x);
            break;
        default:
            printf("No debug string set\n");
            return 0;
        }
        break;
    default:
        printf("No debug string set\n");
        return 0;
    }
    return 1;
}

void in_clear_screen(struct chip8 *state)
{
    memset(state->screen, 0, DISPLAY_SIZE);
}

void in_jump(struct chip8 *state, u16 address)
{
    state->cpu.pc = address;
}

void in_set_vx(struct chip8 *state, u8 reg, u8 value)
{
    state->cpu.v[reg] = value;
}

void in_add_vx(struct chip8 *state, u8 reg, u8 value)
{
    state->cpu.v[reg] += value;
}

void in_set_i(struct chip8 *state, u16 address)
{
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

void in_get_key(struct chip8* state, u8 xreg)
{
    state->await_input = 1;
    state->input_register = xreg;
}

void in_set_vx_vy(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[xreg] = state->cpu.v[yreg];
}

void in_or_vx_vy(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[xreg] |= state->cpu.v[yreg];
}

void in_and_vx_vy(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[xreg] &= state->cpu.v[yreg];
}

void in_xor_vx_vy(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[xreg] ^= state->cpu.v[yreg];
}

void in_add_vx_vy(struct chip8* state, u8 xreg, u8 yreg)
{
    u32 sum = (u32)state->cpu.v[xreg] + (u32)state->cpu.v[yreg];
    state->cpu.v[0xF] = (sum > 255); // Overflow
    state->cpu.v[xreg] = (u8)sum;
}

void in_sub_vx_vy(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[0xF] = (state->cpu.v[xreg] >= state->cpu.v[yreg]);
    state->cpu.v[xreg] -= state->cpu.v[yreg];
}

void in_sub_vy_vx(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[0xF] = (state->cpu.v[yreg] >= state->cpu.v[xreg]);
    state->cpu.v[xreg] = state->cpu.v[yreg] - state->cpu.v[xreg];
}

void in_shift_left_modern(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[xreg] = state->cpu.v[xreg] << 1;
    state->cpu.v[0xF] = (state->cpu.v[xreg] >> 7) & 0x1;
}

void in_shift_right_modern(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[xreg] = state->cpu.v[xreg] >> 1;
    state->cpu.v[0xF] = state->cpu.v[xreg] & 0x1;
}

void in_shift_left_classic(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[xreg] = state->cpu.v[yreg];
    in_shift_left_modern(state, xreg, yreg);
}

void in_shift_right_classic(struct chip8* state, u8 xreg, u8 yreg)
{
    state->cpu.v[xreg] = state->cpu.v[yreg];
    in_shift_right_modern(state, xreg, yreg);
}

void in_store_modern(struct chip8 *state, u8 xreg)
{
    for (int reg = 0; reg <= xreg; reg++)
    {
        state->memory[state->cpu.i + reg] = state->cpu.v[reg];
    }
}

void in_load_modern(struct chip8 *state, u8 xreg)
{
    for (int reg = 0; reg <= xreg; reg++)
    {
        state->cpu.v[reg] = state->memory[state->cpu.i + reg];
    }
}

void in_store_classic(struct chip8 *state, u8 xreg)
{
    in_store_modern(state, xreg);
    state->cpu.i += xreg;
}

void in_load_classic(struct chip8 *state, u8 xreg)
{
    in_load_modern(state, xreg);
    state->cpu.i += xreg;
}

void in_bin_to_dec(struct chip8 *state, u8 xreg)
{
    u8 vx = state->cpu.v[xreg];

    u8 a, b, c;
    a = vx / 100;
    b = (vx%100) / 10;
    c = (vx%10);

    state->memory[state->cpu.i    ] = a;
    state->memory[state->cpu.i + 1] = b;
    state->memory[state->cpu.i + 2] = c;
}

void in_random(struct chip8 *state, u8 xreg, u8 nn)
{
    state->cpu.v[xreg] = (pf_rand() % 256) & nn;
}

void in_skip_vx_pressed(struct chip8 *state, u8 xreg)
{
    if (pf_get_key_held(chip8_keys[state->cpu.v[xreg]]))
    {
        state->cpu.pc += 2;
    }
}

void in_skip_vx_npressed(struct chip8 *state, u8 xreg)
{
    if (!pf_get_key_held(chip8_keys[state->cpu.v[xreg]]))
    {
        state->cpu.pc += 2;
    }
}

void in_font_character(struct chip8 *state, u8 xreg)
{
    u8 character = (u8)(state->cpu.v[xreg] & 0xF);
    state->cpu.i = 0x50 + (5 * character);
}

void in_set_vx_delay(struct chip8 *state, u8 xreg)
{
    state->cpu.v[xreg] = state->cpu.delay;
}

void in_set_delay_vx(struct chip8 *state, u8 xreg)
{
    state->cpu.delay = state->cpu.v[xreg];
}

void in_set_sound_vx(struct chip8 *state, u8 xreg)
{
    state->cpu.sound = state->cpu.v[xreg];
}

void in_jump_offset_classic(struct chip8 *state, u8 xreg, u16 nnn)
{
    state->cpu.pc = nnn + state->cpu.v[0];
}

void in_jump_offset_broken(struct chip8 *state, u8 xreg, u16 nnn)
{
    state->cpu.pc = nnn + state->cpu.v[xreg];
}