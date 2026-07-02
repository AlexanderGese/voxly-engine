#ifndef WORLD_WEATHERSIM_WIND_H
#define WORLD_WEATHERSIM_WIND_H

#include "weathersim_types.h"
#include "weathersim_field.h"

// the wind field. wind in voxl isn't a free fluid solve — it's derived from the
// pressure field each tick the way real geostrophic flow is: air wants to fall
// down the pressure gradient, but a coriolis-ish rotation deflects it so it
// ends up circling the lows instead of pouring straight in. that rotation is
// what makes fronts look like swirls rather than explosions.
//
// concretely: w = -k*grad(P) rotated 90deg, blended with the previous wind for
// inertia, plus the prevailing background drift. the rotation sign flips the
// circulation handedness; counter-clockwise around lows reads "right" for the
// northern-hemisphere-ish vibe the climate latitude band implies.

// recompute every cell's wind from the current pressure field. blends toward
// the new geostrophic estimate by `inertia` (0 = snap, 1 = frozen) so gusts
// ramp instead of teleporting. prevailing is the background drift added on top.
void weathersim_wind_solve(weathersim_field *f, vec2 prevailing,
                           float coriolis, float inertia);

// sample the wind at a fractional grid position with bilinear blend. handy for
// the public query, which lands between cell centers.
vec2 weathersim_wind_bilinear(const weathersim_field *f, float gx, float gz);

// magnitude of the strongest gust anywhere in the window, blocks/sec. cheap
// telemetry for the debug overlay / "it's getting blustery" sfx trigger.
float weathersim_wind_peak(const weathersim_field *f);

#endif
