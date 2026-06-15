#ifndef WORLD_EROSION_FLUX_H
#define WORLD_EROSION_FLUX_H
#include "erosion_types.h"
#include "erosion_field.h"
typedef struct {
    int   to[EROSION_CELLS];     // flat index of the downhill neighbour, -1 = pit
    float accum[EROSION_CELLS];  // upstream drainage area
    float wet[EROSION_CELLS];    // smoothed accum normalised to [0,1] for paint
} erosion_flux;
void erosion_flux_d8(const erosion_field *f, erosion_flux *fx);
void erosion_flux_accumulate(const erosion_field *f, erosion_flux *fx);
void erosion_flux_normalise(erosion_flux *fx);
int erosion_flux_is_channel(const erosion_flux *fx, int x, int z, float thresh);
void erosion_flux_compute(const erosion_field *f, erosion_flux *fx);
#endif
