#include "common.h"

float lpf(float v, float v_bef, float g)
{
    return (g * v + (1 - g) * v_bef);
}

float rescale(float in, float in_max, float in_min, float out_max, float out_min)
{
    return out_min + (out_max - out_min) * (in - in_min) / (float) (in_max - in_min);
}

