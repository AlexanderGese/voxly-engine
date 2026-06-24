#include "rivers_field.h"

#include <stdlib.h>
#include <string.h>

rivers_field *rivers_field_create(void) {
    rivers_field *f = calloc(1, sizeof(*f));
    // calloc already zeroed everything, but reset gives a known origin too.
    rivers_origin o = {0, 0, 0, 0};
    if (f) rivers_field_reset(f, o);
    return f;
}

void rivers_field_destroy(rivers_field *f) {
    free(f);
}

void rivers_field_reset(rivers_field *f, rivers_origin origin) {
    f->origin = origin;
    memset(f->surface, 0, sizeof(f->surface));
    memset(f->filled,  0, sizeof(f->filled));
    memset(f->accum,   0, sizeof(f->accum));
    memset(f->water_y, 0, sizeof(f->water_y));
    // dir defaults to NONE so an un-flowed cell reads as a pit, not east.
    memset(f->dir, RIVERS_DIR_NONE, sizeof(f->dir));
    memset(f->wet, RIVERS_DRY,      sizeof(f->wet));
    f->dirty = 0;
}

int rivers_field_idx(int x, int z) {
    return x + z * RIVERS_DIM_X;
}

int rivers_field_in_bounds(int x, int z) {
    return x >= 0 && x < RIVERS_DIM_X && z >= 0 && z < RIVERS_DIM_Z;
}

void rivers_field_set_surface(rivers_field *f, int x, int z, int y) {
    if (!rivers_field_in_bounds(x, z)) return;
    f->surface[rivers_field_idx(x, z)] = y;
}

int rivers_field_surface(const rivers_field *f, int x, int z) {
    if (!rivers_field_in_bounds(x, z)) return 0;
    return f->surface[rivers_field_idx(x, z)];
}

int rivers_field_on_edge(int x, int z) {
    return x == 0 || z == 0 || x == RIVERS_DIM_X - 1 || z == RIVERS_DIM_Z - 1;
}

void rivers_field_to_world(const rivers_field *f, int x, int z,
                           int *wx, int *wz) {
    // cell (RIVERS_PAD, RIVERS_PAD) is the bottom-left of the real chunk.
    if (wx) *wx = f->origin.base_x + (x - RIVERS_PAD);
    if (wz) *wz = f->origin.base_z + (z - RIVERS_PAD);
}

int rivers_field_from_world(const rivers_field *f, int wx, int wz,
                            int *x, int *z) {
    int lx = (wx - f->origin.base_x) + RIVERS_PAD;
    int lz = (wz - f->origin.base_z) + RIVERS_PAD;
    if (!rivers_field_in_bounds(lx, lz)) return 0;
    if (x) *x = lx;
    if (z) *z = lz;
    return 1;
}

int rivers_field_count_wet(const rivers_field *f, uint8_t state) {
    int n = 0;
    for (int i = 0; i < RIVERS_CELLS; i++)
        if (f->wet[i] == state) n++;
    return n;
}
