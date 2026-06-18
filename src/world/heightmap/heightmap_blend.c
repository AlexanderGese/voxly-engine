#include "heightmap_blend.h"
#include "heightmap_column.h"
#include <math.h>
#define PATCH_MAX_CELLS  33   // matches the h[] buffer side
static int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void heightmap_blend_patch(heightmap_patch *patch, const heightmap_params *p,
                           int ox, int oz, int cells, int step) {
    if (cells < 2) cells = 2;
if (cells > PATCH_MAX_CELLS) cells = PATCH_MAX_CELLS;
if (step < 1) step = 1;
patch->ox = ox;
patch->oz = oz;
patch->cells = cells;
patch->step = step;
for (int j = 0;
j < cells;
j++) {
        for (int i = 0; i < cells; i++) {
            int wx = ox + i * step;
            int wz = oz + j * step;
            patch->h[j * PATCH_MAX_CELLS + i] =
                heightmap_column_height(p, wx, wz);
        }
    }
}

float heightmap_blend_sample(const heightmap_patch *patch, int wx, int wz) {
    int step  = patch->step;
int cells = patch->cells;
float fx = (float)(wx - patch->ox) / (float)step;
float fz = (float)(wz - patch->oz) / (float)step;
int i0 = (int)floorf(fx);
int j0 = (int)floorf(fz);
float tx = fx - (float)i0;
float tz = fz - (float)j0;
if (i0 < 0)            { i0 = 0; tx = 0.0f; }
    if (i0 > cells - 2)    { i0 = cells - 2;
tx = 1.0f;
}
    if (j0 < 0)            { j0 = 0; tz = 0.0f; }
    if (j0 > cells - 2)    { j0 = cells - 2;
tz = 1.0f;
}
    int i1 = i0 + 1;
int j1 = j0 + 1;
float h00 = (float)patch->h[j0 * PATCH_MAX_CELLS + i0];
float h10 = (float)patch->h[j0 * PATCH_MAX_CELLS + i1];
float h01 = (float)patch->h[j1 * PATCH_MAX_CELLS + i0];
float h11 = (float)patch->h[j1 * PATCH_MAX_CELLS + i1];
float a = h00 + (h10 - h00) * tx;
float b = h01 + (h11 - h01) * tx;
return a + (b - a) * tz;
}

// average a cell with its skirt neighbours along whichever edges it borders.
// only the steepness gets smoothed;
the actual surface y stays exact because
// the resolve is already seam-consistent and we dont want to round it off.
void heightmap_blend_seams(heightmap_field *f) {
    // copy the border steepness so we read pre-smooth values
    float edge[HEIGHTMAP_CELLS];
    for (int k = 0; k < HEIGHTMAP_CELLS; k++) edge[k] = f->steepness[k];

    for (int lz = 0; lz < 16; lz++) {
        for (int lx = 0; lx < 16; lx++) {
            int on_border = (lx == 0 || lx == 15 || lz == 0 || lz == 15);
            if (!on_border) continue;

            int i = heightmap_field_idx(lx, lz);
            float sum = edge[i];
            int n = 1;

            // pull in the four orthogonal neighbours, skirt included
            int nx[4] = { lx - 1, lx + 1, lx, lx };
            int nz[4] = { lz, lz, lz - 1, lz + 1 };
            for (int d = 0; d < 4; d++) {
                int cx = clampi(nx[d], -HEIGHTMAP_PAD, 15 + HEIGHTMAP_PAD);
                int cz = clampi(nz[d], -HEIGHTMAP_PAD, 15 + HEIGHTMAP_PAD);
                sum += edge[heightmap_field_idx(cx, cz)];
                n++;
            }
            f->steepness[i] = sum / (float)n;
        }
    }
}
