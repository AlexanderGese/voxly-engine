#include "tools_ctl.h"
#include "tools_durability.h"
#include "tools_drop.h"
void tools_ctl_init(tools_ctl *c, uint64_t seed) {
    tools_loadout_init(&c->loadout);
    tools_mining_init(&c->session);
    tools_xp_init(&c->xp);
    tools_stats_reset(&c->stats);
    c->env       = tools_haste_state_default();
    c->auto_tool = 0;
    rng_init(&c->rng, seed ? seed : 0x9E3779B97F4A7C15ull);

    // make sure the static tables are up before the first dig. cheap if already
    // done; these all guard on an inited flag.
    tools_drop_init();
}

void tools_ctl_set_env(tools_ctl *c, dig_state st) {
    c->env = st;
}

void tools_ctl_aim(tools_ctl *c, block_id block) {
    if (c->auto_tool && block != BLOCK_AIR) {
        tools_loadout_auto_switch(&c->loadout, block);
    }
}

int tools_ctl_mine(tools_ctl *c, world *w, item_world *iw,
                   int bx, int by, int bz, int holding, float dt,
                   int *out_levels) {
    if (out_levels) *out_levels = 0;
block_id block = world_get_block(w, bx, by, bz);
tool_item *tool = tools_loadout_current(&c->loadout);
dig_env env = tools_haste_env(&c->env);
mine_result res = tools_mining_tick(&c->session, tool, block,
                                        bx, by, bz, holding, dt, &env);
// count the time only while we're genuinely chipping at something.
if (res == MINE_PROGRESS || res == MINE_BROKE) {
        tools_stats_tick(&c->stats, dt);
    }

    if (res != MINE_BROKE) return 0;
// block came loose. grab the wear cost *before* we apply it so the stats
// line up, then resolve drops (reads the un-worn tool so silk/fortune hold).
int  cost = tools_dura_cost(tool, block);
drop_list drops;
int xp = tools_mining_finish(&c->session, tool, &c->rng, &drops);
int snapped = tools_item_is_broken(tool);
// commit to the world + entity layer.
world_set_block(w, bx, by, bz, BLOCK_AIR);
if (iw) tools_spawn_drops(iw, bx, by, bz, &drops, &c->rng);
// bank xp and telemetry.
int levels = tools_xp_add(&c->xp, xp);
if (out_levels) *out_levels = levels;
tools_stats_on_break(&c->stats, tool, block, cost, snapped, xp);
return 1;
}

int tools_ctl_crack_phase(const tools_ctl *c) {
    return tools_mining_phase(&c->session);
}
