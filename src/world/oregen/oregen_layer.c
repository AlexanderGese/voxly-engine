#include "oregen_layer.h"
#include "oregen_table.h"
#include "oregen_curve.h"
#include "../../config.h"

// coarse strata over the 0..CHUNK_SIZE_Y column. tuned to roughly match the
// table bands: deep is the rare-ore floor, upper is the common-ore cap.
static const oregen_layer g_layers[OREGEN_LAYER_COUNT] = {
    { "deep",  0,  20 },
    { "mid",  21,  72 },
    { "upper",73, CHUNK_SIZE_Y - 1 },
};

oregen_layer_id oregen_layer_of(int y) {
    if (y <= g_layers[OREGEN_LAYER_DEEP].y_hi)  return OREGEN_LAYER_DEEP;
    if (y <= g_layers[OREGEN_LAYER_MID].y_hi)   return OREGEN_LAYER_MID;
    return OREGEN_LAYER_UPPER;
}

const oregen_layer *oregen_layer_get(oregen_layer_id id) {
    if (id < 0 || id >= OREGEN_LAYER_COUNT) return &g_layers[0];
    return &g_layers[id];
}

// do [a_lo,a_hi] and [b_lo,b_hi] overlap at all.
static int spans_overlap(int a_lo, int a_hi, int b_lo, int b_hi) {
    return a_lo <= b_hi && b_lo <= a_hi;
}

int oregen_layer_has_ore(oregen_layer_id layer, int kind) {
    const oregen_layer *L = oregen_layer_get(layer);
    const oregen_ore   *o = oregen_table_at(kind);
    return spans_overlap(L->y_lo, L->y_hi, o->y_min, o->y_max);
}

int oregen_layer_ores(oregen_layer_id layer, int *out, int out_cap) {
    if (!out || out_cap <= 0) return 0;
    int n = 0;
    int kinds = oregen_table_count();
    for (int k = 0; k < kinds && n < out_cap; k++) {
        if (oregen_layer_has_ore(layer, k)) out[n++] = k;
    }
    return n;
}

oregen_layer_id oregen_layer_peak_of(int kind) {
    const oregen_ore *o = oregen_table_at(kind);

    oregen_layer_id best = OREGEN_LAYER_DEEP;
    float best_w = -1.0f;

    // walk shallow->deep so ties resolve to the deeper layer (>= test).
    for (int id = OREGEN_LAYER_UPPER; id >= OREGEN_LAYER_DEEP; id--) {
        const oregen_layer *L = oregen_layer_get((oregen_layer_id)id);
        if (!spans_overlap(L->y_lo, L->y_hi, o->y_min, o->y_max)) continue;

        // sample the curve at the stratum midpoint, clamped into the band.
        int mid = (L->y_lo + L->y_hi) / 2;
        if (mid < o->y_min) mid = o->y_min;
        if (mid > o->y_max) mid = o->y_max;

        float w = oregen_curve_weight(o, mid);
        if (w >= best_w) {
            best_w = w;
            best = (oregen_layer_id)id;
        }
    }
    return best;
}
