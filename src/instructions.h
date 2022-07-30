#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include "types.h"

/*
I set 0x0000 to be a halt instruction

0NNN is the "Execute machine language routine" instruction
which calls a subroutine in the machine language of the 
emulation device at address NNN

This isn't necessary to implement, so 0x0FFF is free for my halt!
*/

struct chip8;

void fetch_instruction(struct chip8 *state, u16 *instruction);
void decode_instruction(u16 instruction_bytes, struct instruction *instruction);
u8 execute_instruction(struct chip8 *state, struct instruction *instruction); // Returns whether or not instruction was known

u8 debug_instruction(struct chip8 *state, struct instruction *instruction); // Returns whether or not instruction was known

void in_clear_screen(struct chip8 *state);
void in_jump(struct chip8 *state, u16 address);
void in_set_vx(struct chip8 *state, u8 reg, u8 value);
void in_add_vx(struct chip8 *state, u8 reg, u8 value);
void in_set_i(struct chip8 *state, u16 address);
void in_display(struct chip8 *state, u8 xreg, u8 yreg, u8 height);
void in_start_subroutine(struct chip8 *state, u16 address);
void in_end_subroutine(struct chip8 *state);
void in_halt(struct chip8 *state);

#endif //_INSTRUCTIONS_H_