#include "logic_trace.h"
#include "logic_block.h"
#include "logic_dir.h"
#include "../../util/darray.h"
#include <stddef.h>

// a tiny visited set keyed by logic_key. we reuse the cell's VISITED flag for
// the blob walk so we don't need a side table; it's cleared on the way out.
static void clear_visited(logic_grid *g, logic_trace *t) {
    for (int i = 0; i < t->cell_count; i++) {
        logic_cell *c = logic_grid_get(g, t->coords[i][0],
                                       t->coords[i][1], t->coords[i][2]);
        if (c) c->flags &= ~LOGIC_CF_VISITED;
    }
}

int logic_trace_net(logic_grid *g, int x, int y, int z, logic_trace *out) {
    out->cell_count = 0;
    out->feed_count = 0;
    out->strongest_feed = 0;
    out->max_reach = 0;
    out->lit = 0;
    out->truncated = 0;

    logic_cell *start = logic_grid_get(g, x, y, z);
    if (!start || !logic_block_is_wire(start->kind)) return 0;

    // bfs frontier of wire cells. emitters touching the blob are tallied but
    // never enqueued, so the walk stays inside the dust.
    logic_cell **q = NULL;
    darr_push(q, start);
    start->flags |= LOGIC_CF_VISITED;

    size_t qi = 0;
    while (qi < darr_len(q)) {
        logic_cell *c = q[qi++];

        if (out->cell_count >= LOGIC_TRACE_MAX) { out->truncated = 1; break; }
        out->coords[out->cell_count][0] = c->x;
        out->coords[out->cell_count][1] = c->y;
        out->coords[out->cell_count][2] = c->z;
        out->cell_count++;

        if (c->power > out->max_reach) out->max_reach = c->power;
        if (c->power > 0) out->lit = 1;

        for (int d = 0; d < LOGIC_DIR_COUNT; d++) {
            int nx, ny, nz;
            logic_dir_step((logic_dir)d, c->x, c->y, c->z, &nx, &ny, &nz);
            logic_cell *n = logic_grid_get(g, nx, ny, nz);
            if (!n) continue;

            if (logic_block_is_wire(n->kind)) {
                if (n->flags & LOGIC_CF_VISITED) continue;
                n->flags |= LOGIC_CF_VISITED;
                darr_push(q, n);
            } else if (n->power > 0) {
                // an emitter feeding the blob.
                out->feed_count++;
                if (n->power > out->strongest_feed)
                    out->strongest_feed = n->power;
            }
        }
    }

    int n_cells = out->cell_count;
    clear_visited(g, out);
    darr_free(q);
    return n_cells;
}

int logic_trace_distance_to_source(logic_grid *g, int x, int y, int z) {
    logic_cell *start = logic_grid_get(g, x, y, z);
    if (!start) return -1;

    // bfs outward over wire, level by level, until we touch an emitter. each
    // frontier entry carries its step distance alongside the cell.
    typedef struct { logic_cell *c; int dist; } qn;
    qn *q = NULL;
    int found = -1;
    int touched = 0;

    qn first = { start, 0 };
    darr_push(q, first);
    start->flags |= LOGIC_CF_VISITED;

    size_t qi = 0;
    while (qi < darr_len(q) && found < 0 && touched < LOGIC_TRACE_MAX) {
        qn cur = q[qi++];
        touched++;

        // if the starting cell itself is an emitter, distance is zero.
        if (!logic_block_is_wire(cur.c->kind) && cur.c->power > 0) {
            found = cur.dist;
            break;
        }

        for (int d = 0; d < LOGIC_DIR_COUNT; d++) {
            int nx, ny, nz;
            logic_dir_step((logic_dir)d, cur.c->x, cur.c->y, cur.c->z,
                           &nx, &ny, &nz);
            logic_cell *n = logic_grid_get(g, nx, ny, nz);
            if (!n || (n->flags & LOGIC_CF_VISITED)) continue;

            if (!logic_block_is_wire(n->kind)) {
                if (n->power > 0) { found = cur.dist + 1; break; }
                continue; // a sink/unpowered block - dead end
            }
            n->flags |= LOGIC_CF_VISITED;
            qn nx_node = { n, cur.dist + 1 };
            darr_push(q, nx_node);
        }
    }

    // scrub the visited bits we set.
    for (size_t i = 0; i < darr_len(q); i++)
        q[i].c->flags &= ~LOGIC_CF_VISITED;
    darr_free(q);
    return found;
}
