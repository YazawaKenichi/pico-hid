#include "volume.h"

uint volumes[] = {VL1, VL2, VL3};

void volume_init()
{
    adc_init();
}

uint16_t volume_read(uint8_t volume)
{
    adc_select_input(volumes[volume]);
    return adc_read();
}

float volume_read_average()
{
    float s = 0;
    for(uint8_t i = 0; i < 3; i++)
    {
        s += volume_read(i);
    }
    return s / 3;
}

