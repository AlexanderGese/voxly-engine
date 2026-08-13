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
#endif
