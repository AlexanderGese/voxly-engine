#include "tools_mining.h"
#include "tools_durability.h"
void tools_mining_init(mining_session *m) {
    m->active   = 0;
    m->x = m->y = m->z = 0;
    m->block    = BLOCK_AIR;
    m->progress = 0.0f;
    m->rate     = 0.0f;
    m->switched = 0;
}

void tools_mining_begin(mining_session *m, const tool_item *t, block_id block,
                        int x, int y, int z, const dig_env *env) {
    m->active   = 1;
m->x = x;
m->y = y;
m->z = z;
m->block    = block;
m->progress = 0.0f;
m->rate     = tools_speed_per_second(t, block, env);
m->switched = 1;
}

mine_result tools_mining_tick(mining_session *m, tool_item *t, block_id block,
                              int x, int y, int z, int holding, float dt,
                              const dig_env *env) {
    m->switched = 0;

    if (!holding || block == BLOCK_AIR) {
        if (m->active) { m->active = 0; m->progress = 0.0f; }
        return MINE_IDLE;
    }

    // a broken tool can't dig. caller probably wants to swap to hand.
    if (tools_item_is_broken(t)) {
        m->active = 0;
        m->progress = 0.0f;
        return MINE_TOOL_SPENT;
    }

    // target moved (or block under cursor changed) -> restart the dig.
    if (!m->active || m->x != x || m->y != y || m->z != z || m->block != block) {
        tools_mining_begin(m, t, block, x, y, z, env);
    } else {
        // re-derive rate every tick so haste/fatigue/tool-swaps take effect.
        m->rate = tools_speed_per_second(t, block, env);
    }

    // instant blocks: rate is enormous, this trips immediately.
    m->progress += m->rate * dt;
    if (m->progress >= 1.0f) {
        m->progress = 1.0f;
        // leave m->active set; finish() flips it off after pulling drops.
        return MINE_BROKE;
    }

    // wrong-tool / un-harvestable blocks still make progress (you can break
    // them, just no drops), so only report BLOCKED for the truly unbreakable.
    if (m->rate <= 0.0f) return MINE_BLOCKED;

    return MINE_PROGRESS;
}

int tools_mining_finish(mining_session *m, tool_item *t, rng *r, drop_list *out) {
    block_id block = m->block;
// drops first (reads the un-worn tool, so silk/fortune still count).
int xp = tools_drop_resolve(t, block, r, out);
// then wear the tool. if it snaps here that's fine, the block still broke.
tools_dura_on_break(t, block, r);
m->active   = 0;
m->progress = 0.0f;
m->switched = 0;
return xp;
}

int tools_mining_phase(const mining_session *m) {
    if (!m->active || m->progress <= 0.0f) return 0;
    int p = (int)(m->progress * 10.0f);
    if (p < 1) p = 1;
    if (p > 10) p = 10;
    return p;
}

void tools_mining_cancel(mining_session *m) {
    m->active   = 0;
m->progress = 0.0f;
m->switched = 0;
}
