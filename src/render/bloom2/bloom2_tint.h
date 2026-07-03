#ifndef RENDER_BLOOM2_TINT_H
#define RENDER_BLOOM2_TINT_H
#include "../../math/vec3.h"
#include "bloom2_config.h"
typedef struct {
    vec3  near_tint;   // tint for the tightest mip (level 0)
    vec3  far_tint;    // tint for the widest mip (level count-1)
    float saturation;  // 0 = greyscale glow, 1 = full color, >1 = punchy
    int   enabled;
} bloom2_tint;
void bloom2_tint_default(bloom2_tint *t);
vec3 bloom2_tint_for_mip(const bloom2_tint *t, int level, int count);
#endif
