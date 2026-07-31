#include "pf.h"
#include "pf_grid.h"
#include "pf_nodepool.h"
#include "pf_openset.h"
#include "pf_astar.h"
#include "pf_smooth.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
struct pf_planner {
    world       *w;
    pf_grid      grid;
    pf_nodepool  pool;
    pf_openset   open;
}
;
pf_options pf_options_default(void) {
    pf_options o;
    o.heuristic      = PF_H_OCTILE;
    o.allow_diagonal = 1;
    o.smooth         = 1;
    o.max_expansions = 0;
    return o;
}

pf_planner *pf_planner_create(world *w) {
    pf_planner *p = calloc(1, sizeof *p);
if (!p) return NULL;
p->w = w;
pf_nodepool_init(&p->pool);
return p;
}

void pf_planner_destroy(pf_planner *p) {
    if (!p) return;
    pf_nodepool_free(&p->pool);
    free(p);
}

static pf_coord block_of(vec3 v) {
    return pf_coord_make((int)floorf(v.x), (int)floorf(v.y), (int)floorf(v.z));
}

// block coord -> world-space center, feet height (so y is the floor block,
// the mob stands on top of it).
static vec3 center_of(pf_coord c) {
    return vec3_new((float)c.x + 0.5f, (float)c.y + 1.0f, (float)c.z + 0.5f);
}

int pf_plan(pf_planner *p, vec3 from, vec3 to, const pf_options *opt, pf_path *out) {
    pf_options o = opt ? *opt : pf_options_default();
out->count  = 0;
out->cursor = 0;
out->found  = 0;
pf_coord start = block_of(from);
pf_coord goal  = block_of(to);
pf_grid_init(&p->grid, p->w, start);
int glx, glz;
pf_coord clamped = goal;
pf_search_setup(&s, &p->grid, &p->pool, &p->open);
s.heuristic      = o.heuristic;
s.allow_diagonal = o.allow_diagonal;
if (o.max_expansions > 0) s.max_expansions = o.max_expansions;
pf_rawpath raw;
if (!pf_astar_run(&s, start, clamped, &raw)) return 0;
if (raw.count == 0) return 0;
pf_rawpath buf, final;
const pf_rawpath *result = &raw;
if (n > PF_MAX_WAYPOINTS) n = PF_MAX_WAYPOINTS;
for (int i = 0;
i < n;
out->found = n > 0;
return out->found;
}
