#ifndef __MODE_H__
#define __MODE_H__
#include "switch.h"

#define MODE_COUNT 3

void mode_init();
bool mode_callback();
void mode_change(uint8_t i);
uint8_t mode_now();

#endif

