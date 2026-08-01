#ifndef ENTITY_PROJECTILE_RNG_H
#define ENTITY_PROJECTILE_RNG_H

#include "../../math/vec3.h"
#include "../../math/rng.h"

// thin shim over the engine rng for projectile-flavoured randomness. the world
// owns one of these so firing is deterministic from a seed (handy for replays
// and for the fuzzer that hammers the ballistics in isolation). nothing here
// allocates; it's just convenience math on top of rng.

typedef struct {
    rng r;
    uint64_t fired;     // monotone counter, also feeds projectile ids
} projectile_rng;

void projectile_rng_init(projectile_rng *pr, uint64_t seed);

// spread a unit heading into a cone of the given half-angle (radians). 0 spread
// returns dir unchanged. used to model bow accuracy and thrown wobble.
vec3 projectile_rng_cone(projectile_rng *pr, vec3 dir, float half_angle);

// small symmetric jitter on each axis, scaled by mag. for muzzle velocity slop.
vec3 projectile_rng_jitter(projectile_rng *pr, float mag);

// scalar in [lo,hi). just forwards to rng but keeps call sites local.
float projectile_rng_frange(projectile_rng *pr, float lo, float hi);

// next id from the fired counter. never returns 0 (0 is the invalid id).
uint32_t projectile_rng_next_id(projectile_rng *pr);

#endif
