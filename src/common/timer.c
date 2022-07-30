#include "timer.h"

#include <Windows.h>

static u64 frequency;
static f32 period;

u64 get_time_us();

void init_timers()
{
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    frequency = f.QuadPart;
    period = 1.0f/frequency;
}

void create_timer_us(struct timer *timer, u64 delay_us)
{
    timer->_delay_us = delay_us;
    timer->_last_tick = get_time_us();
}

u8 should_tick(struct timer *timer)
{
    u64 current_time = get_time_us();
    u64 time_since_last_tick_us = current_time - timer->_last_tick;

    if (time_since_last_tick_us > timer->_delay_us)
    {
        timer->_last_tick += timer->_delay_us;
        return 1;
    }

    return 0;
}

u64 get_time_us()
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time.QuadPart * 1000000 * period;
}