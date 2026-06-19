#ifndef WORLD_LOADER_PRIORITY_H
#define WORLD_LOADER_PRIORITY_H

#include "loader_types.h"
#include "loader_job.h"

// turns "this chunk at (cx,cz) needs job K" into a single integer priority. the
// queue is a min-heap so SMALLER means run-sooner. we want, roughly:
// - close chunks before far chunks
// - chunks the player is walking toward before ones behind them
// - cheap early stages (alloc/gen) slightly ahead of expensive late ones at the
// same distance, so the world "fills in" rather than popping one finished
// chunk at a time. felt much better in playtest.
// everything is integer math, no float in the hot path, because this gets called
// for every candidate chunk every frame.

// tuning knobs. exposed so the debug menu can poke them live.
typedef struct {
    int ring_weight;     // cost added per chunk of distance from the player
    int facing_bonus;    // priority shaved off chunks in the facing cone
    int facing_penalty;  // priority added to chunks directly behind
    int stage_weight;    // cost added per pipeline stage (favours early stages)
    int dot_num;         // facing cone: accept when dot*dot_den >= dot_num*lensq
    int dot_den;
} loader_priority_cfg;

loader_priority_cfg loader_priority_default(void);

// the score. lower = more urgent. `focus` gives player pos + heading.
int loader_priority_score(const loader_priority_cfg *cfg, loader_focus focus,
                          int cx, int cz, loader_job_kind kind);

// squared chunk-space distance from focus to (cx,cz). pulled out because the ring
// uses it too for its keep/drop decision.
int loader_priority_dist2(loader_focus focus, int cx, int cz);

#endif
