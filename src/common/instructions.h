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
struct instruction;

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
void in_add_i(struct chip8 *state, u8 xreg);
void in_skip_vx_eq_nn(struct chip8 *state, u8 xreg, u8 nn);
void in_skip_vx_neq_nn(struct chip8 *state, u8 xreg, u8 nn);
void in_skip_vx_eq_vy(struct chip8 *state, u8 xreg, u8 yreg);
void in_skip_vx_neq_vy(struct chip8 *state, u8 xreg, u8 yreg);
void in_get_key(struct chip8* state, u8 xreg);
void in_set_vx_vy(struct chip8* state, u8 xreg, u8 yreg);
void in_or_vx_vy(struct chip8* state, u8 xreg, u8 yreg);
void in_and_vx_vy(struct chip8* state, u8 xreg, u8 yreg);
void in_xor_vx_vy(struct chip8* state, u8 xreg, u8 yreg);
void in_add_vx_vy(struct chip8* state, u8 xreg, u8 yreg);
void in_sub_vx_vy(struct chip8* state, u8 xreg, u8 yreg);
void in_sub_vy_vx(struct chip8* state, u8 xreg, u8 yreg);
void in_shift_left_modern(struct chip8* state, u8 xreg, u8 yreg); // Using modern convention of ignoring vy
void in_shift_right_modern(struct chip8* state, u8 xreg, u8 yreg); // Using modern convention of ignoring vy
void in_shift_left_classic(struct chip8* state, u8 xreg, u8 yreg); // Using classic convention of copying vy into vx
void in_shift_right_classic(struct chip8* state, u8 xreg, u8 yreg); // Using classic convention of copying vy into vx
void in_store_modern(struct chip8 *state, u8 xreg); // Using modern convention of fixing I
void in_load_modern(struct chip8 *state, u8 xreg); // Using modern convention of fixing I
void in_store_classic(struct chip8 *state, u8 xreg); // Using classic convention of incrementing I
void in_load_classic(struct chip8 *state, u8 xreg); // Using classic convention of incrementing I
void in_bin_to_dec(struct chip8 *state, u8 xreg);
void in_random(struct chip8 *state, u8 xreg, u8 nn);
void in_skip_vx_pressed(struct chip8 *state, u8 xreg);
void in_skip_vx_npressed(struct chip8 *state, u8 xreg);
void in_font_character(struct chip8 *state, u8 xreg);
void in_set_vx_delay(struct chip8 *state, u8 xreg);
void in_set_delay_vx(struct chip8 *state, u8 xreg);
void in_set_sound_vx(struct chip8 *state, u8 xreg);
void in_jump_offset_classic(struct chip8 *state, u8 xreg, u16 nnn); // Uses classic convention of jumping to NNN + v0
void in_jump_offset_broken(struct chip8 *state, u8 xreg, u16 nnn); // Uses chip-48 unintentional convention of jumping to XNN + vx

#endif //_INSTRUCTIONS_H_