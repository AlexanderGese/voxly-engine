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
    for (int k = 0; k < LT_LEVEL_COUNT; k++) {
        if (chunk_dist < cfg->band[k]) { level = k; break; }
        level = (k + 1 < LT_LEVEL_COUNT) ? k + 1 : k;
    }
    if (level > cfg->max_level) level = cfg->max_level;
    return level;
}

int lt_select_pick_stable(const lt_select_config *cfg,
                          float chunk_dist, int prev_level) {
    int want = lt_select_pick(cfg, chunk_dist);
    if (prev_level < 0 || prev_level == want) return want;

    // find the boundary that sits between the two candidate levels and only
    // commit to crossing it once we're a deadzone past it. same shape as the
    // cull-side picker but on linear distance, not squared.
    int lo = prev_level < want ? prev_level : want;
    if (lo >= LT_LEVEL_COUNT) return want;

    float boundary = cfg->band[lo];
    float prev_b   = (lo > 0) ? cfg->band[lo - 1] : 0.0f;
    float width    = boundary - prev_b;
    float dead     = width * cfg->hysteresis;

    if (want > prev_level) {
        // receding: don't coarsen until we're clearly past the ring
        if (chunk_dist < boundary + dead) return prev_level;
    } else {
        // approaching: don't refine until we're clearly inside it
        if (chunk_dist > boundary - dead) return prev_level;
    }
    if (want > cfg->max_level) want = cfg->max_level;
    return want;
}

float lt_select_chunk_dist(int cx, int cz, float wx, float wz) {
    // chunk centre in world block coords, then convert the gap to chunk units.
    float ccx = ((float)cx + 0.5f) * (float)CHUNK_SIZE_X;
    float ccz = ((float)cz + 0.5f) * (float)CHUNK_SIZE_Z;
    float dx = (wx - ccx) / (float)CHUNK_SIZE_X;
    float dz = (wz - ccz) / (float)CHUNK_SIZE_Z;
    return sqrtf(dx * dx + dz * dz);
}
