#ifndef ENTITY_SPAWN_SPAWNX_COMMIT_H
#define ENTITY_SPAWN_SPAWNX_COMMIT_H

#include "spawnx_types.h"
#include "mspawn_rand.h"
#include "../mob.h"
#include "../../world/world.h"

// the single choke point every spawnx producer funnels through. block spawners,
// event waves, command spawns all build a spawnx_request and call commit. this
// is where we do the boring-but-load-bearing validation that mspawn already does
// for ambient spawns but the placed paths would otherwise each reimplement:
//
// - the feet block and the headroom above must be non-solid (no spawning a
// mob's head inside a wall),
// - the block under the feet must be solid (mob needs a floor), unless the
// request is for something that flies,
// - no other mob may already occupy that voxel (no stacking),
//
// only when all of that holds do we touch the registry. the registry is small
// (MAX_MOBS) so we never grow it; a full registry just fails the commit.

// how much clear air a standard mob needs above its feet block.
#define SPAWNX_HEADROOM 2

// is (wx,wy,wz) a legal feet voxel for a grounded mob? checks floor, feet and
// headroom. exposed because event ring placement wants to probe spots before it
// even builds a request.
int spawnx_spot_clear(world *w, int wx, int wy, int wz, int needs_floor);

// nudge a feet position down/up a couple blocks to find the nearest legal floor
// near `y_hint`. returns the resolved feet-y or -1 if nothing legal within the
// search band. keeps placed spawns from floating when the requester's y is off.
int spawnx_settle_y(world *w, int wx, int y_hint, int wz, int needs_floor);

// try to put one request into the world. fills *out. does not touch caps or
// anchors; the driver layers those on. returns out->placed for convenience.
int spawnx_commit(world *w, mob_registry *mr, const spawnx_request *req,
                  spawnx_result *out);

#endif
