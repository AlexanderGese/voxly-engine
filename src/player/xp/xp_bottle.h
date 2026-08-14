#ifndef PLAYER_XP_XP_BOTTLE_H
#define PLAYER_XP_XP_BOTTLE_H

// "bottle o' enchanting": a thrown projectile that arcs, and on impact (or
// timeout) shatters into a small burst of xp orbs. this is the one xp source
// that's literally an item you throw, so it gets its own little physics.
//
// we keep a fixed ring of in-flight bottles. they're cheap and short-lived.

#include <stdint.h>

#include "../../math/vec3.h"

#define XP_BOTTLE_MAX        16
#define XP_BOTTLE_GRAVITY    -18.0f
#define XP_BOTTLE_LIFETIME   6.0f
#define XP_BOTTLE_MIN_XP     3
#define XP_BOTTLE_MAX_XP     11

typedef struct {
    vec3    pos;
    vec3    vel;
    float   age;
    float   spin;       // cosmetic, radians
    int     xp_value;   // rolled at throw time
    uint8_t alive;
} xp_bottle;

typedef struct {
    xp_bottle bottles[XP_BOTTLE_MAX];
    int       live;
    uint64_t  rng_state;  // tiny inline rng so we don't drag the math one in
} xp_bottle_set;

typedef struct xp_orb_pool xp_orb_pool;

void xp_bottle_init(xp_bottle_set *s, uint64_t seed);

// throw a bottle from `origin` along `dir` (need not be normalized). returns
// the slot, or -1 if the ring is full.
int  xp_bottle_throw(xp_bottle_set *s, vec3 origin, vec3 dir, float power);

// step all bottles. any that hit `ground_y` (a flat floor approximation) or
// time out shatter, spawning their xp as orbs into `pool`. returns the number
// that shattered this step.
int  xp_bottle_update(xp_bottle_set *s, xp_orb_pool *pool, float ground_y, float dt);

// live count, for the renderer.
int  xp_bottle_live(const xp_bottle_set *s);

#endif
