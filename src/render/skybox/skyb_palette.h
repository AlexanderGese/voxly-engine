#ifndef RENDER_SKYBOX_PALETTE_H
#define RENDER_SKYBOX_PALETTE_H

// the color "script" for a day. a handful of keyed times-of-day, each with a
// zenith color, a horizon color and a sun-tint. we sample this by hour with a
// wrap-around catmull-ish blend so dawn/dusk get a soft push of warmth.
//
// the actual day/night sim lives in world/sky; this is purely the look. you
// could hot-swap palettes (alien planet, nether, whatever) without the sim
// caring.

#include "skyb_common.h"

typedef struct {
    float    hour;     // 0..24 key time
    skyb_rgb zenith;   // top of the dome
    skyb_rgb horizon;  // where dome meets ground
    skyb_rgb sun_tint; // multiplies the sun disc + scatter glow
} skyb_key;

#define SKYB_PALETTE_MAX 12

typedef struct {
    skyb_key keys[SKYB_PALETTE_MAX];
    int      count;
} skyb_palette;

// the sampled result for one moment.
typedef struct {
    skyb_rgb zenith;
    skyb_rgb horizon;
    skyb_rgb sun_tint;
} skyb_sky_colors;

// load the built-in earth-like day palette. overwrites *p.
void skyb_palette_default(skyb_palette *p);

// append a key, keeping the array sorted by hour. ignored if full.
void skyb_palette_add(skyb_palette *p, skyb_key k);

// sample the palette at an hour (wrapped). blends between the two bracketing
// keys with a smoothstep so transitions aren't linear-ugly.
skyb_sky_colors skyb_palette_sample(const skyb_palette *p, float hour);

#endif
