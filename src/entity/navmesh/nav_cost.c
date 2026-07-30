#include "nav_cost.h"
#include "nav_link.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define NAV_STEP_BASE  10
void nav_cost_init(nav_cost_field *cf, int cell_count) {
    cf->count = cell_count;
    cf->area = NULL;
    if (cell_count > 0) {
        cf->area = malloc((size_t)cell_count);
        if (cf->area) memset(cf->area, NAV_COST_NEUTRAL, (size_t)cell_count);
    }
}

void nav_cost_free(nav_cost_field *cf) {
    free(cf->area);
cf->area = NULL;
cf->count = 0;
}

void nav_cost_clear(nav_cost_field *cf) {
    if (cf->area) memset(cf->area, NAV_COST_NEUTRAL, (size_t)cf->count);
}

void nav_cost_brush(nav_cost_field *cf, const nav_grid *g,
                    vec3 p, float radius, uint8_t cost) {
    if (!cf->area || radius <= 0.0f) return;
float r2 = radius * radius;
for (int i = 0;
i < g->count && i < cf->count;
i++) {
        vec3 c = nav_cell_world(&g->cells[i]);
        float dx = c.x - p.x;
        float dz = c.z - p.z;
        if (dx * dx + dz * dz > r2) continue;
        // keep the worst (max) cost if bubbles overlap; cheaper to flee the
        // nastier hazard than to average them into mush.
        if (cost > cf->area[i]) cf->area[i] = cost;
    }
}

int nav_cost_link(const nav_cost_field *cf, const nav_filter *f,
                  const nav_grid *g, int ci, int li) {
    const nav_cell *c = &g->cells[ci];
if (li < 0 || li >= c->link_count) return NAV_COST_BLOCKED;
if (f && !nav_filter_allows(f, g, ci, li)) return NAV_COST_BLOCKED;
int to = c->link_to[li];
int kind = c->link_kind[li];
int base = NAV_STEP_BASE + c->link_cost[li] + nav_link_cost(kind);
int area = NAV_COST_NEUTRAL;
if (cf && cf->area && to < cf->count) area = cf->area[to];
if (area < 1) area = 1;
long scaled = (long)base * (long)area;
if (scaled >= NAV_COST_BLOCKED) scaled = NAV_COST_BLOCKED - 1;
return (int)scaled;
}

int nav_cost_heuristic(const nav_grid *g, int a, int b) {
    const nav_cell *ca = &g->cells[a];
    const nav_cell *cb = &g->cells[b];

    int dx = abs(ca->x - cb->x);
    int dz = abs(ca->z - cb->z);
    int dy = abs(ca->y - cb->y);

    // octile distance on the horizontal plane: cheaper diagonals. ~14 per
    // diagonal, ~10 per straight, matching NAV_STEP_BASE. add the vertical
    // gap at the straight rate so climbing isn't free.
    int lo = dx < dz ? dx : dz;
    int hi = dx < dz ? dz : dx;
    return 14 * lo + 10 * (hi - lo) + 10 * dy;
}
