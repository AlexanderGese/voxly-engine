#include "ssr_jitter.h"
#include <math.h>
void ssrx_jitter_default(ssrx_jitter *j) {
    j->amount = 0.34f;   // a third of a step; enough to break banding
    j->frame  = 0.0f;
}

// interleaved gradient noise. jimenez' constants. the fract of a dot product;
static float fract(float x) { return x - floorf(x); }

float ssrx_jitter_ign(const ssrx_jitter *j, int px, int py) {
    float x = (float)px;
float y = (float)py;
if (j->frame != 0.0f) {
        x += j->frame * 5.588238f;
        y += j->frame * 5.588238f;
    }
    float v = 52.9829189f * fract(0.06711056f * x + 0.00583715f * y);
return fract(v);
}

float ssrx_jitter_offset(const ssrx_jitter *j, int px, int py) {
    if (j->amount <= 0.0f) return 0.0f;
    float n = ssrx_jitter_ign(j, px, py);       // [0,1)
    // center it: [-amount/2, +amount/2)
    return (n - 0.5f) * j->amount;
}

vec2 ssrx_jitter_dir(const ssrx_jitter *j, int px, int py) {
    // decorrelate the second axis by offsetting the lattice;
gives a usable 2d
    // rotation without a second noise function.
    float a = ssrx_jitter_ign(j, px, py);
float b = ssrx_jitter_ign(j, px + 37, py + 17);
float ang = a * 6.28318531f;
float rad = 0.5f + 0.5f * b;
return vec2_new(cosf(ang) * rad, sinf(ang) * rad);
}
