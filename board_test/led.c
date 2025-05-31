#include "led.h"

uint leds[] = {LED0, LED1, LED2, LED3, LED4};
uint _led = 0;
struct repeating_timer led_timer;

void led_init()
{
    static bool _led_inited = false;
    if(!_led_inited)
    {
        for(uint8_t i = 0; i < sizeof(leds)/sizeof(leds[0]); i++)
        {
            gpio_init(leds[i]);
            gpio_set_dir(leds[i], GPIO_OUT);
        }
        gpio_put_all(0);
        add_repeating_timer_ms(10, led_callback, NULL, &led_timer);
        _led_inited = true;
    }
}

bool led_callback(struct repeating_timer *t)
{
    uint data = _led;
    uint mask = 0;
    uint out = 0;
    for(uint8_t i = 0; i < sizeof(leds)/sizeof(leds[0]); i++)
    {
        mask |= (1u << leds[i]);
        if(~data & (1u << i))
        {
            out |= (1u << leds[i]);
        }
    }
    gpio_put_masked(mask, out);
    return true;
}

uint led_read()
{
    return _led;
}

void led_write(uint data)
{
    _led = data;
}

