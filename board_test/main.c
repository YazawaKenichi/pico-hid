#include "main.h"

#include <math.h>
#include "common.h"

void main_init()
{
    led_init();
    switch_init();
    volume_init();
    time_init();
    mode_init();
}

int main()
{
    main_init();

    time_start();
    while(1)
    {
        process();
    }

    return 0;
}

void process()
{
    switch(mode_now())
    {
        case 0:
            waveled();
            break;
        case 1:
            switch2led();
            break;
        case 2:
            sinwaveled();
            break;
        default:
            break;
    }
}

void switch2led()
{
    led_write(switch_read() << 1);
}

void waveled()
{
    static uint8_t bit = 1;
    led_write(bit);
    bit = (bit < 1 << 4) ? bit << 1 : 1;
    sleep_ms(1000);
}

void sinwaveled()
{
    static float _w = 0;

    float vra = volume_read_average();
    _w = lpf(vra, _w, 0.25);

    float w = _w * 10 * 2 * M_PI / (float) 4096;
    float y = sin(w * time_now());

    if(y > 0)
    {
        if(2.0 / 5.0 / 2.0 > y)
        {
            led_write(1 << 2);
        }
        else if(2.0 / 5.0 * (1 + 1.0 / 2.0) > y)
        {
            led_write(1 << 1);
        }
        else if(2.0 / 5.0 * (2 + 1.0 / 2.0) > y)
        {
            led_write(1 << 0);
        }
        else
        {
            led_write(0);
        }
    }
    else if(0 > y)
    {
        if(y > - 2.0 / 5.0 / 2.0)
        {
            led_write(1 << 2);
        }
        else if(y > - 2.0 / 5.0 * (1 + 1.0 / 2.0))
        {
            led_write(1 << 3);
        }
        else if(y > - 2.0 / 5.0 * (2 + 1.0 / 2.0))
        {
            led_write(1 << 4);
        }
        else
        {
            led_write(0);
        }
    }
    else if(y == 0)
    {
        led_write(1 << 2);
    }
}

