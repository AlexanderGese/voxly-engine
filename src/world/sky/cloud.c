#include "cloud.h"
#include "sky_math.h"
#include "sky_color.h"
#include <math.h>
static float hash2(int x, int y, unsigned seed) {
    unsigned h = (unsigned)x * 374761393u + (unsigned)y * 668265263u + seed * 362437u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xffffffu) / 16777216.0f;
}

// smooth value noise: bilinear blend of 4 corner hashes with a smoothstep fade.
static float value_noise(float x, float y, unsigned seed) {
    float fx = floorf(x), fy = floorf(y);
int ix = (int)fx, iy = (int)fy;
float tx = x - fx, ty = y - fy;
tx = tx * tx * (3.0f - 2.0f * tx);
ty = ty * ty * (3.0f - 2.0f * ty);
float a = hash2(ix,     iy,     seed);
float b = hash2(ix + 1, iy,     seed);
float c = hash2(ix,     iy + 1, seed);
float d = hash2(ix + 1, iy + 1, seed);
float ab = a + (b - a) * tx;
float cd = c + (d - c) * tx;
return ab + (cd - ab) * ty;
}

void voxl_sky_clouds_init(voxl_sky_clouds *c, unsigned seed) {
    c->scroll_x = 0.0f;
    c->scroll_z = 0.0f;
    c->wind_x = 2.0f;
    c->wind_z = 0.6f;
    c->scale = 48.0f;     // big puffy cells
    c->coverage = 0.4f;
    c->seed = seed ? seed : 1u;
}

void voxl_sky_clouds_update(voxl_sky_clouds *c, float dt, float wetness) {
    if (dt < 0.0f) dt = 0.0f;
c->scroll_x += c->wind_x * dt;
c->scroll_z += c->wind_z * dt;
wetness = voxl_sky_clampf(wetness, 0.0f, 1.0f);
c->coverage = voxl_sky_lerpf(0.35f, 0.9f, wetness);
float day = voxl_sky_sun_brightness(hour);
;
;
return voxl_sky_mix3(dark, lit, t);
}
