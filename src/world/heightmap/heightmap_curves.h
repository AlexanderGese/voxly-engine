#ifndef WORLD_HEIGHTMAP_CURVES_H
#define WORLD_HEIGHTMAP_CURVES_H

#include "heightmap_spline.h"

// the built-in terrain spline tables. this is where the world's character
// actually lives. three named curves, all in [-1,1] input:
//
// continentalness: ocean floor -> coast -> inland -> far inland plateau.
// output is a height offset factor, mostly negative out at sea.
// erosion: how much relief survives. low erosion = jagged, high = worn
// flat. output is a 0..1 multiplier the pv spline gets scaled by.
// peaks/valleys: the actual ridges. a roughly W shaped curve so the noise
// spends most of its range near flat and only the extremes spike.
//
// the curves are filled lazily into a shared cache the first time anything
// asks. they never change at runtime, so we build once and hand out const
// pointers. if you want a different planet, this is the file to fork.

void heightmap_curves_init(void);   // idempotent, builds the tables once

const heightmap_spline *heightmap_curve_continental(void);
const heightmap_spline *heightmap_curve_erosion(void);
const heightmap_spline *heightmap_curve_peaks_valleys(void);

// convenience: run all three in one go and hand back the raw spline outputs.
// the column code wants them together more often than not.
void heightmap_curves_eval(float cont, float eros, float pv,
                           float *out_cont, float *out_eros, float *out_pv);

#endif
