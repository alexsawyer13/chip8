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

struct instruction
{
    u8 i;
    u8 x;
    u8 y;
    u8 N;
    u8 NN;
    u16 NNN;
};

static struct sdl_state sdl_state;
static struct state state;
static u8 halt;

void print_screen();
void render_screen();
void set_pixel(int width, int height);
void clear_pixel(int width, int height);
u8 toggle_pixel(int width, int height); // Returns the state of the pixel

void print_memory(int offset, int count, int vals_per_line);

void print_stack(int offset, int count);
void push_stack(u8 byte);
u8 pop_stack();

u8 step_emulation(); // Returns whether emulation should continue

void in_clear_screen();
void in_jump(u16 address);
void in_set_vx(u8 reg, u8 value);
void in_add_vx(u8 reg, u8 value);
void in_set_i(u16 address);
void in_display(u8 xreg, u8 yreg, u8 height);

int main(int argc, char *argv[])
{
    // Parse command line args
    if (argc != 2)
    {
        printf("Usage: chip8 <filename>\n");
        return 0;
    }
    const char *rom_path = argv[1];

    // Init CPU
    state.sp = state.stack; // Set stack pointer to the beginning of the stack
    state.cpu.pc = 0x200; // Program should be loaded in at 0x200 since OG hardware stored emulator from 0x000 to 0x1FF

    // Load rom into memory at location 0x200
    printf("Loading rom: \"%s\"\n", rom_path);

    FILE *rom_file = fopen(rom_path, "rb");
    
    fseek(rom_file, 0, SEEK_END);
    int rom_size = (int)ftell(rom_file);
    fseek(rom_file, 0, SEEK_SET);
    
    if (rom_size - 512 > MEMORY_SIZE)
    {
        printf("Rom size is %d bytes which is too large to fit in memory\n", rom_size);
        return 1;
    }

    printf("Rom size is %d bytes, reading into memory\n", rom_size);

    fread(&state.memory[0x200], 1, rom_size, rom_file);
    fclose(rom_file);

    // print_memory(0x200, 160, 16);

    // Setup font
    const char *font_path = "fonts/default.font";
    printf("Loading font: %s\n", font_path);

    FILE *font_file = fopen(font_path, "rb");

    fseek(font_file, 0, SEEK_END);
    int font_size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);

    if (font_size < 80)
    {
        printf("Font file size is %d bytes, it should be 80 bytes\n", font_size);
        return 1;
    }
    else if (font_size > 80)
    {
        printf("Font file size is %d bytes, taking first 80 bytes\n", font_size);
    }

    fread(&state.memory[0x50], 1, 80, font_file);
    fclose(font_file);

    // print_memory(0x50, 80, 5);

    // Init SDL
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &sdl_state.window, &sdl_state.renderer);
    SDL_RenderSetScale(sdl_state.renderer, (float)DISPLAY_SCALE, (float)DISPLAY_SCALE);

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

        if (!halt)
        {
            halt = !step_emulation();
        }
    }

    // SDL_Delay(3000);
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

u8 toggle_pixel(int width, int height)
{
    int i = height * DISPLAY_WIDTH + width;
    if (state.screen[i] == (u8)0)
    {
        state.screen[i] = (u8)1;
        return (u8)1;
    }
    else if (state.screen[i] == (u8)1)
    {
        state.screen[i] = (u8)0;
        return (u8)0;
    }
    else
    {
        printf("Trying to toggle pixel (%d, %d) but it isn't set to 1 or 0", width, height);
        return (u8)0;
    }
}

void print_memory(int offset, int count, int vals_per_line)
{
    for (int i = offset; i < offset + count; i++)
    {
        if ((i - offset) == 0)
        {
            printf("%#05x: %#04x ", i, state.memory[i]);
        }
        else if ((i - offset) % vals_per_line == 0)
        {
            printf("\n%#05x: %#04x ", i, state.memory[i]);
        }
        else
        {
            printf("%#04x ",state.memory[i]);
        }
    }
    printf("\n");
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

void in_clear_screen()
{
    // printf("Clear screen\n");
    memset(state.screen, 0, DISPLAY_SIZE);
}

void in_jump(u16 address)
{
    // printf("Jumping to address %x\n", address);
    state.cpu.pc = address;
}

void in_set_vx(u8 reg, u8 value)
{
    // printf("Setting v%x to %x\n", reg, value);
    state.cpu.v[reg] = value;
}

void in_add_vx(u8 reg, u8 value)
{
    // printf("Adding %x to v%x\n", value, reg);
    state.cpu.v[reg] += value;
}

void in_set_i(u16 address)
{
    // printf("Setting i to %x\n", address);
    state.cpu.i = address;
}

void in_display(u8 xreg, u8 yreg, u8 height)
{
    u8 x = state.cpu.v[xreg] % DISPLAY_WIDTH;
    u8 y = state.cpu.v[yreg] % DISPLAY_HEIGHT;
    state.cpu.v[0xF] = 0;

    for (int row = 0; row < height; row++)
    {
        u8 row_data = state.memory[state.cpu.i + row];
        for (int bit = 0; bit < 8; bit++)
        {
            u8 should_toggle = (row_data >> (7 - bit)) & 0x1;
            if (should_toggle && x + bit < DISPLAY_WIDTH && y + row < DISPLAY_HEIGHT)
            {
                u8 pixel = toggle_pixel(x + bit, y + row);
                if (pixel == 0)
                    state.cpu.v[0xF] = 1;
            }
        }
    }
}

u8 step_emulation()
{
    // Fetch
    u8 higher = state.memory[state.cpu.pc];
    u8 lower = state.memory[state.cpu.pc + 1];
    u16 whole_instruction = ((u16)higher << 8) + (u16)lower;
    // printf("Instruction: %#6x\n", (int)whole_instruction);
    state.cpu.pc += 2;

    if (whole_instruction == 0x1228) return 0; // TEMP
    if (whole_instruction == 0xFFFF) return 0; // TEMP

    // Decode
    struct instruction instruction;
    instruction.i   = (whole_instruction >> 12) & 0xF;
    instruction.x   = (whole_instruction >> 8)  & 0xF;
    instruction.y   = (whole_instruction >> 4)  & 0xF;
    instruction.N   = (whole_instruction >> 0)  & 0xF;
    instruction.NN  = (whole_instruction) & 0xFF;
    instruction.NNN = (whole_instruction) & 0xFFF;

    // printf("Instruction:%x x:%x y:%x N:%x NN:%#04x NNN:%#05x\n", instruction.i, instruction.x, instruction.y, instruction.N, instruction.NN, instruction.NNN);

    // Execute
    switch(instruction.i)
    {
    case 0x0:
        if (instruction.NNN == 0x0E0)
            in_clear_screen();
        break;
    case 0x1:
        in_jump(instruction.NNN);
        break;
    case 0x6:
        in_set_vx(instruction.x, instruction.NN);
        break;
    case 0x7:
        in_add_vx(instruction.x, instruction.NN);
        break;
    case 0xA:
        in_set_i(instruction.NNN);
        break;
    case 0xD:
        in_display(instruction.x, instruction.y, instruction.N);
        break;
    default:
        printf("Unknown instruction: %#06x", whole_instruction);
        break;
    }

    render_screen();

    return 1;
}