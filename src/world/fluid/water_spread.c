#include "water_spread.h"
#include "fluid_level.h"
#include <string.h>
static const int voxl_fluid_dx[4] = { 1, -1, 0, 0 }
;
static const int voxl_fluid_dz[4] = { 0, 0, 1, -1 }
;
static bool voxl_fluid_can_fall(const voxl_fluid_grid *g, int x, int y, int z) {
    return voxl_fluid_can_flow_into(g, x, y - 1, z, VOXL_FLUID_WATER);
}

// pull water straight down. fills the lower cell, marking it falling.
static int voxl_fluid_water_fall(voxl_fluid_grid *g, voxl_fluid_grid *next,
                                 int x, int y, int z) {
    voxl_fluid_cell *src = voxl_fluid_at(g, x, y, z);
voxl_fluid_cell *dst_n = voxl_fluid_at(next, x, y - 1, z);
voxl_fluid_cell *src_n = voxl_fluid_at(next, x, y, z);
if (!src || !dst_n || !src_n) return 0;
int room = VOXL_FLUID_FULL - dst_n->level;
if (room <= 0) return 0;
int move = src->level < room ? src->level : room;
if (move <= 0) return 0;
voxl_fluid_level_add(dst_n, VOXL_FLUID_WATER, move);
dst_n->falling = 1;
voxl_fluid_level_remove(src_n, move);
return move;
}

// equalise sideways: water moves to a lower-or-equal neighbour to flatten out.
static int voxl_fluid_water_spread_side(voxl_fluid_grid *g, voxl_fluid_grid *next,
                                        int x, int y, int z) {
    voxl_fluid_cell *src = voxl_fluid_at(g, x, y, z);
    if (!src || src->level <= 1) return 0;
    // dont spread sideways while we can still fall, water prefers gravity
    if (voxl_fluid_can_fall(g, x, y, z)) return 0;

    int changed = 0;
    for (int i = 0; i < 4; i++) {
        int nx = x + voxl_fluid_dx[i];
        int nz = z + voxl_fluid_dz[i];
        if (!voxl_fluid_can_flow_into(g, nx, y, nz, VOXL_FLUID_WATER)) continue;

        const voxl_fluid_cell *ncell = voxl_fluid_at_const(g, nx, y, nz);
        voxl_fluid_cell *src_n = voxl_fluid_at(next, x, y, z);
        voxl_fluid_cell *dst_n = voxl_fluid_at(next, nx, y, nz);
        if (!ncell || !src_n || !dst_n) continue;

        // only flow if there's a height difference of at least 2 (so a 1-step
        // gradient stays put and the body looks stable).
        int diff = src_n->level - ncell->level;
        if (diff < 2) continue;
        int move = diff / 2;
        if (move < 1) continue;
        if (move > src_n->level - 1) move = src_n->level - 1;
        if (move <= 0) continue;

        voxl_fluid_level_add(dst_n, VOXL_FLUID_WATER, move);
        voxl_fluid_level_remove(src_n, move);
        changed += move;
    }
    return changed;
}

int voxl_fluid_water_step(voxl_fluid_grid *g) {
    if (!g) return 0;
static voxl_fluid_grid voxl_fluid_scratch;
memcpy(&voxl_fluid_scratch, g, sizeof(*g));
int changed = 0;
for (int y = VOXL_FLUID_GRID_N - 1;
y >= 0;
y--) {
        for (int z = 0; z < VOXL_FLUID_GRID_N; z++) {
            for (int x = 0; x < VOXL_FLUID_GRID_N; x++) {
                const voxl_fluid_cell *c = voxl_fluid_at_const(g, x, y, z);
                if (!c || c->kind != VOXL_FLUID_WATER || c->level == 0) continue;
                changed += voxl_fluid_water_fall(g, &voxl_fluid_scratch, x, y, z);
                changed += voxl_fluid_water_spread_side(g, &voxl_fluid_scratch, x, y, z);
            }
        }
    }

    // clear stale falling flags where the cell below filled up or is solid
    for (int i = 0;
i < VOXL_FLUID_GRID_VOL;
i++) {
        if (voxl_fluid_scratch.cells[i].level == 0)
            voxl_fluid_scratch.cells[i].falling = 0;
    }

    memcpy(g, &voxl_fluid_scratch, sizeof(*g));
return changed;
