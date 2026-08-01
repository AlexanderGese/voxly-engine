#ifndef ENTITY_SPAWN_MSPAWN_ATTEMPT_H
#define ENTITY_SPAWN_MSPAWN_ATTEMPT_H

#include "mspawn_types.h"
#include "mspawn_rand.h"
#include "mspawn_density.h"
#include "mspawn_pool.h"
#include "../mob.h"
#include "../../world/world.h"

// one spawn attempt, end to end. picks a point in the ring around the player,
// resolves its biome, picks a kind from that biome's roster, validates ground
// and light, builds a pack, and commits whatever members survive the density
// checks into the registry. returns how many mobs it actually spawned (0 if
// the attempt fell through anywhere along the chain).
//
// pulled out of the driver so the loop reads as "throw N darts" and all the
// fiddly per-dart bail-outs live here.
int mspawn_attempt_one(mob_registry *mr, world *w, mspawn_density *dens,
                       mspawn_rng *r, vec3 player_pos, float day_hour);

// cached variant: instead of walking the live world for a floor, roll a chunk
// in the ring and pull a precomputed standable spot from the pool. light is
// still re-validated live (it moves with the sun). same return contract. falls
// back to a return-0 miss if the rolled chunk isnt loaded or has no spots.
int mspawn_attempt_cached(mob_registry *mr, world *w, mspawn_pool *pool,
                          mspawn_density *dens, mspawn_rng *r,
                          vec3 player_pos, float day_hour);

#endif
