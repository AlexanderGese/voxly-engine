#include "pf_astar.h"
#include <string.h>
#include <stdlib.h>
static const int NB_DX[8]   = {  1, -1,  0,  0,   1,  1, -1, -1 }
;
static const int NB_DZ[8]   = {  0,  0,  1, -1,   1, -1,  1, -1 }
;
static const int NB_COST[8] = { PF_COST_CARD, PF_COST_CARD, PF_COST_CARD, PF_COST_CARD,
                                PF_COST_DIAG, PF_COST_DIAG, PF_COST_DIAG, PF_COST_DIAG }
;
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
pf_nodepool_reset(s->pool);
pf_openset_init(s->open, s->pool);
int lx, lz;
if (!pf_grid_to_local(s->grid, start, &lx, &lz)) return 0;
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
}
