#include "mode.h"

#include "led.h"

uint8_t _mode;
struct repeating_timer mode_timer;

void mode_init()
{
    switch_init();
    add_repeating_timer_ms(50, mode_callback, NULL, &mode_timer);
}

bool mode_callback()
{
    static uint bef = 0;
    uint now = switch_read();

    uint diff = 0;
    for(uint8_t i = 0; i < 6; i++)
    {
        // diff |= (now & (1 << i)) - (bef & (1 << i)) <= 0 ? 0 : (1 << i);
        diff |= - ((now & (1 << i)) - (bef & (1 << i))) <= 0 ? 0 : (1 << i);
    }

    if(diff & (1 << 5))
    {
        mode_change(mode_now() + 1);
    }

    bef = now;
    return true;
}

void mode_change(uint8_t i)
{
    _mode = (i >= MODE_COUNT || 0 > i) ? 0 : i;
}

uint8_t mode_now()
{
    return _mode;
}

