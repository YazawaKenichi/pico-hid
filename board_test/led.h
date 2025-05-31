#ifndef __LED_H_
#define __LED_H_
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define LED0 0  //! 赤 LED
#define LED1 1  //! 桃 LED
#define LED2 2
#define LED3 3
#define LED4 4

void led_init();
bool led_callback(struct repeating_timer *t);
uint led_read();
void led_write(uint);

#endif

