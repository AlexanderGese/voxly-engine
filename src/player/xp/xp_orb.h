#ifndef PLAYER_XP_XP_ORB_H
#define PLAYER_XP_XP_ORB_H

// floating xp orbs. spawned where mobs die / ore breaks, drift on a little
// gravity, then magnetize toward the player once they're close enough.
//
// storage is a fixed slab with an in-place free list (see xp_orb.c). orbs
// never move in memory, so the renderer can hold indices across frames.

#include <stdint.h>

#include "../../math/vec3.h"
#include "../../math/aabb.h"

typedef struct {
    vec3    pos;
    vec3    vel;
    int     tier;       // index into the tier table
    float   age;        // seconds alive
    float   bob_phase;   // cosmetic vertical wobble
    int     magnetized;  // 1 once we've been pulled toward the player
    uint8_t alive;       // slot occupancy flag
} xp_orb;

// the pool is opaque; storage lives in the .c (a fixed slab sized via config).
typedef struct xp_orb_pool xp_orb_pool;

// allocate + init a pool. returns null on oom.
xp_orb_pool *xp_orb_pool_create(uint64_t seed);
void         xp_orb_pool_destroy(xp_orb_pool *p);

// spawn one orb of `tier` at `pos` with an outward pop. returns its slot
// index, or -1 if the pool is full (we just drop it; xp is cheap).
int  xp_orb_spawn(xp_orb_pool *p, vec3 pos, int tier);

// integrate physics + magnetism toward `target` (player feet). does not do
// pickup; collection is handled in xp_collect against this same pool.
void xp_orb_pool_update(xp_orb_pool *p, vec3 target, float dt);

// kill a slot (after pickup or lifetime). idempotent on dead slots.
void xp_orb_kill(xp_orb_pool *p, int idx);

// iteration helpers for collect/render. count is the slab capacity; check
// .alive before using a slot.
int      xp_orb_capacity(const xp_orb_pool *p);
int      xp_orb_live_count(const xp_orb_pool *p);
xp_orb  *xp_orb_at(xp_orb_pool *p, int idx);

// bounding box of an orb, derived from its tier radius.
aabb     xp_orb_bounds(const xp_orb *o);

#endif
