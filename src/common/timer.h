#ifndef _TIMER_H_
#define _TIMER_H_

#include "types.h"

/*
Currently the timer only works on windows using QPC
https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancecounter
SDL timer only has ms precision which is bad :(
*/

struct timer
{
    u64 _delay_us;
    u64 _last_tick;
};

void init_timers();

void create_timer_us(struct timer *timer, u64 delay_us);

// Returns whether the timer should tick
u8 should_tick(struct timer *timer);

#endif //_TIMER_H_