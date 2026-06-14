#ifndef WORLD_CAVEGEN_WORM_H
#define WORLD_CAVEGEN_WORM_H

#include "cavegen_grid.h"
#include "cavegen_types.h"

// perlin worms. a worm is a turtle that walks a curving path through the rock
// and hollows out a sphere at every step. the heading is nudged by a noise
// field each step so the path snakes instead of going straight, hence "perlin
// worm". this is what makes the long ropey tunnels between the blobby automata
// rooms.

// spawn the region's worms. seeds are derived from the chunk coords so the same
// chunk always grows the same worms, and so a worm leaving one chunk re-enters
// the neighbour identically (it relies on the 1-cell pad overlap + shared seed).
void cavegen_worm_spawn_all(cavegen_grid *g, const cavegen_params *p);

// init a single worm at a position. exposed mostly so tests can drive one.
cavegen_worm cavegen_worm_init(vec3 pos, uint32_t stream, const cavegen_params *p);

// advance the worm one step: turn, move, carve. returns 0 when the worm is done
// (out of steps or wandered fully out of the region).
int  cavegen_worm_step(cavegen_grid *g, cavegen_worm *w, const cavegen_params *p);

// run a worm to completion.
void cavegen_worm_run(cavegen_grid *g, cavegen_worm *w, const cavegen_params *p);

#endif
