#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

struct chip8;

void init_graphics();
void render_screen(struct chip8 *state);
void shutdown_graphics();

#endif //_GRAPHICS_H_