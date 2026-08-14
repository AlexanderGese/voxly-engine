#ifndef PLAYER_TOOLS_MINING_H
#define PLAYER_TOOLS_MINING_H

#include "tools_types.h"
#include "tools_item.h"
#include "tools_speed.h"
#include "tools_drop.h"
#include "../../world/block.h"
#include "../../math/rng.h"

// the mining session: per-block break progress driven by the dig math. this is
// the thing the player loop pokes every frame while LMB is held. it's a thin
// state machine layered on top of tools_speed + tools_durability + tools_drop.

typedef struct {
    int      active;
    int      x, y, z;          // block being dug, in world coords
    block_id block;            // cached so we notice if it changes under us
    float    progress;         // 0..1
    float    rate;             // cached progress-per-second for the current dig
    int      switched;         // target changed this frame, callers may want it
} mining_session;

void tools_mining_init(mining_session *m);

// (re)point the session at a block. resets progress and recomputes the rate
// from the current tool + environment. cheap to call every frame.
void tools_mining_begin(mining_session *m, const tool_item *t, block_id block,
                        int x, int y, int z, const dig_env *env);

// advance the dig by dt seconds while LMB is held on (x,y,z). returns a
// mine_result. on MINE_BROKE the caller should remove the block and call
// tools_mining_finish to get drops + apply wear. recomputes rate each call so
// swapping tools mid-dig behaves.
mine_result tools_mining_tick(mining_session *m, tool_item *t, block_id block,
                              int x, int y, int z, int holding, float dt,
                              const dig_env *env);

// resolve drops and apply tool wear for the block that just broke. fills `out`
// and returns awarded xp. also flips the session back to idle.
int  tools_mining_finish(mining_session *m, tool_item *t, rng *r, drop_list *out);

// crack-overlay stage 0..10, for the renderer. 0 == no overlay.
int  tools_mining_phase(const mining_session *m);

void tools_mining_cancel(mining_session *m);

#endif
