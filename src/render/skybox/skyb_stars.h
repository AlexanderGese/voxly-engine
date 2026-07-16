#ifndef RENDER_SKYBOX_STARS_H
#define RENDER_SKYBOX_STARS_H

// renderer-side star field. baked once from a seed onto the sky dome. unlike
// the world/sky starfield this one carries a color temperature and a magnitude
// so we can size the point sprites, and it bakes flat into a vertex buffer the
// render layer can upload directly.

#include "skyb_common.h"

#define SKYB_STAR_MAX 1024

typedef struct {
    vec3     dir;       // unit dir on the dome
    float    mag;       // visual magnitude proxy 0..1 (1 = brightest)
    float    size;      // point size in px before distance scaling
    float    twinkle;   // twinkle rate
    float    phase;     // per-star phase offset so they don't sync
    skyb_rgb color;     // baked star color (white..blue..warm)
} skyb_star;

typedef struct {
    skyb_star star[SKYB_STAR_MAX];
    int       count;
    unsigned  seed;
} skyb_starfield;

// the flat per-star vertex we hand to gl. one point per star.
typedef struct {
    float x, y, z;   // dir * dome radius (filled by render layer)
    float r, g, b;   // color premultiplied by current brightness
    float size;      // final point size
} skyb_star_vertex;

// bake `count` stars (clamped) from seed. magnitude follows a power law so most
// stars are dim and a few are bright, which reads more naturally.
void skyb_starfield_bake(skyb_starfield *sf, int count, unsigned seed);

// overall star visibility 0..1 for the hour (day kills it, night = full).
float skyb_star_visibility(float hour);

// current brightness of one star: visibility * magnitude * twinkle. 0..1.
float skyb_star_brightness(const skyb_star *s, float vis, float time_s);

// fill `out` (must hold sf->count entries) with renderable vertices for the
// current moment. returns how many were written (skips fully-dark stars).
// `radius` places them on the dome. `min_b` culls anything dimmer.
int skyb_starfield_emit(const skyb_starfield *sf, skyb_star_vertex *out,
                        float hour, float time_s, float radius, float min_b);

#endif
