#ifndef ENTITY_VILLAGER_DEF_H
#define ENTITY_VILLAGER_DEF_H

#include "villager_types.h"
#include "../../world/block.h"

// static per-profession data. one row per profession, looked up by enum.
// kept as a flat table because it never changes at runtime and i like being
// able to read the whole roster in one screen.

typedef struct {
    const char *name;
    block_id    station_block;   // the block this profession claims as work
    int         work_radius;     // how far it'll wander from its station
    int         restock_amount;  // trades refilled per work session
    float       walk_speed;      // blocks/sec; clerics dawdle, smiths march
    uint8_t     base_offers;     // how many trade slots unlock at novice
} villager_def;

// fetch the row for a profession. always returns a valid pointer (clamps).
const villager_def *villager_def_get(villager_profession p);

// reverse lookup: given a workstation block, which profession claims it?
// returns VILLAGER_PROF_COUNT if no profession wants this block.
villager_profession villager_def_for_block(block_id b);

// pretty name, never NULL.
const char *villager_def_name(villager_profession p);

#endif
