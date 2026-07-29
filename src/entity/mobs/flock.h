#ifndef ENTITY_MOBS_FLOCK_H
#define ENTITY_MOBS_FLOCK_H

#include "mob_common.h"
#include "../../math/vec3.h"

// dead-simple boids so herd animals clump up. operates on an array of mob
// pointers (only ones of the same kind should be passed in together).

// tuning knobs for a flocking pass. radii in metres.
typedef struct {
    float neighbor_radius;   // who counts as a neighbor
    float separation_dist;   // start pushing apart closer than this
    float cohesion_weight;   // pull toward the herd center
    float alignment_weight;  // match the herd heading
    float separation_weight; // shove off close neighbors
    float max_speed;         // clamp the resulting steer
} voxl_flock_params;

// sensible defaults.
voxl_flock_params voxl_flock_default_params(void);

// run one flocking step over `mobs[0..count)`. nudges each mob's velocity
// (xz only) toward herd behavior. does not integrate position; the per-mob
// update / physics still moves them.
void voxl_flock_step(voxl_mob **mobs, int count,
                     const voxl_flock_params *p, float dt);

// compute the average position of a herd (y included). count must be > 0.
vec3 voxl_flock_center(voxl_mob **mobs, int count);

#endif
