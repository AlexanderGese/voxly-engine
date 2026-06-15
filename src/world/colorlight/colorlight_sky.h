#ifndef WORLD_COLORLIGHT_SKY_H
#define WORLD_COLORLIGHT_SKY_H

#include "../time_of_day.h"
#include "colorlight_rgb.h"

// drives the blend module's sun tint off the day/night cycle. time_of_day
// already computes a float sun color + strength for the renderer; this just
// converts that to the 0..255 rgb colorlight wants and pushes it into blend
// once per frame. keeping it here means blend.c doesn't have to include the
// whole daynight facade.

// convert tod's float sun color (0..1, pre-strength) to a colorlight rgb,
// folding in sun_strength so dawn is both warm AND dim.
colorlight_rgb colorlight_sky_sun_tint(const time_of_day *t);

// the once-per-frame call: compute the tint and hand it to blend. after this,
// every colorlight_sample / colorlight_blend uses the new sky color.
void colorlight_sky_update(const time_of_day *t);

// a flat night ambient that follows the moon a touch blue. used as the blend
// ambient floor at deep night so caves and shadows aren't identical to noon.
colorlight_rgb colorlight_sky_night_ambient(const time_of_day *t);

#endif
