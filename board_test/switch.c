#include "switch.h"

struct repeating_timer switch_timer;
uint switches[] = {SW1, SW2, SW3, SW4, SWL, SWR};
uint _switch;

void switch_init()
{
    static bool _switch_inited = false;
    if(!_switch_inited)
    {
        for(uint8_t i = 0; i < sizeof(switches)/sizeof(switches[0]); i++)
        {
            gpio_init(switches[i]);
            gpio_set_dir(switches[i], GPIO_IN);
        }
        add_repeating_timer_ms(10, switch_callback, NULL, &switch_timer);
        _switch = 0;
        _switch_inited = true;
    }
}

bool switch_callback(struct repeating_timer *t)
{
    uint data = 0;
    for(uint8_t i = 0; i < sizeof(switches)/sizeof(switches[0]); i++)
    {
        data |= gpio_get(switches[i]) ? 1 << i : 0;
    }
    _switch = ~data;
    return true;
}

uint switch_read()
{
    return _switch;
}

