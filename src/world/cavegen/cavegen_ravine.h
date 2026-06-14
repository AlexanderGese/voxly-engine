#ifndef WORLD_CAVEGEN_RAVINE_H
#define WORLD_CAVEGEN_RAVINE_H

#include "cavegen_grid.h"
#include "cavegen_types.h"
#include "../../math/vec3.h"

// ravines (canyons). rare, dramatic, tall thin slashes through the rock. they
// share the worm idea — a walker laying down volume — but the brush is a
// vertically stretched ellipse instead of a sphere, and the walker barely turns
// in pitch so the thing stays a near-vertical wall. you know the ones: you
// strip-mine sideways and suddenly theres a 40-block drop and your stuff is gone.

typedef struct {
    vec3   pos;
    float  yaw;        // heading in the xz plane only
    float  width;      // horizontal half-extent
    float  height;     // vertical half-extent (much bigger than width)
    int    steps_left;
    uint32_t rng;
} cavegen_ravine;

// roll for and possibly spawn a ravine in this region. ravines are gated hard so
// most chunks get none. returns 1 if one was carved.
int cavegen_ravine_maybe_spawn(cavegen_grid *g, const cavegen_params *p);

// init a ravine walker. exposed for tests.
cavegen_ravine cavegen_ravine_init(vec3 pos, uint32_t stream);

// advance one step. returns 0 when finished.
int cavegen_ravine_step(cavegen_grid *g, cavegen_ravine *r, const cavegen_params *p);

#endif
