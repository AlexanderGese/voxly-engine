#ifndef ENTITY_VILLAGER_VILLAGER_H
#define ENTITY_VILLAGER_VILLAGER_H

#include "villager_types.h"
#include "villager_schedule.h"
#include "villager_trade.h"
#include "villager_gossip.h"
#include "../../math/vec3.h"
#include "../../math/aabb.h"
#include "../../math/rng.h"
#include <stdint.h>

// one villager. owns its identity, current activity state, schedule, trades
// and reputation ledger. the brain (villager_brain.c) drives it; this file
// is just the data + cheap accessors.

typedef struct {
    uint32_t id;            // unique, nonzero. 0 means "no villager".
    villager_profession prof;
    int      level;         // mirrors trades.level for quick display
    int      is_baby;       // babies can't work; they grow up

    vec3     pos;
    vec3     vel;
    float    yaw;
    int      on_ground;

    int      hp;
    int      max_hp;
    float    hurt_timer;

    // ai bookkeeping
    villager_activity activity;
    float    act_timer;     // seconds spent in current activity
    float    repath_timer;  // throttle for expensive pathfinding
    float    grow_timer;    // baby -> adult countdown
    float    work_progress; // 0..1 toward one restock cycle while WORKING
    float    wander_dir;    // cached heading for idle wander

    // poi claims (indices into the village poi set, -1 = none)
    int      bed_poi;
    int      work_poi;

    villager_schedule sched;
    villager_trades   trades;
    villager_gossip   gossip;

    rng      rng;           // per-villager noise so they don't sync up
} villager;

// spawn a fresh adult villager of a profession at pos. assigns a new id.
villager villager_make(uint32_t id, villager_profession prof, vec3 pos, uint64_t seed);

// spawn a baby. it'll grow up and pick a job once it claims a workstation.
villager villager_make_baby(uint32_t id, vec3 pos, uint64_t seed);

// re-assign a profession (e.g. after claiming a new workstation). rebuilds
// the schedule and trade table; resets level. nitwits refuse to change.
void villager_set_profession(villager *v, villager_profession prof);

aabb  villager_aabb(const villager *v);
int   villager_is_alive(const villager *v);

// apply damage; returns 1 if it killed the villager. also stamps hurt_timer
// so the brain can flip into PANIC.
int   villager_hurt(villager *v, int amount);

#endif
