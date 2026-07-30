#include "nav_flow.h"
#include "../../util/darray.h"

#include <stdlib.h>
#include <limits.h>

void nav_flow_init(nav_flow *f) {
    f->dist = NULL;
    f->next = NULL;
    f->count = 0;
    f->goal = -1;
}

void nav_flow_free(nav_flow *f) {
    free(f->dist);
    free(f->next);
    nav_flow_init(f);
}

// grow the owned arrays to fit `n` cells, reallocating only when the grid got
// bigger. flow fields get rebuilt every time the goal moves so reuse matters.
static int ensure(nav_flow *f, int n) {
    if (n > f->count) {
        int *nd = realloc(f->dist, sizeof(int) * n);
        int *nn = realloc(f->next, sizeof(int) * n);
        if (!nd || !nn) { free(nd); free(nn); return 0; }
        f->dist = nd;
        f->next = nn;
    }
    f->count = n;
    return 1;
}

// pull the unvisited cell with the smallest tentative distance. same linear
// frontier nav_path uses — the graphs are sparse and small, a heap isn't worth
// it. `open` is a darray of candidate indices, lazily compacted by the visited
// flag living in dist (== -1 sentinel would clash, so we use a side array).
static int pop_min(const int *open, const int *dist, const char *done) {
    int best = -1, best_d = INT_MAX;
    for (int i = 0; i < (int)darr_len(open); i++) {
        int c = open[i];
        if (done[c]) continue;
        if (dist[c] < best_d) { best_d = dist[c]; best = c; }
    }
    return best;
}

int nav_flow_build(nav_flow *f, nav_grid *g, int goal) {
    if (!ensure(f, g->count)) return 0;
    f->goal = goal;

    for (int i = 0; i < g->count; i++) {
        f->dist[i] = INT_MAX;
        f->next[i] = -1;
    }
    if (goal < 0 || goal >= g->count) { f->goal = -1; return 0; }

    char *done = calloc((size_t)g->count, 1);
    if (!done) return 0;

    int *open = NULL;
    f->dist[goal] = 0;
    darr_push(open, goal);

    int reached = 0;
    while (!darr_empty(open)) {
        int cur = pop_min(open, f->dist, done);
        if (cur < 0) break;
        if (done[cur]) continue;
        done[cur] = 1;
        reached++;

        // relax *incoming* links: for every cell `nb` that has a link INTO cur,
        // reaching the goal via cur might be cheaper. we don't store reverse
        // adjacency, so we scan nb's forward links for one that targets cur.
        // O(E) per pop; fine for these graphs. the next-hop we record is the
        // forward link nb->cur, exactly what the mover should take.
        for (int nb = 0; nb < g->count; nb++) {
            if (done[nb]) continue;
            nav_cell *cn = &g->cells[nb];
            for (int li = 0; li < cn->link_count; li++) {
                if (cn->link_to[li] != cur) continue;
                int nd = f->dist[cur] + cn->link_cost[li];
                if (nd < f->dist[nb]) {
                    f->dist[nb] = nd;
                    f->next[nb] = cur;
                    darr_push(open, nb);   // may double-list; pop skips done
                }
                break;   // a cell links a given target at most once
            }
        }
    }

    darr_free(open);
    free(done);
    return reached;
}

vec3 nav_flow_step(const nav_flow *f, const nav_grid *g, int from, vec3 fallback) {
    if (from < 0 || from >= f->count) return fallback;
    if (from == f->goal) return nav_cell_world(&g->cells[from]);
    int nx = f->next[from];
    if (nx < 0) return fallback;     // stranded, no route to goal
    return nav_cell_world(&g->cells[nx]);
}

int nav_flow_reachable(const nav_flow *f, int from) {
    if (from < 0 || from >= f->count) return 0;
    return f->dist[from] != INT_MAX;
}
