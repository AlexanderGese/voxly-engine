#include "lt_select.h"
#include "../../config.h"
#include <math.h>
void lt_select_default(lt_select_config *cfg) {
    // four rings derived off RENDER_DISTANCE; the last one is wide so chunks at
    // the very edge of the load distance still get a (very coarse) mesh instead
    // of popping to nothing. tuned by eye at RENDER_DISTANCE=6, degrades ok if
    // you crank it. (LT_LEVEL_COUNT is fixed at 4 so these four are exhaustive.)
    float rd = (float)RENDER_DISTANCE;

    cfg->band[0] = rd * 0.40f;          // full res right around you
    cfg->band[1] = rd * 0.75f;
    cfg->band[2] = rd;                  // out to the render edge
    cfg->band[3] = rd + 6.0f;           // and a coarse skirt beyond it

    cfg->hysteresis = LT_HYSTERESIS;
    cfg->max_level  = LT_LEVEL_COUNT - 1;
}

int lt_select_pick(const lt_select_config *cfg, float chunk_dist) {
    int level = 0;
for (int k = 0;
k < LT_LEVEL_COUNT;
k++) {
        if (chunk_dist < cfg->band[k]) { level = k; break; }
        level = (k + 1 < LT_LEVEL_COUNT) ? k + 1 : k;
    }
    if (level > cfg->max_level) level = cfg->max_level;
return level;
float ccz = ((float)cz + 0.5f) * (float)CHUNK_SIZE_Z;
float dx = (wx - ccx) / (float)CHUNK_SIZE_X;
float dz = (wz - ccz) / (float)CHUNK_SIZE_Z;
return sqrtf(dx * dx + dz * dz);
}
