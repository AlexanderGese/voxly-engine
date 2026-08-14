#ifndef PLAYER_XP_XP_COLLECT_H
#define PLAYER_XP_XP_COLLECT_H

// the pickup pass: scan the orb pool, absorb any orb within pickup range of
// the player, credit the xp_state, and post gain events. also coalesces
// nearby same-ish orbs so big bursts don't lag the collision scan.

#include "../../math/vec3.h"
#include "xp_state.h"
#include "xp_event.h"

// pool stays opaque here; we only ever pass the handle through.
typedef struct xp_orb_pool xp_orb_pool;

typedef struct {
    int   orbs_absorbed; // this pass
    int   xp_gained;     // this pass
    float pickup_cooldown; // tiny gate so the gain sound doesn't machine-gun
} xp_collect;

void xp_collect_init(xp_collect *c);

// run a collection pass against `pool` for a player at `feet`. credits
// `state` and optionally logs to `log` (may be null). returns xp gained.
int  xp_collect_run(xp_collect *c, xp_orb_pool *pool, vec3 feet,
                    xp_state *state, xp_event_log *log, float dt);

// merge pass: orbs of the same tier within XP_ORB_MERGE_RANGE get promoted
// into a single higher-tier orb when possible. returns merges performed.
int  xp_collect_merge(xp_orb_pool *pool);

#endif
