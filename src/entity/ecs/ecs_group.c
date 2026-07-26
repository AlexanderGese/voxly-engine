#include "ecs_group.h"

#include "../../util/darray.h"

void ecs_group_init(ecs_group *g, ecs_world *w,
                    ecs_signature require, ecs_signature exclude) {
    g->w           = w;
    g->require     = require;
    g->exclude     = exclude;
    g->members     = NULL;
    g->revision    = 0;
    g->built_count = 0;
    g->dirty       = 1;     // force the first refresh to actually build
}

void ecs_group_free(ecs_group *g) {
    darr_free(g->members);
    g->w = NULL;
}

void ecs_group_invalidate(ecs_group *g) {
    g->dirty = 1;
}

static void rebuild(ecs_group *g, uint32_t world_revision) {
    darr_clear(g->members);
    ecs_query q;
    ecs_query_begin(&q, g->w, g->require, g->exclude);
    while (ecs_query_next(&q))
        darr_push(g->members, q.e);

    g->revision    = world_revision;
    g->built_count = ecs_count(g->w);
    g->dirty       = 0;
}

int ecs_group_refresh(ecs_group *g, uint32_t world_revision) {
    // rebuild when explicitly dirtied, when the caller's revision advanced, or
    // when the live entity count drifted from what we built against (a despawn
    // we werent told about). the count check is what catches silent reaps from
    // the health/lifetime systems.
    if (g->dirty ||
        world_revision != g->revision ||
        ecs_count(g->w) != g->built_count) {
        rebuild(g, world_revision);
        return 1;
    }
    return 0;
}

uint32_t ecs_group_count(const ecs_group *g) {
    return (uint32_t)darr_len(g->members);
}

ecs_entity ecs_group_at(const ecs_group *g, uint32_t i) {
    if (i >= darr_len(g->members)) return ECS_NULL;
    return g->members[i];
}

void ecs_group_each(ecs_group *g, ecs_group_visit_fn fn, void *ctx) {
    size_t n = darr_len(g->members);
    for (size_t i = 0; i < n; i++) {
        ecs_entity e = g->members[i];
        // the cache can outlive a member by a frame; gate on liveness so the
        // callback never sees a dangling handle.
        if (!ecs_alive(g->w, e)) continue;
        fn(g->w, e, ctx);
    }
}
