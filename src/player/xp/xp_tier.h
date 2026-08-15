#ifndef PLAYER_XP_XP_TIER_H
#define PLAYER_XP_XP_TIER_H

// orb tiers. instead of spawning N tiny orbs for a big reward we pack the
// value into a small number of bigger orbs. each tier has a value, a render
// scale, and a tint index the renderer maps to a color.

#include "../../math/vec3.h"

typedef struct {
    int   value;      // xp granted when this orb is absorbed
    float radius;     // collision/visual radius
    int   tint;       // 0..N palette index, bigger = warmer
} xp_tier_info;

// look up tier metadata. tier is clamped into range.
const xp_tier_info *xp_tier_get(int tier);

// pick the largest tier whose value <= remaining. used to greedily split a
// reward into orbs. returns -1 when remaining hits zero.
int   xp_tier_pick(int remaining);

// render color for a tier's tint index, as an rgb vec3 in 0..1.
vec3  xp_tier_color(int tier);

#endif
