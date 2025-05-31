#include "board_time.h"

uint64_t __time = 0;
bool time_skip = true;
struct repeating_timer time_timer;

void time_init()
{
    add_repeating_timer_ms(TIME_PERIOD_MS, time_callback, NULL, &time_timer);
    __time = 0;
    time_stop();
}

void time_start()
{
    time_skip = false;
}

bool time_callback(struct repeating_timer *t)
{
    time_main();
    return true;
}

void time_main()
{
    __time += !time_skip ? TIME_PERIOD_MS : 0;
}

void time_stop()
{
    time_skip = true;
}

void time_deinit()
{
    cancel_repeating_timer(&time_timer);
}

uint64_t time_now_ms()
{
    return __time;
}

double time_now()
{
    return time_now_ms() / (double) 1000;
}

