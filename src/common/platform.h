#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "types.h"

struct chip8;

void init_platform();
void shutdown_platform();

void pf_render_screen(struct chip8 *state);

u64 pf_get_time_us();

u8 pf_mkdir(const char *path);

#endif //_PLATFORM_H_