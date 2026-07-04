#include "lod.h"

#include "../../config.h"

// the bands grow quadratically-ish: lod0 close, then progressively bigger
// rings. these are tuned by eye against RENDER_DISTANCE=6. if you bump the
// render distance way up youll want to retune, but it degrades gracefully.

void cull_lod_default(cull_lod_config *cfg) {
    // thresholds in chunk-units squared. lod0 out to ~2 chunks, lod1 to
    // ~4, lod2 to ~RENDER_DISTANCE, lod3 beyond.
    float rd = (float)RENDER_DISTANCE;

    cfg->band_sq[CULL_LOD0] = 2.0f * 2.0f;
    cfg->band_sq[CULL_LOD1] = 4.0f * 4.0f;
    cfg->band_sq[CULL_LOD2] = rd * rd;
    cfg->band_sq[CULL_LOD3] = (rd + 4.0f) * (rd + 4.0f);

    cfg->hysteresis = 0.08f;
    cfg->max_lod    = CULL_LOD3;
}

int cull_lod_pick(const cull_lod_config *cfg, float chunk_dist_sq) {
    int lod = CULL_LOD0;
    for (int k = 0; k < CULL_LOD_COUNT; k++) {
        if (chunk_dist_sq < cfg->band_sq[k]) { lod = k; break; }
        lod = (k + 1 < CULL_LOD_COUNT) ? k + 1 : k;
    }
    if (lod > cfg->max_lod) lod = cfg->max_lod;
    return lod;
}

int cull_lod_pick_stable(const cull_lod_config *cfg,
                         float chunk_dist_sq, int prev_lod) {
    int want = cull_lod_pick(cfg, chunk_dist_sq);
    if (prev_lod < 0 || prev_lod == want) return want;

    // only switch if weve crossed the boundary by more than the deadzone.
    // figure out the boundary between prev_lod and want, widen it.
    int lo = prev_lod < want ? prev_lod : want;   // lower lod number
    if (lo >= CULL_LOD_COUNT) return want;

    float boundary = cfg->band_sq[lo];
    float prev_b   = (lo > 0) ? cfg->band_sq[lo - 1] : 0.0f;
    float width    = boundary - prev_b;
    float dead     = width * cfg->hysteresis;

    if (want > prev_lod) {
        // moving away: require crossing boundary + dead
        if (chunk_dist_sq < boundary + dead) return prev_lod;
    } else {
        // moving closer: require crossing boundary - dead
        if (chunk_dist_sq > boundary - dead) return prev_lod;
    }
    if (want > cfg->max_lod) want = cfg->max_lod;
    return want;
}

float cull_lod_tri_scale(int lod) {
    switch (lod) {
    case CULL_LOD0: return 1.0f;
    case CULL_LOD1: return 0.45f;
    case CULL_LOD2: return 0.18f;
    case CULL_LOD3: return 0.06f;
    default:        return 0.04f;
    }
}
