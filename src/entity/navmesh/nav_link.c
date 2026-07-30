#include "nav_link.h"
#include "nav_cell.h"

#include <stdlib.h>

int nav_link_cost(int kind) {
    switch (kind) {
        case NAV_LINK_WALK: return 10;   // one tile, 10 = "1.0" in fixedish
        case NAV_LINK_DROP: return 14;   // falling is cheap but not free
        case NAV_LINK_JUMP: return 20;   // hops are expensive, avoid spamming
        default:            return 10;
    }
}

// push a directed link onto a cell's fan, if there's room. dedupes on target
// so re-linking the same neighbour (from both add passes) doesn't stack.
static void push_link(nav_cell *c, int to, int kind) {
    for (int i = 0; i < c->link_count; i++)
        if (c->link_to[i] == to) return;
    if (c->link_count >= NAV_CELL_LINKS) return;   // fan full, drop it
    int i = c->link_count++;
    c->link_to[i]   = to;
    c->link_kind[i] = (uint8_t)kind;
    c->link_cost[i] = (uint8_t)nav_link_cost(kind);
}

// a step across a block edge is only valid if the agent's body can fit in the
// diagonal-free L: both the source-side and target-side columns must be clear
// at the higher of the two floors, otherwise we'd be clipping a corner block.
// we already know each endpoint is standable; this guards the in-between.
static int corner_clear(world *w, int x0, int z0, int x1, int z1, int y) {
    for (int h = 1; h <= NAV_AGENT_HEIGHT; h++) {
        if (!nav_cell_passable(w, x0, y + h, z0)) return 0;
        if (!nav_cell_passable(w, x1, y + h, z1)) return 0;
    }
    return 1;
}

int nav_link_step(nav_grid *g, world *w, int src, int dx, int dz) {
    nav_cell *s = nav_grid_at(g, src);
    int nx = s->x + dx;
    int nz = s->z + dz;

    // scan the neighbour column for a floor within reach. prefer the floor
    // closest to our own height so flat ground stays a cheap WALK link.
    int best = -1, best_kind = 0, best_dy = 999;
    for (int dy = NAV_STEP_UP; dy >= -NAV_JUMP_DOWN; dy--) {
        int ny = s->y + dy;
        int idx = nav_grid_find(g, nav_coord_make(nx, ny, nz));
        if (idx < 0) continue;

        int kind;
        if (dy > 0)                 kind = NAV_LINK_JUMP;   // stepping up = hop
        else if (dy >= -NAV_STEP_DOWN) kind = (dy == 0) ? NAV_LINK_WALK
                                                        : NAV_LINK_DROP;
        else                        kind = NAV_LINK_DROP;   // long fall, ok one way

        // the squeeze test uses the higher floor: clearance has to exist where
        // the body passes through the edge.
        int hi = (ny > s->y) ? ny : s->y;
        if (!corner_clear(w, s->x, s->z, nx, nz, hi)) continue;

        if (abs(dy) < best_dy) {
            best = idx;
            best_kind = kind;
            best_dy = abs(dy);
        }
    }

    if (best < 0) return 0;
    push_link(s, best, best_kind);

    // mirror the link when it's safely two-way. a WALK is always mutual; a
    // single-block DROP an agent can jump back up, so mirror it as a JUMP.
    // bigger drops stay one-directional, the planner won't route up them.
    nav_cell *t = nav_grid_at(g, best);
    int back_dy = s->y - t->y;
    if (best_kind == NAV_LINK_WALK) {
        push_link(t, src, NAV_LINK_WALK);
    } else if (back_dy <= NAV_STEP_UP && back_dy >= -NAV_STEP_DOWN) {
        push_link(t, src, back_dy > 0 ? NAV_LINK_JUMP : NAV_LINK_DROP);
    }
    return 1;
}

void nav_link_all(nav_grid *g, world *w) {
    // four cardinals only. the loop is over a snapshot count so links added
    // to later cells don't get re-walked (they're handled by their own pass).
    static const int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    int n = g->count;
    for (int i = 0; i < n; i++) {
        for (int d = 0; d < 4; d++) {
            nav_link_step(g, w, i, dirs[d][0], dirs[d][1]);
        }
    }
}
