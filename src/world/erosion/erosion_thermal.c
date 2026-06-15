#include "erosion_thermal.h"

#include <math.h>
#include <string.h>

// 4-neighbour offsets. we keep talus to the cardinal directions; diagonals
// would need a sqrt(2) talus scale and honestly nobody can tell from the air.
static const int NB_DX[4] = { 1, -1, 0,  0 };
static const int NB_DZ[4] = { 0,  0, 1, -1 };

float erosion_thermal_sweep(erosion_field *f, const erosion_params *p) {
    static float delta[EROSION_CELLS];   // scratch, single-threaded gen path
    memset(delta, 0, sizeof delta);

    float moved = 0.0f;

    for (int z = 0; z < EROSION_DIM_Z; z++) {
        for (int x = 0; x < EROSION_DIM_X; x++) {
            int   ci = erosion_idx(x, z);
            float hc = f->height[ci];

            // gather the downhill neighbours that exceed the talus angle and
            // the total excess slope, so we can split the slump proportionally.
            float excess[4];
            float total = 0.0f;
            int   any = 0;

            for (int k = 0; k < 4; k++) {
                int nx = x + NB_DX[k], nz = z + NB_DZ[k];
                excess[k] = 0.0f;
                if (!erosion_in_bounds(nx, nz)) continue;
                float diff = hc - f->height[erosion_idx(nx, nz)];
                if (diff > p->talus_angle) {
                    excess[k] = diff - p->talus_angle;
                    total += excess[k];
                    any = 1;
                }
            }
            if (!any) continue;

            // move a fraction of the *largest* excess so the slope eases off
            // gradually over several sweeps instead of snapping flat. harder
            // rock holds its angle better, so scale the move by softness.
            float soft = 1.0f - f->hardness[ci];
            float move = total * 0.5f * p->thermal_rate * soft;
            if (move <= 0.0f) continue;

            delta[ci] -= move;
            for (int k = 0; k < 4; k++) {
                if (excess[k] <= 0.0f) continue;
                int nx = x + NB_DX[k], nz = z + NB_DZ[k];
                float share = excess[k] / total;
                delta[erosion_idx(nx, nz)] += move * share;
            }
            moved += move;
        }
    }

    for (int i = 0; i < EROSION_CELLS; i++) f->height[i] += delta[i];
    return moved;
}

void erosion_thermal_pass(erosion_field *f, const erosion_params *p,
                          erosion_stats *st) {
    for (int it = 0; it < p->thermal_iters; it++) {
        float moved = erosion_thermal_sweep(f, p);
        if (st) {
            st->total_eroded += moved;     // slumped material counts both ways
            st->total_deposited += moved;
        }
        if (moved < 1e-4f) break;          // stable, no point sweeping more
    }
}
