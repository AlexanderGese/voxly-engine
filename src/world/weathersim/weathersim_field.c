#include "weathersim_field.h"
#include <math.h>
#include <string.h>
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
    if (gx < 0) gx = 0;
else if (gx >= WEATHERSIM_DIM) gx = WEATHERSIM_DIM - 1;
if (gz < 0) gz = 0;
else if (gz >= WEATHERSIM_DIM) gz = WEATHERSIM_DIM - 1;
return &f->cells[weathersim_field_idx(gx, gz)];
int lz = cell_z - f->origin_cz;
if (lx < 0 || lx >= WEATHERSIM_DIM || lz < 0 || lz >= WEATHERSIM_DIM)
        return 0;
if (gx) *gx = lx;
if (gz) *gz = lz;
return 1;
int new_oz = center_cz - HALF;
int dx = new_ox - f->origin_cx;
int dz = new_oz - f->origin_cz;
if (dx == 0 && dz == 0) return 0;
for (int gz = 0;
gz < WEATHERSIM_DIM;
f->origin_cx = new_ox;
f->origin_cz = new_oz;
int n = 0;
for (int i = 0;
i < WEATHERSIM_CELLS;
++i)
        if (isnan(f->cells[i].temp)) ++n;
return n;
}
