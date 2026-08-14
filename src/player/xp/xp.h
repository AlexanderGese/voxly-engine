#ifndef PLAYER_XP_XP_H
#define PLAYER_XP_XP_H

// the experience subsystem facade. owns the player's xp state, perk set,
// event log and the orb pool, and drives them in the right order each frame.
// the rest of the engine talks to this and (almost) nothing below it.
//
// lifecycle:
// xp_system *x = xp_create(seed);
// ... on mob death:  xp_award_mob(x, mob_type, mob_pos);
// ... each frame:     int heal = xp_update(x, player_feet, dt);
// if (heal) damage_heal(dmg, heal);
// ... shutdown:       xp_destroy(x);

#include <stdint.h>

#include "../../math/vec3.h"
#include "xp_state.h"
#include "xp_perk.h"
#include "xp_event.h"
#include "xp_collect.h"
#include "xp_drop.h"
#include "xp_bottle.h"

typedef struct xp_orb_pool xp_orb_pool;

typedef struct {
    xp_state      state;
    xp_perk_set   perks;
    xp_event_log  log;
    xp_collect    collect;
    xp_orb_pool  *orbs;
    xp_bottle_set bottles;

    float merge_timer;   // throttles the O(n^2) merge pass
    int   total_levelups; // lifetime counter, for stats screens
} xp_system;

xp_system *xp_create(uint64_t seed);
void       xp_destroy(xp_system *x);

// award xp from a killed entity (looks up the mob value, spawns orbs).
void  xp_award_mob(xp_system *x, int entity_type, vec3 pos);

// award xp from a generic source (ore, smelt, breeding...).
void  xp_award(xp_system *x, int amount, vec3 pos, xp_source src);

// grant xp directly to the player, no orbs (debug / commands / bottle splash
// that landed right on you).
void  xp_grant_direct(xp_system *x, int amount, vec3 at);

// hurl a bottle o' enchanting from `origin` along `dir`. it arcs, then bursts
// into orbs on impact during xp_update. returns the bottle slot, or -1 if the
// in-flight ring is full.
int   xp_throw_bottle(xp_system *x, vec3 origin, vec3 dir, float power);

// run a frame: orb physics, merging, pickup, reward dispatch, event aging.
// returns total hp the player should be healed this frame (from rewards).
int   xp_update(xp_system *x, vec3 player_feet, float dt);

// resolved stat for consumers (move speed mult, reach add, etc).
// (named _value to dodge the xp_stat typedef in the same namespace.)
float xp_stat_value(const xp_system *x, xp_stat s);

// convenience accessors the hud wants.
int   xp_level(const xp_system *x);
float xp_progress(const xp_system *x);
int   xp_live_orbs(const xp_system *x);

#endif
