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
for (size_t i = 0;
i < n;
}
