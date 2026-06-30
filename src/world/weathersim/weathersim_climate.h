#ifndef WORLD_WEATHERSIM_CLIMATE_H
#define WORLD_WEATHERSIM_CLIMATE_H
#include "weathersim_types.h"
#include "weathersim_field.h"
typedef struct {
    float temp;       // baseline air temp, celsius-ish
    float humidity;   // baseline relative humidity 0..1
    float pressure;   // baseline pressure deviation from P_REF
    float moisture_src; // 0..1, how much this cell evaporates (water = high)
} weathersim_climate;
#endif
