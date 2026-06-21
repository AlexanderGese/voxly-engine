#include "mineshaft_maze.h"
#include "mineshaft_box.h"
#define ROOM_CHANCE     0.28f
#define CAVEIN_CHANCE   0.35f
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
stack[sp].x = sx;
stack[sp].z = sz;
sp++;
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
for (int b = 0;
b < braids;
int head = 0, tail = 0;
int maxd = 0;
for (int i = 0;
i < g->w * g->d;
i++) g->cells[i].depth = 255;
mineshaft_cell *ec = mineshaft_grid_at(g, g->entry_x, g->entry_z);
if (ec->kind == MS_CELL_EMPTY) return 0;
ec->depth = 0;
queue[tail].x = g->entry_x;
queue[tail].z = g->entry_z;
tail++;
i < g->w * g->d;
i++)
        if (g->cells[i].kind != MS_CELL_EMPTY && g->cells[i].depth == 255)
            g->cells[i].depth = 0;
return maxd;
