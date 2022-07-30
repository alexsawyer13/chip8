#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "types.h"

struct chip8;

void init_platform();
void shutdown_platform();

// Rendering
void pf_render_screen(struct chip8 *state);

// Events
u8 pf_poll_events(); // Returns 0 if program should exit
u8 pf_get_key_pressed(int scancode);
u8 pf_get_key_released(int scancode);
u8 pf_get_key_held(int scancode);

// Time
u64 pf_get_time_us();

// Maths
int pf_rand();

// Files
u8 pf_mkdir(const char *path);

#endif //_PLATFORM_H_