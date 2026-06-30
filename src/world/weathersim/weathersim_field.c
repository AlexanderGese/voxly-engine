#include "weathersim_field.h"

#include <math.h>
#include <string.h>

// half-width of the grid. center cell sits at (HALF, HALF).
#define HALF (WEATHERSIM_DIM / 2)

int weathersim_field_in_bounds(int gx, int gz) {
    return gx >= 0 && gx < WEATHERSIM_DIM && gz >= 0 && gz < WEATHERSIM_DIM;
}

int weathersim_world_to_cell(int wx) {
    // floor-divide so negative coords don't fold toward zero.
    int b = WEATHERSIM_BLOCKS_PER_CELL;
    return (wx >= 0) ? (wx / b) : -(((-wx) + b - 1) / b);
}

void weathersim_cell_center_world(int cell_x, int cell_z, int *wx, int *wz) {
    int half = WEATHERSIM_BLOCKS_PER_CELL / 2;
    if (wx) *wx = cell_x * WEATHERSIM_BLOCKS_PER_CELL + half;
    if (wz) *wz = cell_z * WEATHERSIM_BLOCKS_PER_CELL + half;
}

weathersim_cell *weathersim_field_at(weathersim_field *f, int gx, int gz) {
    if (gx < 0) gx = 0; else if (gx >= WEATHERSIM_DIM) gx = WEATHERSIM_DIM - 1;
    if (gz < 0) gz = 0; else if (gz >= WEATHERSIM_DIM) gz = WEATHERSIM_DIM - 1;
    return &f->cells[weathersim_field_idx(gx, gz)];
}

const weathersim_cell *weathersim_field_at_const(const weathersim_field *f,
                                                 int gx, int gz) {
    if (gx < 0) gx = 0; else if (gx >= WEATHERSIM_DIM) gx = WEATHERSIM_DIM - 1;
    if (gz < 0) gz = 0; else if (gz >= WEATHERSIM_DIM) gz = WEATHERSIM_DIM - 1;
    return &f->cells[weathersim_field_idx(gx, gz)];
}

int weathersim_field_cell_to_grid(const weathersim_field *f,
                                  int cell_x, int cell_z, int *gx, int *gz) {
    int lx = cell_x - f->origin_cx;
    int lz = cell_z - f->origin_cz;
    if (lx < 0 || lx >= WEATHERSIM_DIM || lz < 0 || lz >= WEATHERSIM_DIM)
        return 0;
    if (gx) *gx = lx;
    if (gz) *gz = lz;
    return 1;
}

void weathersim_field_init(weathersim_field *f, int center_cx, int center_cz) {
    memset(f, 0, sizeof *f);
    f->origin_cx = center_cx - HALF;
    f->origin_cz = center_cz - HALF;
    f->seeded = 0;
    // mark every cell unseeded with a NAN temp; the climate pass replaces it.
    for (int i = 0; i < WEATHERSIM_CELLS; ++i)
        f->cells[i].temp = NAN;
}

int weathersim_field_recenter(weathersim_field *f, int center_cx, int center_cz) {
    int new_ox = center_cx - HALF;
    int new_oz = center_cz - HALF;
    int dx = new_ox - f->origin_cx;
    int dz = new_oz - f->origin_cz;
    if (dx == 0 && dz == 0) return 0;

    // copy cells that survive the shift into scratch at their new slot, mark
    // everything that doesn't have a source as unseeded (NAN temp).
    for (int gz = 0; gz < WEATHERSIM_DIM; ++gz) {
        for (int gx = 0; gx < WEATHERSIM_DIM; ++gx) {
            int src_gx = gx + dx;
            int src_gz = gz + dz;
            weathersim_cell *dst = &f->scratch[weathersim_field_idx(gx, gz)];
            if (src_gx >= 0 && src_gx < WEATHERSIM_DIM &&
                src_gz >= 0 && src_gz < WEATHERSIM_DIM) {
                *dst = f->cells[weathersim_field_idx(src_gx, src_gz)];
            } else {
                memset(dst, 0, sizeof *dst);
                dst->temp = NAN; // freshly exposed: needs a climate seed
            }
        }
    }
    memcpy(f->cells, f->scratch, sizeof f->cells);
    f->origin_cx = new_ox;
    f->origin_cz = new_oz;

    // count how many need re-seeding so the caller can decide whether it's
    // worth a full climate sweep or a strip.
    int n = 0;
    for (int i = 0; i < WEATHERSIM_CELLS; ++i)
        if (isnan(f->cells[i].temp)) ++n;
    return n;
}
