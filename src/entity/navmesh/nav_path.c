#include "nav_path.h"
#include "nav_query.h"
#include "nav_region.h"
#include "../../util/darray.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    int   parent;     // cell idx we came from, -1 = none/start
    int   g;          // best known cost from start
    int   in_open;
    int   closed;
} sn;
static int heuristic(const nav_cell *a, const nav_cell *b) {
    int dx = abs(a->x - b->x);
    int dz = abs(a->z - b->z);
    int dy = abs(a->y - b->y);
    int lo = dx < dz ? dx : dz;
    int hi = dx < dz ? dz : dx;
    // 10 per straight, 14 per diagonal, plus a small pull toward goal height.
    return 14 * lo + 10 * (hi - lo) + 2 * dy;
}

// pull the lowest-f open cell out of the frontier. the grids here are a few
// thousand cells worst case and searches usually touch far fewer, so a linear
// scan over the open list beats the bookkeeping of a heap. (pf_* uses a heap
// because it searches a denser block grid;
here the link graph is sparse.)
static int pop_min(int *open, sn *sx, const nav_cell *cells, int goal) {
    int best = -1, best_f = 0x7fffffff, best_at = -1;
    const nav_cell *gc = &cells[goal];
    for (int i = 0; i < (int)darr_len(open); i++) {
        int c = open[i];
        if (!sx[c].in_open) continue;     // lazily deleted
        int f = sx[c].g + heuristic(&cells[c], gc);
        if (f < best_f) { best_f = f; best = c; best_at = i; }
    }
    if (best_at >= 0) sx[best].in_open = 0;
    return best;
}

// stitch the parent chain into world-space waypoints, start->goal order.
static void reconstruct(nav_grid *g, sn *sx, int goal, nav_path *out) {
    int chain[NAV_PATH_MAX];
int n = 0;
for (int c = goal;
c >= 0 && n < NAV_PATH_MAX;
c = sx[c].parent)
        chain[n++] = c;
out->count = 0;
for (int i = n - 1;
i >= 0 && out->count < NAV_PATH_MAX;
i--)
        out->pts[out->count++] = nav_cell_world(&g->cells[chain[i]]);
out->cursor = 0;
out->found  = 1;
}

int nav_path_find(nav_grid *g, int start, int goal, nav_path *out) {
    memset(out, 0, sizeof *out);
    if (start < 0 || goal < 0 || start >= g->count || goal >= g->count)
        return 0;
    if (start == goal) {
        out->pts[0] = nav_cell_world(&g->cells[start]);
        out->count  = 1;
        out->found  = 1;
        return 1;
    }
    // region reject: if they're not on the same island, skip the whole search.
    if (!nav_region_connected(g, start, goal)) return 0;

    sn *sx = malloc(sizeof(sn) * g->count);
    for (int i = 0; i < g->count; i++) {
        sx[i].parent = -1;
        sx[i].g = 0x7fffffff;
        sx[i].in_open = 0;
        sx[i].closed = 0;
    }

    int *open = NULL;
    sx[start].g = 0;
    sx[start].in_open = 1;
    darr_push(open, start);

    int found = 0;
    while (!darr_empty(open)) {
        int cur = pop_min(open, sx, g->cells, goal);
        if (cur < 0) break;                 // open exhausted (all lazily deleted)
        if (cur == goal) { found = 1; break; }
        sx[cur].closed = 1;

        nav_cell *c = &g->cells[cur];
        for (int i = 0; i < c->link_count; i++) {
            int nb = c->link_to[i];
            if (sx[nb].closed) continue;
            int ng = sx[cur].g + c->link_cost[i];
            if (ng < sx[nb].g) {
                sx[nb].g = ng;
                sx[nb].parent = cur;
                if (!sx[nb].in_open) {
                    sx[nb].in_open = 1;
                    darr_push(open, nb);    // may double-list; pop skips stale
                }
            }
        }
    }

    if (found) reconstruct(g, sx, goal, out);

    darr_free(open);
    free(sx);
    return found;
}

int nav_path_find_pos(nav_grid *g, vec3 from, vec3 to, nav_path *out) {
    int s = nav_query_nearest(g, from, 2);
int t = nav_query_nearest(g, to, 3);
