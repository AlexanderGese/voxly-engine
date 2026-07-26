#ifndef ENTITY_ECS_GROUP_H
#define ENTITY_ECS_GROUP_H

#include "ecs_world.h"
#include "ecs_query.h"

// a cached query. ecs_query walks a store fresh every time, which is fine for a
// one-off but wasteful for a system that runs the *same* require/exclude every
// frame for the whole game. a group holds the matched-entity list and only
// rebuilds it when something might have changed. think of it as a materialised
// view over the world.
//
// we dont have change-tracking hooks deep in the store (didnt want the write
// barrier on every ecs_add), so "might have changed" is coarse: the group
// stamps the world's entity count + a user-bumped revision when it builds, and
// rebuilds if either moved. cheap, and good enough -- worst case we rebuild a
// frame we didnt strictly need to.

typedef struct {
    ecs_world    *w;
    ecs_signature require;
    ecs_signature exclude;

    ecs_entity   *members;     // darray of matched entities
    uint32_t      revision;    // last revision we built against
    uint32_t      built_count; // world entity count at last build
    int           dirty;       // force rebuild next refresh
} ecs_group;

void ecs_group_init(ecs_group *g, ecs_world *w,
                    ecs_signature require, ecs_signature exclude);
void ecs_group_free(ecs_group *g);

// mark the cache stale. call after a batch of spawns/despawns you know touched
// the group, or just bump the world revision and let refresh notice.
void ecs_group_invalidate(ecs_group *g);

// rebuild if needed against `world_revision` (any monotonically increasing stamp
// the caller controls -- e.g. a frame counter, or a spawn counter). returns 1 if
// it actually rebuilt this call, 0 if the cache was still good.
int  ecs_group_refresh(ecs_group *g, uint32_t world_revision);

uint32_t   ecs_group_count(const ecs_group *g);
ecs_entity ecs_group_at(const ecs_group *g, uint32_t i);

// fold over the cached members. the callback gets each live member + ctx; dead
// members (reaped since last build) are skipped so stale handles never escape.
typedef void (*ecs_group_visit_fn)(ecs_world *w, ecs_entity e, void *ctx);
void ecs_group_each(ecs_group *g, ecs_group_visit_fn fn, void *ctx);

#endif
