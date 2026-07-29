#ifndef ENTITY_MOBS_MOB_STATS_H
#define ENTITY_MOBS_MOB_STATS_H

#include "mob_common.h"

// per-kind tuning numbers. one row per voxl_mob_kind. read-only.

typedef struct {
    const char *name;
    int   max_health;
    float walk_speed;     // wander / amble speed (m/s)
    float run_speed;      // chase / flee speed (m/s)
    float sight_range;    // how far hostiles notice the player (m)
    float attack_reach;   // melee distance (m)
    int   attack_damage;  // hp removed per swing
    float width;          // collision box width
    float height;         // collision box height
} voxl_mob_spec;

// fetch the spec for a kind. always returns a valid pointer (clamps oob).
const voxl_mob_spec *voxl_mob_spec_get(voxl_mob_kind k);

// spawn-init a mob struct in place with the right defaults for its kind.
void voxl_mob_init(voxl_mob *m, voxl_mob_kind k, vec3 pos, uint32_t id);

#endif
