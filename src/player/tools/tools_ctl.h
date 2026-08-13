#ifndef PLAYER_TOOLS_CTL_H
#define PLAYER_TOOLS_CTL_H

#include "tools_loadout.h"
#include "tools_mining.h"
#include "tools_haste.h"
#include "tools_xp.h"
#include "tools_stats.h"
#include "tools_spawn.h"
#include "../../world/world.h"
#include "../../world/item.h"
#include "../../math/rng.h"

// the one object the player loop owns for everything tools+mining. it bundles
// the held loadout, the active mining session, the xp pool and the session
// stats, and exposes a single per-frame "i'm looking at this block holding lmb"
// call that does the whole dance: dig, break, drops, wear, xp, telemetry.

typedef struct {
    tools_loadout  loadout;
    mining_session session;
    xp_state       xp;
    tools_stats    stats;
    dig_state      env;        // current environment, refreshed by the caller
    rng            rng;        // drives drops + unbreaking, seeded per player
    int            auto_tool;  // 1 = auto-switch to the best tool while aiming
} tools_ctl;

void tools_ctl_init(tools_ctl *c, uint64_t seed);

// refresh the dig environment (water/ground/haste/fatigue) before mining. the
// player loop fills `st` from physics + status effects each frame.
void tools_ctl_set_env(tools_ctl *c, dig_state st);

// the player just aimed at a (possibly different) block. recomputes auto-tool.
void tools_ctl_aim(tools_ctl *c, block_id block);

// per-frame mining step. `holding` is lmb-down. on a break it writes the block
// to BLOCK_AIR in the world, spawns drops into `iw`, banks xp, and returns 1.
// otherwise returns 0. `out_levels` (nullable) receives xp levels gained.
int  tools_ctl_mine(tools_ctl *c, world *w, item_world *iw,
                    int bx, int by, int bz, int holding, float dt,
                    int *out_levels);

// crack overlay stage 0..10 for the renderer, straight off the session.
int  tools_ctl_crack_phase(const tools_ctl *c);

#endif
