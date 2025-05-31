#ifndef __MAIN_H_
#define __MAIN_H_
#include "led.h"
#include "switch.h"
#include "volume.h"
#include "board_time.h"
#include "mode.h"

void main_init();
void process();
void switch2led();
void waveled();
void sinwaveled();

#endif

