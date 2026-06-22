#include "ravine_field.h"

#include "../../util/log.h"
#include <stdlib.h>
#include <string.h>

ravine_field *ravine_field_create(void) {
    ravine_field *f = calloc(1, sizeof *f);
    if (!f) {
        LOGE("ravine: field alloc failed");
        return NULL;
    }
    return f;
}

void ravine_field_destroy(ravine_field *f) {
    if (!f) return;
    free(f);
}

void ravine_field_reset(ravine_field *f, ravine_origin origin) {
    f->origin = origin;
    f->dirty = 0;
    // surface is the caller's job; everything else clears to "untouched".
    memset(f->cut,  0, sizeof f->cut);
    memset(f->kind, RAVINE_OUTSIDE, sizeof f->kind);
    for (int i = 0; i < RAVINE_CELLS; i++) {
        f->surface[i] = CHUNK_SIZE_Y - 1;
        f->floor_y[i] = CHUNK_SIZE_Y - 1;
    }
}

int ravine_field_in_bounds(int x, int z) {
    return x >= 0 && x < RAVINE_DIM_X && z >= 0 && z < RAVINE_DIM_Z;
}

void ravine_field_set_surface(ravine_field *f, int x, int z, int y) {
    int i = ravine_cell_index(x, z);
    if (i < 0) return;
    f->surface[i] = y;
}

int ravine_field_surface(const ravine_field *f, int x, int z) {
    int i = ravine_cell_index(x, z);
    if (i < 0) return CHUNK_SIZE_Y - 1;
    return f->surface[i];
}

void ravine_field_to_world(const ravine_field *f, int x, int z,
                           int *wx, int *wz) {
    // cell (PAD,PAD) maps to chunk-local (0,0); base carries the chunk origin.
    if (wx) *wx = f->origin.base_x + (x - RAVINE_PAD);
    if (wz) *wz = f->origin.base_z + (z - RAVINE_PAD);
}

int ravine_field_from_world(const ravine_field *f, int wx, int wz,
                            int *x, int *z) {
    int cx = (wx - f->origin.base_x) + RAVINE_PAD;
    int cz = (wz - f->origin.base_z) + RAVINE_PAD;
    if (!ravine_field_in_bounds(cx, cz)) return 0;
    if (x) *x = cx;
    if (z) *z = cz;
    return 1;
}

int ravine_field_count_kind(const ravine_field *f, uint8_t kind) {
    int n = 0;
    for (int i = 0; i < RAVINE_CELLS; i++)
        if (f->kind[i] == kind) n++;
    return n;
}
