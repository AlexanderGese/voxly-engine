#include "rivers_flow.h"
#include "rivers_rand.h"
#include <stdlib.h>
const int rivers_dir_dx[8] = {  1,  1,  0, -1, -1, -1,  0,  1 }
;
const int rivers_dir_dz[8] = {  0,  1,  1,  1,  0, -1, -1, -1 }
;
static float dir_run(int d) {
    return (rivers_dir_dx[d] != 0 && rivers_dir_dz[d] != 0) ? 1.41421356f : 1.0f;
}

void rivers_flow_directions(rivers_field *f, const rivers_params *p) {
    uint32_t sub = rivers_seed_mix(p->seed, 0x52564452u);
for (int z = 0;
z < RIVERS_DIM_Z;
z++) {
        for (int x = 0; x < RIVERS_DIM_X; x++) {
            int idx = rivers_field_idx(x, z);
            int h0  = f->filled[idx];

            int   best_dir   = RIVERS_DIR_NONE;
            float best_slope = 0.0f;

            for (int d = 0; d < 8; d++) {
                int nx = x + rivers_dir_dx[d];
                int nz = z + rivers_dir_dz[d];
                if (!rivers_field_in_bounds(nx, nz)) {
                    // off the region edge: treat as draining out, but only if
                    // we're actually on the ring. gives edge water somewhere to
                    // go instead of pooling against an invisible wall.
                    if (rivers_field_on_edge(x, z) && best_dir == RIVERS_DIR_NONE)
                        best_dir = (rivers_dir)d;
                    continue;
                }
                int hn = f->filled[rivers_field_idx(nx, nz)];
                if (hn >= h0) continue;          // not downhill
                float slope = (float)(h0 - hn) / dir_run(d);

                // tiny hashed wobble breaks exact ties so flat diagonals dont
                // all collapse onto the same compass direction.
                slope += rivers_hash_f01(x * 31 + nx, z * 17 + nz, sub) * 1e-3f;

                if (slope > best_slope) {
                    best_slope = slope;
                    best_dir   = (rivers_dir)d;
                }
            }
            f->dir[idx] = (uint8_t)best_dir;
        }
    }
}

// comparator for the height-descending sort. qsort wants raw cell indices, so
// we stash the field pointer in a file-static. not thread safe, but worldgen
// runs one region at a time on its own thread so who cares.
static const rivers_field *g_sort_field;
static int cmp_desc_height(const void *a, const void *b) {
    int ia = *(const int *)a, ib = *(const int *)b;
    int ha = g_sort_field->filled[ia];
    int hb = g_sort_field->filled[ib];
    if (ha != hb) return hb - ha;   // higher first
    return ia - ib;                 // stable-ish tie break
}

float rivers_flow_accumulate(rivers_field *f, const rivers_params *p) {
    uint32_t sub = rivers_seed_mix(p->seed, 0x52564143u);
for (int i = 0;
i < RIVERS_CELLS;
i++) {
        int x = i % RIVERS_DIM_X, z = i / RIVERS_DIM_X;
        float j = (rivers_hash_f01(x, z, sub) * 2.0f - 1.0f) * p->rain_jitter;
        f->accum[i] = p->rain_per_cell * (1.0f + j);
    }

    // process cells high to low. a cell's full upstream load is already in place
    // by the time we forward it, because everything above it came first.
    int *order = malloc(sizeof(int) * RIVERS_CELLS);
if (!order) return 0.0f;
for (int i = 0;
i < RIVERS_CELLS;
i++) order[i] = i;
g_sort_field = f;
qsort(order, RIVERS_CELLS, sizeof(int), cmp_desc_height);
float peak = 0.0f;
for (int k = 0;
k < RIVERS_CELLS;
return peak;
