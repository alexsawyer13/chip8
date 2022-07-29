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
#define STACK_SIZE 1024

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
    struct cpu cpu;

    u8 memory[MEMORY_SIZE];
    u8 screen[DISPLAY_SIZE];

    u8 stack[STACK_SIZE];
    u8 *sp;
};

static struct sdl_state sdl_state;
static struct state state;

void print_screen();
void render_screen();
void set_pixel(int width, int height);
void clear_pixel(int width, int height);

void print_memory(int offset, int count);

void print_stack(int offset, int count);
void push_stack(u8 byte);
u8 pop_stack();

int main(int argc, char *argv[])
{
    // Parse command line args
    if (argc != 2)
    {
        printf("Usage: chip8 <filename>\n");
        return 0;
    }
    const char *path = argv[1];

    // Init CPU
    state.sp = state.stack; // Set stack pointer to the beginning of the stack
    state.cpu.pc = 0x200; // Program should be loaded in at 0x200 since OG hardware stored emulator from 0x000 to 0x1FF

    // Load rom into memory at location 0x200
    printf("Loading rom: \"%s\"\n", path);

    FILE *file = fopen(path, "rb");
    
    fseek(file, 0, SEEK_END);
    int size = (int)ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size - 512 > MEMORY_SIZE)
    {
        printf("Rom size is %d bytes which is too large to fit in memory\n", size);
        return 1;
    }

    printf("Rom size is %d bytes, reading into memory\n", size);

    fread(&state.memory[0x200], 1, size, file);
    fclose(file);

    // TODO: SETUP FONT

    // Init SDL
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &sdl_state.window, &sdl_state.renderer);
    SDL_RenderSetScale(sdl_state.renderer, (float)DISPLAY_SCALE, (float)DISPLAY_SCALE);
    set_pixel(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);

    // Start emulation
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
        printf("%#05x: %#04x\n", i, state.memory[i]);
    }
}

void print_stack(int offset, int count)
{
    printf("Current stack pointer is at offset %zu\n", (size_t)(state.sp-state.stack));
    for (int i = offset; i < offset + count; i++)
    {
        printf("%d: %" PRIu32 "\n", i, state.stack[i]);
    }
}

void push_stack(u8 byte)
{
    if (state.sp > state.stack + STACK_SIZE - 1)
    {
        printf("Trying to push to a full stack, uh oh!\n");
        return;
    }
    *state.sp = byte;
    state.sp++;
}

u8 pop_stack()
{
    if (state.sp <= state.stack)
    {
        printf("Trying to pop from an empty stack, uh oh!\n");
        return 0;
    }
    state.sp--;
    return *state.sp;
}