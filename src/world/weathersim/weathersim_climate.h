#ifndef WORLD_WEATHERSIM_CLIMATE_H
#define WORLD_WEATHERSIM_CLIMATE_H

#include "weathersim_types.h"
#include "weathersim_field.h"

// the climate baseline: the slowly-varying temperature/humidity/pressure a cell
// would settle to with no fronts in play. this is what the field relaxes toward
// each tick, so the sky has a "normal" to return to instead of drifting forever.
//
// it's derived from the same low-frequency value-noise the biome picker uses in
// spirit (two octaves of hashed lattice), plus a latitude band so the far north
// stays cold. completely deterministic in (cell, seed): no state, no time. the
// daynight phase is folded in by the caller, not here — climate is the seasonal
// floor, diurnal swing rides on top.

typedef struct {
    float temp;       // baseline air temp, celsius-ish
    float humidity;   // baseline relative humidity 0..1
    float pressure;   // baseline pressure deviation from P_REF
    float moisture_src; // 0..1, how much this cell evaporates (water = high)
} weathersim_climate;

// sample the baseline at a global cell coord.
weathersim_climate weathersim_climate_sample(int cell_x, int cell_z,
                                             uint32_t seed);

// fill every unseeded cell (temp == NAN) in the field with its baseline. the
// rest are left alone so a recenter only pays for the freshly-exposed strip.
// pass force != 0 to reseed the whole window regardless. returns cells touched.
int weathersim_climate_seed_field(weathersim_field *f, uint32_t seed, int force);

#endif
