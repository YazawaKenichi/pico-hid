#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED0 0  //! 赤 LED
#define LED1 1  //! 桃 LED
#define LED2 2
#define LED3 3
#define LED4 4

#define SW1 5
#define SW2 7
#define SW3 12
#define SW4 13
#define SWL 15
#define SWR 14

#define VL0 3   //! ADC3 は存在しないので使用不可
#define VL1 2
#define VL2 1
#define VL3 0

#define INTERVAL_MS 1

#define RESC(X) rescale(X, 4096, 0, 50, 0)

enum {BLINK_NOT_MOUNTED = 250, BLINK_MOUNTED = 1000, BLINK_SUSPENDED = 2500,};

void main_init();
void task();
int on_timing();
void hid_task(bool);
void send_hid_report(uint8_t);
float rescale(int16_t, int16_t, int16_t, int16_t, int16_t);
void tud_hid_report_complete_cb(uint8_t, uint8_t const*, uint16_t);
uint16_t tud_hid_get_report_cb(uint8_t, uint8_t, hid_report_type_t, uint8_t*, uint16_t);
void tud_hid_set_report_cb(uint8_t, uint8_t, hid_report_type_t, uint8_t const*, uint16_t);
void led_blinking_task();
void tud_mount_cb();
void tud_umount_cb();
void tud_suspend_cb(bool);
void tud_resume_cb();

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
uint leds[] = {LED0, LED1, LED2, LED3, LED4};
uint switches[] = {SW1, SW2, SW3, SW4, SWL, SWR};


