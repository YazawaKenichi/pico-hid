#ifndef __VOLUME_H_
#define __VOLUME_H_
#include "hardware/adc.h"

#define VL0 26
#define VL1 0
#define VL2 1
#define VL3 2

void volume_init();
uint16_t volume_read(uint8_t);
float volume_read_average();

#endif

