#ifndef RENDER_SHADOW_STABILIZE_H
#define RENDER_SHADOW_STABILIZE_H

#include "shadow_types.h"

// temporal stabilization. two separate problems live here:
//
// 1. the sun direction is driven by the day/night clock and moves a hair
// each frame. feeding that raw into the matrices makes the shadow edges
// crawl. we slew the light direction toward its target instead.
//
// 2. at a cascade boundary the resolution jumps, which shows as a visible
// seam. we compute a blend band around each split so the shader can
// cross-fade between cascade i and i+1 instead of hard-switching.

typedef struct {
    vec3  smoothed_dir;   // the slewed light direction we actually use
    float slew_rate;      // how fast we chase the target dir, per second
    int   primed;         // false until the first update seeds smoothed_dir
} shadow_stabilizer;

void shadow_stabilize_init(shadow_stabilizer *st, float slew_rate);

// advance toward target_dir by dt and return the stabilized direction. pass
// this result into shadow_update instead of the raw sun direction.
vec3 shadow_stabilize_dir(shadow_stabilizer *st, vec3 target_dir, float dt);

// half-width of the blend band at split index i (1..count-1), in view-space
// world units. derived from the gap between neighbouring splits.
float shadow_stabilize_band(const shadow_csm *csm, int split);

// blend weight in [0,1] for a fragment at view_depth sitting near split i:
// 0 = fully in cascade i, 1 = fully in cascade i+1.
float shadow_stabilize_blend(const shadow_csm *csm, int split, float view_depth);

#endif
