#ifndef ENTITY_VILLAGER_BRAIN_H
#define ENTITY_VILLAGER_BRAIN_H

#include "villager.h"
#include "villager_pathing.h"
#include "villager_poi.h"
#include "../../world/world.h"
#include "../../math/vec3.h"

// the brain: per-tick logic that reads the schedule, picks an activity, and
// drives movement + work via the nav layer and poi registry. one call per
// villager per frame.
//
// inputs the brain needs from outside:
// - day_t: normalized day clock in [0,1), 0 = dawn (from the daynight sys)
// - threat: world position of something scary (e.g. the player when
// aggro'd, or a nearby monster), and whether it's active.

typedef struct {
    float  day_t;
    vec3   threat_pos;
    int    threat_active;
    vec3   bell_pos;        // village center fallback for GATHER
    int    have_bell;
} villager_brain_ctx;

// advance one villager. mutates the villager and may claim/release POIs.
// `nav` is the villager's persistent navigation state (caller-owned, 1:1).
void villager_brain_tick(villager *v, villager_nav *nav,
                         villager_poi_set *pois, world *w,
                         const villager_brain_ctx *ctx, float dt);

#endif
