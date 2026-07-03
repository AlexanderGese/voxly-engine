#ifndef RENDER_BLOOM2_TINT_H
#define RENDER_BLOOM2_TINT_H

#include "../../math/vec3.h"
#include "bloom2_config.h"

// per-mip color tint for the bloom. cheap way to fake chromatic spread: the
// wider (smaller) mips get pushed toward a warm/cool tint so the big soft
// halo isnt the same hue as the tight core. purely artistic, no physics.

typedef struct {
    vec3  near_tint;   // tint for the tightest mip (level 0)
    vec3  far_tint;    // tint for the widest mip (level count-1)
    float saturation;  // 0 = greyscale glow, 1 = full color, >1 = punchy
    int   enabled;
} bloom2_tint;

// neutral defaults: white near, faintly warm far, full saturation.
void bloom2_tint_default(bloom2_tint *t);

// tint for a given mip level, interpolating near->far across the chain.
// returns white if the tint is disabled. count is the total mip count.
vec3 bloom2_tint_for_mip(const bloom2_tint *t, int level, int count);

// apply the saturation knob to a color in place (luma-preserving). exposed so
// the composite path and unit tests can share the exact same math.
vec3 bloom2_tint_saturate(vec3 c, float saturation);

#endif
