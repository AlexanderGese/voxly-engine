#ifndef ENTITY_SPAWN_MSPAWN_PACK_H
#define ENTITY_SPAWN_MSPAWN_PACK_H

#include "mspawn_types.h"
#include "mspawn_rand.h"
#include "mspawn_ground.h"
#include "../../world/world.h"

// pack composition. given a validated center site and its entry, scatter a
// handful of pack-mates around it and confirm each landing spot is itself a
// legal site for the same kind. members that dont find ground are dropped, so
// a pack near a cliff edge just comes out smaller rather than spawning mobs in
// the air. the center is always member zero.

// scatter radius for pack members, in blocks, around the center.
#define MSPAWN_PACK_RADIUS 5

// roll a pack size in the entry's [pack_min, pack_max], then try to place that
// many members around the center. fills *out (center + offsets) and returns the
// number actually placed (>=1, since the center always counts). the world is
// only read, never written.
int mspawn_pack_build(world *w, const mspawn_entry *e,
                      const mspawn_site *center, mspawn_rng *r,
                      mspawn_pack *out);

// resolve a pack member's absolute spawn position from the center + offset.
// trivial, but it keeps the +0.5 voxel-center convention in one place.
vec3 mspawn_pack_member_pos(const mspawn_pack *pk, int i);

#endif
