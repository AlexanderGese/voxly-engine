#ifndef ENTITY_SPAWN_MSPAWN_RULES_H
#define ENTITY_SPAWN_MSPAWN_RULES_H

#include "mspawn_types.h"
#include "mspawn_rand.h"

// the spawn rule table. maps biome -> the set of mob entries that may appear
// there, and exposes a weighted pick that also respects the time band so we
// dont roll a zombie at noon then throw it away. the entry table itself is
// shared across biomes; each biome just lists indices into it with a per-biome
// weight multiplier, which keeps cows-everywhere from meaning cows-equally.

// one biome's roster: which entries are allowed and how strongly each leans.
typedef struct {
    const int   *entries;   // indices into the global mspawn_entry table
    const float *bias;      // per-entry weight multiplier, same length
    int          count;
} mspawn_roster;

// look up the static entry for a kind. returns NULL if the type isnt natural.
const mspawn_entry *mspawn_entry_for(entity_type t);

// the whole entry table, mostly for the density tracker to walk categories.
const mspawn_entry *mspawn_entry_table(int *out_count);

// roster for a biome. always valid (plains is the fallback).
mspawn_roster mspawn_roster_for(biome_id biome);

// is this entry allowed to appear right now given the clock? day_hour is 0..24.
int mspawn_time_ok(const mspawn_entry *e, float day_hour);

// weighted pick from a biome roster filtered by time band. writes the chosen
// entry into *out and returns 1, or returns 0 if nothing was eligible (e.g.
// a desert at noon with only nocturnal hostiles left in budget).
int mspawn_pick(biome_id biome, float day_hour, mspawn_rng *r,
                const mspawn_entry **out);

#endif
