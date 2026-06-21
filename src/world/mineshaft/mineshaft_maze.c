#include "mineshaft_maze.h"
#include "mineshaft_box.h"

// chances that live with the maze topology, not the block palette. an end-stub
// blossoms into an ore room, or partly caves in, else it stays a plain dead end.
#define ROOM_CHANCE     0.28f
#define CAVEIN_CHANCE   0.35f

// explicit stack frame for the dfs. recursion on a 256-cell grid is asking for
// trouble, so we drive it with a fixed scratch stack instead.
typedef struct { int x, z; } cellpos;

static int visited(mineshaft_grid *g, int x, int z) {
    if (!mineshaft_grid_in_bounds(g, x, z)) return 1;   // off-grid = "blocked"
    return mineshaft_grid_at(g, x, z)->kind != MS_CELL_EMPTY;
}

void mineshaft_maze_carve(mineshaft_grid *g, const mineshaft_config *cfg,
                          mineshaft_rng *rng) {
    (void)cfg;
    int total = g->w * g->d;
    static cellpos stack[MINESHAFT_GRID_MAX * MINESHAFT_GRID_MAX];
    int sp = 0;

    int sx = g->entry_x, sz = g->entry_z;
    mineshaft_grid_at(g, sx, sz)->kind = MS_CELL_CORRIDOR;
    stack[sp].x = sx; stack[sp].z = sz; sp++;

    while (sp > 0) {
        cellpos cur = stack[sp - 1];

        // gather unvisited neighbours, shuffled so growth wanders.
        int dirs[4] = { MINESHAFT_NORTH, MINESHAFT_EAST,
                        MINESHAFT_SOUTH, MINESHAFT_WEST };
        mineshaft_rng_shuffle(rng, dirs, 4);

        int advanced = 0;
        for (int i = 0; i < 4; i++) {
            mineshaft_dir d = (mineshaft_dir)dirs[i];
            int dx, dz;
            mineshaft_dir_step(d, &dx, &dz);
            int nx = cur.x + dx, nz = cur.z + dz;
            if (visited(g, nx, nz)) continue;

            mineshaft_grid_at(g, nx, nz)->kind = MS_CELL_CORRIDOR;
            mineshaft_grid_link(g, cur.x, cur.z, d);
            stack[sp].x = nx; stack[sp].z = nz; sp++;
            advanced = 1;
            break;
        }

        if (!advanced) sp--;   // dead end for the walk, backtrack
    }

    // braid pass: knock a handful of extra links so the maze loops. we target
    // degree-1 cells (dead ends) and merge them with an unlinked neighbour.
    int braids = total / 5;
    for (int b = 0; b < braids; b++) {
        int x = mineshaft_rng_range(rng, 0, g->w - 1);
        int z = mineshaft_rng_range(rng, 0, g->d - 1);
        if (mineshaft_grid_at(g, x, z)->kind == MS_CELL_EMPTY) continue;
        if (mineshaft_grid_degree(g, x, z) != 1) continue;

        int dirs[4] = { MINESHAFT_NORTH, MINESHAFT_EAST,
                        MINESHAFT_SOUTH, MINESHAFT_WEST };
        mineshaft_rng_shuffle(rng, dirs, 4);
        for (int i = 0; i < 4; i++) {
            mineshaft_dir d = (mineshaft_dir)dirs[i];
            int dx, dz;
            mineshaft_dir_step(d, &dx, &dz);
            int nx = x + dx, nz = z + dz;
            if (!mineshaft_grid_in_bounds(g, nx, nz)) continue;
            if (mineshaft_grid_at(g, nx, nz)->kind == MS_CELL_EMPTY) continue;
            if (mineshaft_grid_at(g, x, z)->links & mineshaft_dir_link_bit(d)) continue;
            mineshaft_grid_link(g, x, z, d);
            break;
        }
    }
}

int mineshaft_maze_depths(mineshaft_grid *g) {
    static cellpos queue[MINESHAFT_GRID_MAX * MINESHAFT_GRID_MAX];
    int head = 0, tail = 0;
    int maxd = 0;

    // depth==255 is the "unvisited" sentinel during the walk.
    for (int i = 0; i < g->w * g->d; i++) g->cells[i].depth = 255;

    mineshaft_cell *ec = mineshaft_grid_at(g, g->entry_x, g->entry_z);
    if (ec->kind == MS_CELL_EMPTY) return 0;
    ec->depth = 0;
    queue[tail].x = g->entry_x; queue[tail].z = g->entry_z; tail++;

    while (head < tail) {
        cellpos cur = queue[head++];
        mineshaft_cell *c = mineshaft_grid_at(g, cur.x, cur.z);
        if (c->depth != 255 && c->depth > maxd) maxd = c->depth;

        for (int d = 0; d < 4; d++) {
            if (!(c->links & (1 << d))) continue;
            int dx, dz;
            mineshaft_dir_step((mineshaft_dir)d, &dx, &dz);
            int nx = cur.x + dx, nz = cur.z + dz;
            if (!mineshaft_grid_in_bounds(g, nx, nz)) continue;
            mineshaft_cell *nc = mineshaft_grid_at(g, nx, nz);
            if (nc->depth != 255) continue;   // already reached
            nc->depth = (uint8_t)(c->depth + 1);
            queue[tail].x = nx; queue[tail].z = nz; tail++;
        }
    }

    // any unreached carved cell (shouldn't happen post-braid) gets depth 0 so
    // downstream math never sees the 255 sentinel.
    for (int i = 0; i < g->w * g->d; i++)
        if (g->cells[i].kind != MS_CELL_EMPTY && g->cells[i].depth == 255)
            g->cells[i].depth = 0;

    return maxd;
}

void mineshaft_maze_classify(mineshaft_grid *g, const mineshaft_config *cfg,
                             mineshaft_rng *rng) {
    (void)cfg;
    int maxd = mineshaft_maze_depths(g);

    for (int z = 0; z < g->d; z++) {
        for (int x = 0; x < g->w; x++) {
            mineshaft_cell *c = mineshaft_grid_at(g, x, z);
            if (c->kind == MS_CELL_EMPTY) continue;

            int deg = mineshaft_grid_degree(g, x, z);
            if (deg >= 3) {
                c->kind = MS_CELL_JUNCTION;
            } else if (deg == 1) {
                // a stub. ore room, cave-in, or plain dead end, in that priority.
                if (mineshaft_rng_chance(rng, ROOM_CHANCE)) {
                    c->kind = MS_CELL_ROOM;
                    c->flags |= MS_FLAG_CHEST;
                } else if (mineshaft_rng_chance(rng, CAVEIN_CHANCE)) {
                    c->kind = MS_CELL_DEADEND;
                    c->flags |= MS_FLAG_COLLAPSED;
                } else {
                    c->kind = MS_CELL_DEADEND;
                }
            } else {
                c->kind = MS_CELL_CORRIDOR;
            }
        }
    }

    // the deepest reachable cell drops a vertical shaft to a lower level - the
    // classic "this keeps going" hook. only if the maze is deep enough to earn it.
    if (maxd >= 4) {
        for (int z = 0; z < g->d; z++) {
            for (int x = 0; x < g->w; x++) {
                mineshaft_cell *c = mineshaft_grid_at(g, x, z);
                if (c->kind != MS_CELL_EMPTY && c->depth == (uint8_t)maxd) {
                    c->kind = MS_CELL_SHAFT;
                    c->flags |= MS_FLAG_LADDER;
                    return;   // one shaft is plenty
                }
            }
        }
    }
}
