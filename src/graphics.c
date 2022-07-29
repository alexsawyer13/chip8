#include "graphics.h"
#include "chip8.h"

#include <SDL.h>

struct sdl_state
{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

static struct sdl_state sdl_state;

void init_graphics()
{
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &sdl_state.window, &sdl_state.renderer);
    SDL_RenderSetScale(sdl_state.renderer, (float)DISPLAY_SCALE, (float)DISPLAY_SCALE);
}

void shutdown_graphics()
{
    SDL_Quit();
}

void render_screen(struct chip8 *state)
{
    SDL_SetRenderDrawColor(sdl_state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_state.renderer);
    SDL_SetRenderDrawColor(sdl_state.renderer, 255, 255, 255, 255);

    for (int i = 0; i < DISPLAY_SIZE; i++)
    {
        if (state->screen[i] == (u8)1)
        {
            int y = i / DISPLAY_WIDTH;
            int x = i - (y * DISPLAY_WIDTH);
            SDL_RenderDrawPoint(sdl_state.renderer, x, y);
        }
    }
    SDL_RenderPresent(sdl_state.renderer);
}