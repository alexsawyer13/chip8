#include "platform.h"
#include "chip8.h"
#include "types.h"

#include <SDL.h>

#include <stdlib.h>
#include <time.h>

// Windows
#include <Windows.h> // QPF
#include <direct.h> // _mkdir

#define MAX_KEYS 1024

struct sdl_state
{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

struct timer_state
{
    u64 frequency;
    f32 period;
};

struct input_state
{
    u8 pressed[MAX_KEYS];
    u8 released[MAX_KEYS];
    u8 held[MAX_KEYS];
};

static struct sdl_state sdl_state;
static struct timer_state timer_state;
static struct input_state input_state;

void init_platform()
{
    // SDl
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &sdl_state.window, &sdl_state.renderer);
    SDL_RenderSetScale(sdl_state.renderer, (float)DISPLAY_SCALE, (float)DISPLAY_SCALE);
    SDL_SetRenderDrawColor(sdl_state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_state.renderer);
    SDL_RenderPresent(sdl_state.renderer);

    // Windows QPF timer
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    timer_state.frequency = f.QuadPart;
    timer_state.period = 1.0f/timer_state.frequency;

    // RNG
    srand((unsigned int)time(NULL));
}

void shutdown_platform()
{
    SDL_Quit();
}

void pf_render_screen(struct chip8 *state)
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

u8 pf_poll_events()
{
    memset(input_state.pressed, 0, MAX_KEYS);
    memset(input_state.released, 0, MAX_KEYS);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_QUIT:
            return 0;
            break;
        case SDL_KEYDOWN:
            input_state.pressed[(int)event.key.keysym.scancode] = 1;
            input_state.held[(int)event.key.keysym.scancode] = 1;
            break;
        case SDL_KEYUP:
            input_state.released[(int)event.key.keysym.scancode] = 1;
            input_state.held[(int)event.key.keysym.scancode] = 0;
            break;
        }
    }
    return 1;
}

u8 pf_get_key_pressed(int scancode)
{
    return input_state.pressed[scancode];
}

u8 pf_get_key_released(int scancode)
{
    return input_state.released[scancode];
}

u8 pf_get_key_held(int scancode)
{
    return input_state.held[scancode];
}

u64 pf_get_time_us()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return (u64) (time.QuadPart * 1000000 * timer_state.period);
}

int pf_rand()
{
    return rand();
}

u8 pf_mkdir(const char *path)
{
    if (_mkdir(path) == 0) return 1;
    return 0;
}