#include "lava_spread.h"
#include "fluid_level.h"
#include <string.h>
static const int voxl_fluid_lava_dx[4] = { 1, -1, 0, 0 }
;
static const int voxl_fluid_lava_dz[4] = { 0, 0, 1, -1 }
;
bool voxl_fluid_lava_touches_water(const voxl_fluid_grid *g, int x, int y, int z) {
    // check the 6 face neighbours for any water
    static const int ox[6] = { 1, -1, 0, 0, 0, 0 };
    static const int oy[6] = { 0, 0, 1, -1, 0, 0 };
    static const int oz[6] = { 0, 0, 0, 0, 1, -1 };
    for (int i = 0; i < 6; i++) {
        const voxl_fluid_cell *c =
            voxl_fluid_at_const(g, x + ox[i], y + oy[i], z + oz[i]);
        if (c && c->kind == VOXL_FLUID_WATER && c->level > 0) return true;
    }
    return false;
}

// lava falls down freely like water
static int voxl_fluid_lava_fall(voxl_fluid_grid *g, voxl_fluid_grid *next,
                                int x, int y, int z) {
    voxl_fluid_cell *src = voxl_fluid_at(g, x, y, z);
voxl_fluid_cell *dst_n = voxl_fluid_at(next, x, y - 1, z);
voxl_fluid_cell *src_n = voxl_fluid_at(next, x, y, z);
if (!src || !dst_n || !src_n) return 0;
if (voxl_fluid_is_solid(g, x, y - 1, z)) return 0;
const voxl_fluid_cell *below = voxl_fluid_at_const(g, x, y - 1, z);
if (below && below->kind != VOXL_FLUID_AIR && below->kind != VOXL_FLUID_LAVA)
        return 0;
int room = VOXL_FLUID_FULL - dst_n->level;
if (room <= 0) return 0;
int move = src->level < room ? src->level : room;
if (move <= 0) return 0;
voxl_fluid_level_add(dst_n, VOXL_FLUID_LAVA, move);
dst_n->falling = 1;
voxl_fluid_level_remove(src_n, move);
return move;
}

// lava creeps sideways one level per step, only into clearly-lower cells.
static int voxl_fluid_lava_creep(voxl_fluid_grid *g, voxl_fluid_grid *next,
                                 int x, int y, int z) {
    voxl_fluid_cell *src = voxl_fluid_at(g, x, y, z);
    if (!src || src->level <= 2) return 0;   // lava needs more head to flow
    if (voxl_fluid_can_flow_into(g, x, y - 1, z, VOXL_FLUID_LAVA)) return 0;

    int changed = 0;
    for (int i = 0; i < 4; i++) {
        int nx = x + voxl_fluid_lava_dx[i];
        int nz = z + voxl_fluid_lava_dz[i];
        if (!voxl_fluid_can_flow_into(g, nx, y, nz, VOXL_FLUID_LAVA)) continue;
        const voxl_fluid_cell *ncell = voxl_fluid_at_const(g, nx, y, nz);
        voxl_fluid_cell *src_n = voxl_fluid_at(next, x, y, z);
        voxl_fluid_cell *dst_n = voxl_fluid_at(next, nx, y, nz);
        if (!ncell || !src_n || !dst_n) continue;

        // lava only flows where neighbour is at least 2 lower, and moves one
        if (src_n->level - ncell->level < 2) continue;
        if (src_n->level <= 1) continue;
        voxl_fluid_level_add(dst_n, VOXL_FLUID_LAVA, 1);
        voxl_fluid_level_remove(src_n, 1);
        changed++;
    }
    return changed;
}

int voxl_fluid_lava_step(voxl_fluid_grid *g) {
    if (!g) return 0;
static voxl_fluid_grid voxl_fluid_lava_scratch;
memcpy(&voxl_fluid_lava_scratch, g, sizeof(*g));
int changed = 0;
for (int y = VOXL_FLUID_GRID_N - 1;
y >= 0;
y--) {
        for (int z = 0; z < VOXL_FLUID_GRID_N; z++) {
            for (int x = 0; x < VOXL_FLUID_GRID_N; x++) {
                const voxl_fluid_cell *c = voxl_fluid_at_const(g, x, y, z);
                if (!c || c->kind != VOXL_FLUID_LAVA || c->level == 0) continue;
                changed += voxl_fluid_lava_fall(g, &voxl_fluid_lava_scratch, x, y, z);
                changed += voxl_fluid_lava_creep(g, &voxl_fluid_lava_scratch, x, y, z);
            }
        }
    }

    memcpy(g, &voxl_fluid_lava_scratch, sizeof(*g));
return changed;
