#ifndef WORLD_OREGEN_CURVE_H
#define WORLD_OREGEN_CURVE_H

#include "oregen_types.h"

// depth-band density curves. given a world y and an ore's band, return a
// weight in [0,1] describing how much that ore likes to be there. the
// seeder multiplies its base spawn chance by this, so band edges fade out
// instead of clipping. all pure functions, no rng.

// normalized position of y inside [y_min,y_max], clamped to [0,1].
// returns 0 outside the band on the low side, 1 on the high side, but
// callers should gate on oregen_curve_in_band first.
float oregen_curve_t(int y, int y_min, int y_max);

// 1 if y is inside the inclusive band, else 0.
int   oregen_curve_in_band(int y, int y_min, int y_max);

// evaluate a curve shape at normalized t in [0,1]. returns weight [0,1].
float oregen_curve_eval(oregen_curve curve, float t);

// convenience: weight for an ore at a given world y. 0 if out of band.
float oregen_curve_weight(const oregen_ore *ore, int y);

// integrate the curve across the band in whole-y steps. used to normalize
// per-chunk try counts so a wide band and a narrow band with the same
// tries_per_chunk actually place comparable amounts. returns the average
// weight over the band (so multiply tries by band_height * this if you
// want the absolute expected count).
float oregen_curve_band_mean(const oregen_ore *ore);

#endif
