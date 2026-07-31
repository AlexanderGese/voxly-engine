#include "pf_astar.h"

#include <string.h>
#include <stdlib.h>

// 8-connected neighbourhood. first 4 are cardinal, last 4 diagonal, so we
// can clip the loop at 4 when diagonals are disabled.
static const int NB_DX[8]   = {  1, -1,  0,  0,   1,  1, -1, -1 };
static const int NB_DZ[8]   = {  0,  0,  1, -1,   1, -1,  1, -1 };
static const int NB_COST[8] = { PF_COST_CARD, PF_COST_CARD, PF_COST_CARD, PF_COST_CARD,
                                PF_COST_DIAG, PF_COST_DIAG, PF_COST_DIAG, PF_COST_DIAG };

void pf_search_setup(pf_search *s, pf_grid *grid, pf_nodepool *pool,
                     pf_openset *open) {
    s->grid           = grid;
    s->pool           = pool;
    s->open           = open;
    s->heuristic      = PF_H_OCTILE;
    s->max_expansions = PF_MAX_NODES;     // window-bounded anyway
    s->allow_diagonal = 1;
}

// don't let a diagonal cut through the corner of a solid block. both of the
// orthogonal cells the diagonal "brushes" past must be standable too, else
// the mob would clip the corner. classic grid-pathing gotcha.
static int diagonal_clear(pf_search *s, int lx, int lz, int dx, int dz, int y) {
    int fa, fb;
    int ax = lx + dx, az = lz;
    int bx = lx,      bz = lz + dz;
    if (!pf_grid_standable(s->grid, ax, az, y, &fa)) return 0;
    if (!pf_grid_standable(s->grid, bx, bz, y, &fb)) return 0;
    return 1;
}

// turn a finished search into a block path by walking parents back from the
// goal node, then reversing. returns the number of points written.
static int reconstruct(pf_search *s, int goal_idx, pf_rawpath *out) {
    int chain[PF_RAW_MAX];
    int n = 0;
    int idx = goal_idx;
    while (idx >= 0 && n < PF_RAW_MAX) {
        chain[n++] = idx;
        idx = s->pool->nodes[idx].parent;
    }
    out->count = 0;
    for (int i = n - 1; i >= 0; i--) {
        out->pts[out->count++] = s->pool->nodes[chain[i]].c;
    }
    return out->count;
}

int pf_astar_run(pf_search *s, pf_coord start, pf_coord goal, pf_rawpath *out) {
    out->count = 0;

    pf_nodepool_reset(s->pool);
    pf_openset_init(s->open, s->pool);

    int lx, lz;
    if (!pf_grid_to_local(s->grid, start, &lx, &lz)) return 0;

    // snap the start onto the actual floor so y is consistent with neighbours.
    int sfloor;
    if (!pf_grid_standable(s->grid, lx, lz, start.y, &sfloor)) return 0;
    start.y = (int16_t)sfloor;

    int start_idx = pf_nodepool_alloc(s->pool, start);
    if (start_idx < 0) return 0;
    pf_node *sn = &s->pool->nodes[start_idx];
    sn->g = 0;
    sn->f = pf_heuristic(s->heuristic, start, goal);
    pf_openset_push(s->open, start_idx);

    int dir_count = s->allow_diagonal ? 8 : 4;
    int expansions = 0;

    while (!pf_openset_empty(s->open)) {
        if (expansions++ >= s->max_expansions) break;

        int cur_idx = pf_openset_pop(s->open);
        pf_node *cur = &s->pool->nodes[cur_idx];
        cur->closed = 1;

        // goal test on the column (x,z) so dropping/jumping onto the target
        // tile still counts as arrival.
        if (cur->c.x == goal.x && cur->c.z == goal.z) {
            return reconstruct(s, cur_idx, out) > 0;
        }

        int clx, clz;
        if (!pf_grid_to_local(s->grid, cur->c, &clx, &clz)) continue;

        for (int d = 0; d < dir_count; d++) {
            int nlx = clx + NB_DX[d];
            int nlz = clz + NB_DZ[d];
            if (!pf_grid_in_bounds(s->grid, nlx, nlz)) continue;

            int nfloor;
            if (!pf_grid_standable(s->grid, nlx, nlz, cur->c.y, &nfloor)) continue;

            // refetch cur->c.y for clarity; the move's vertical delta adds a
            // surcharge so the planner avoids needless stairs.
            if (d >= 4 && !diagonal_clear(s, clx, clz, NB_DX[d], NB_DZ[d], cur->c.y))
                continue;

            pf_coord nc = pf_coord_make(s->grid->origin.x + nlx, nfloor,
                                        s->grid->origin.z + nlz);

            int step_cost = NB_COST[d] + abs(nfloor - cur->c.y) * PF_COST_STEP;
            int ng = cur->g + step_cost;

            int nidx = pf_nodepool_find(s->pool, nc);
            if (nidx < 0) {
                nidx = pf_nodepool_alloc(s->pool, nc);
                if (nidx < 0) goto out_of_nodes;   // pool exhausted, bail clean
                pf_node *nn = &s->pool->nodes[nidx];
                nn->g = ng;
                nn->f = ng + pf_heuristic(s->heuristic, nc, goal);
                nn->parent = cur_idx;
                pf_openset_push(s->open, nidx);
            } else {
                pf_node *nn = &s->pool->nodes[nidx];
                if (nn->closed) continue;
                if (ng < nn->g) {
                    nn->g = ng;
                    nn->f = ng + pf_heuristic(s->heuristic, nc, goal);
                    nn->parent = cur_idx;
                    pf_openset_push(s->open, nidx);   // decrease-key
                }
            }
        }
    }

out_of_nodes:
    return 0;
}
