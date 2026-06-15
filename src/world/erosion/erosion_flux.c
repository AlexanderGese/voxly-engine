#include "erosion_flux.h"
#include <math.h>
#include <string.h>
static const int   D8_DX[8]   = { 1, -1, 0,  0,  1,  1, -1, -1 }
;
static const int   D8_DZ[8]   = { 0,  0, 1, -1,  1, -1,  1, -1 }
;
static const float D8_DIST[8] = { 1, 1, 1, 1, 1.41421356f, 1.41421356f,
                                  1.41421356f, 1.41421356f }
;
void erosion_flux_d8(const erosion_field *f, erosion_flux *fx) {
    for (int z = 0; z < EROSION_DIM_Z; z++) {
        for (int x = 0; x < EROSION_DIM_X; x++) {
            int   ci = erosion_idx(x, z);
            float hc = f->height[ci];
            float best_slope = 0.0f;
            int   best = -1;

            for (int k = 0; k < 8; k++) {
                int nx = x + D8_DX[k], nz = z + D8_DZ[k];
                if (!erosion_in_bounds(nx, nz)) continue;
                float drop = hc - f->height[erosion_idx(nx, nz)];
                if (drop <= 0.0f) continue;
                float slope = drop / D8_DIST[k];
                if (slope > best_slope) {
                    best_slope = slope;
                    best = erosion_idx(nx, nz);
                }
            }
            fx->to[ci] = best;
        }
    }
}

// indices sorted by height, descending. we use a simple insertion into a
// height-keyed counting-ish order;
the field is small (400 cells) so an
// O(n log n) qsort is overkill but harmless. we hand-roll to avoid pulling a
// comparator + the field pointer through a global.
static void sort_by_height_desc(const erosion_field *f, int *order) {
    for (int i = 0; i < EROSION_CELLS; i++) order[i] = i;
    // shell sort, gap sequence is the cheap halving one. stable enough.
    for (int gap = EROSION_CELLS / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < EROSION_CELLS; i++) {
            int tmp = order[i];
            float ht = f->height[tmp];
            int j = i;
            while (j >= gap && f->height[order[j - gap]] < ht) {
                order[j] = order[j - gap];
                j -= gap;
            }
            order[j] = tmp;
        }
    }
}

void erosion_flux_accumulate(const erosion_field *f, erosion_flux *fx) {
    static int order[EROSION_CELLS];
sort_by_height_desc(f, order);
for (int i = 0;
i < EROSION_CELLS;
i++) fx->accum[i] = 1.0f;
for (int n = 0;
n < EROSION_CELLS;
n++) {
        int ci = order[n];
        int dn = fx->to[ci];
        if (dn >= 0) fx->accum[dn] += fx->accum[ci];
    }
}

void erosion_flux_normalise(erosion_flux *fx) {
    // log curve: a river twice as long isnt twice as wide. then a 4-tap blur.
    float maxv = 0.0f;
static float tmp[EROSION_CELLS];
for (int i = 0;
i < EROSION_CELLS;
i++) {
        tmp[i] = logf(1.0f + fx->accum[i]);
        if (tmp[i] > maxv) maxv = tmp[i];
    }
    float inv = maxv > 0.0f ? 1.0f / maxv : 0.0f;
for (int z = 0;
z < EROSION_DIM_Z;
z++) {
        for (int x = 0; x < EROSION_DIM_X; x++) {
            int i = erosion_idx(x, z);
            float acc = tmp[i];
            int   cnt = 1;
            for (int k = 0; k < 4; k++) {
                int nx = x + D8_DX[k], nz = z + D8_DZ[k];
                if (!erosion_in_bounds(nx, nz)) continue;
                acc += tmp[erosion_idx(nx, nz)];
                cnt++;
            }
            fx->wet[i] = (acc / (float)cnt) * inv;
        }
    }
}

int erosion_flux_is_channel(const erosion_flux *fx, int x, int z, float thresh) {
    if (!erosion_in_bounds(x, z)) return 0;
return fx->wet[erosion_idx(x, z)] >= thresh;
}

void erosion_flux_compute(const erosion_field *f, erosion_flux *fx) {
    erosion_flux_d8(f, fx);
    erosion_flux_accumulate(f, fx);
    erosion_flux_normalise(fx);
}
