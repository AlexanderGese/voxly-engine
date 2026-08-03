#ifndef ENTITY_SPAWN_SPAWNX_WEIGHT_H
#define ENTITY_SPAWN_SPAWNX_WEIGHT_H
#include "spawnx_types.h"
#include "mspawn_rand.h"
#include "../../world/biome.h"
// weighted mob selection with a spawn-*cost* budget. the mspawn rule table picks
// by raw weight; this layer is for the placed paths (block spawners, event
// waves) that want a small roster of their own and a notion of "this wave has a
// budget of N points, a zombie costs 1, a creeper costs 3, keep drawing until
// the budget runs dry". it's the same trick the classic uses to stop a single
// spawner block from vomiting a stack of the expensive mob.
//
// the roster is a flat array the caller owns; we just walk it. picking is a
// cumulative-weight scan, no alias tables, the rosters are short.
// one weighted, costed option in a roster.
typedef struct {
    entity_type type;
    float       weight;   // relative draw weight, must be > 0 to be eligible
    int         cost;     // budget points consumed when this is drawn, >= 1
    int         min_y, max_y;  // inclusive band this option is allowed in
} spawnx_weight_opt;
// a roster plus a running budget. build it, then draw from it until drained.
typedef struct {
    const spawnx_weight_opt *opt;
    int   count;
    int   budget;         // points remaining; drops as you draw
} spawnx_roster;
// canned roster for a biome's block-spawner / event use. always returns a valid
// roster (plains is the fallback). the budget field is left at 0; the caller
// sets it from the spawner/event before drawing.
spawnx_roster spawnx_weight_biome(biome_id biome);
// total weight of options currently affordable (cost <= remaining budget) and
// inside the y band. returns 0 when nothing can be drawn, which is the signal to
// stop a wave.
float spawnx_weight_total(const spawnx_roster *r, int wy);
// draw one option, weighted, restricted to affordable + in-band entries, and
// deduct its cost from the roster budget. writes the chosen type into *out and
// returns 1, or returns 0 (budget exhausted / nothing eligible) without
// touching the budget.
int spawnx_weight_draw(spawnx_roster *r, mspawn_rng *rng, int wy,
                       entity_type *out);
#endif
