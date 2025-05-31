#ifndef __BOARD_TIME_H__
#define __BOARD_TIME_H__

#include "pico/stdlib.h"

#define TIME_PERIOD_MS 1

void time_init();
void time_start();
bool time_callback(struct repeating_timer *t);
void time_main();
void time_stop();
void time_deinit();
uint64_t time_now_ms();
double time_now();

#endif

