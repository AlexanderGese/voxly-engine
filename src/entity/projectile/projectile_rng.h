#ifndef ENTITY_PROJECTILE_RNG_H
#define ENTITY_PROJECTILE_RNG_H
#include "../../math/vec3.h"
#include "../../math/rng.h"
// thin shim over the engine rng for projectile-flavoured randomness. the world
// owns one of these so firing is deterministic from a seed (handy for replays
// and for the fuzzer that hammers the ballistics in isolation). nothing here
typedef struct {
    rng r;
    uint64_t fired;     // monotone counter, also feeds projectile ids
} projectile_rng;
void projectile_rng_init(projectile_rng *pr, uint64_t seed);
vec3 projectile_rng_cone(projectile_rng *pr, vec3 dir, float half_angle);
vec3 projectile_rng_jitter(projectile_rng *pr, float mag);
float projectile_rng_frange(projectile_rng *pr, float lo, float hi);
uint32_t projectile_rng_next_id(projectile_rng *pr);
#endif
