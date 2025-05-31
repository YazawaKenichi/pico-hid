#ifndef __SWITCH_H_
#define __SWITCH_H_
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define SW1 5
#define SW2 7
#define SW3 12
#define SW4 13
#define SWL 15
#define SWR 14

void switch_init();
uint switch_read();
bool switch_callback(struct repeating_timer *t);

#endif

