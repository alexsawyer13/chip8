// https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT)

#define DISPLAY_SCALE 16
#define WINDOW_WIDTH (DISPLAY_WIDTH * DISPLAY_SCALE)
#define WINDOW_HEIGHT (DISPLAY_HEIGHT * DISPLAY_SCALE)

#define MEMORY_SIZE 4096

typedef uint8_t u8;
typedef uint16_t u16;

struct sdl_state
{
    SDL_Window *window;
    SDL_Renderer *renderer;
};

struct cpu
{
    u16 pc;
    u16 i;

    u8 delay;
    u8 sound;

    u8 v[16];
};

struct state
{
    struct cpu;

    u8 memory[MEMORY_SIZE];
    u8 screen[DISPLAY_SIZE];
};

static struct sdl_state sdl_state;
static struct state state;

void init_sdl();

void print_screen();
void render_screen();
void set_pixel(int width, int height);
void clear_pixel(int width, int height);

void print_memory(int offset, int count);

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }

    init_sdl();

    set_pixel(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);

    u8 loop = 1;
    while (loop)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
                loop = 0;
                break;
            }
        }
        if (!loop) break;

        render_screen();
    }

    SDL_Quit();
    return 0;
}

void init_sdl(struct sdl_state *state)
{
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &sdl_state.window, &sdl_state.renderer);
    SDL_RenderSetScale(sdl_state.renderer, (float)DISPLAY_SCALE, (float)DISPLAY_SCALE);
}

void print_screen()
{
    for (int j = 0; j < DISPLAY_HEIGHT; j++)
    {
        for (int i = 0; i < DISPLAY_WIDTH; i++)
        {
            printf("%d", state.screen[j * DISPLAY_WIDTH + i]);
        }
        printf("\n");
    }
    printf("\n");
}

void render_screen()
{
    SDL_SetRenderDrawColor(sdl_state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_state.renderer);
    SDL_SetRenderDrawColor(sdl_state.renderer, 255, 255, 255, 255);

    for (int i = 0; i < DISPLAY_SIZE; i++)
    {
        if (state.screen[i] == (u8)1)
        {
            int y = i / DISPLAY_WIDTH;
            int x = i - (y * DISPLAY_WIDTH);
            SDL_RenderDrawPoint(sdl_state.renderer, x, y);
        }
    }
    SDL_RenderPresent(sdl_state.renderer);
}

void set_pixel(int width, int height)
{
    state.screen[height * DISPLAY_WIDTH + width] = (u8)1;
}

void clear_pixel(int width, int height)
{
    state.screen[height * DISPLAY_WIDTH + width] = (u8)0;
}

void print_memory(int offset, int count)
{
    for (int i = offset; i < offset + count; i++)
    {
        printf("%#05x: %d\n", i, state.memory[i]);
    }
}