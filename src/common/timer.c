#include "timer.h"

#include "platform.h"

void create_timer_us(struct timer *timer, u64 delay_us)
{
    timer->_delay_us = delay_us;
    timer->_last_tick = pf_get_time_us();
}

u8 should_tick(struct timer *timer)
{
    u64 current_time = pf_get_time_us();
    u64 time_since_last_tick_us = current_time - timer->_last_tick;

    if (time_since_last_tick_us > timer->_delay_us)
    {
        timer->_last_tick += timer->_delay_us;
        return 1;
    }

    return 0;
}