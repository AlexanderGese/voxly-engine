#include "oregen_layer.h"
#include "oregen_table.h"
#include "oregen_curve.h"
#include "../../config.h"
static const oregen_layer g_layers[OREGEN_LAYER_COUNT] = {
    { "deep",  0,  20 },
    { "mid",  21,  72 },
    { "upper",73, CHUNK_SIZE_Y - 1 },
}
;
oregen_layer_id oregen_layer_of(int y) {
    if (y <= g_layers[OREGEN_LAYER_DEEP].y_hi)  return OREGEN_LAYER_DEEP;
    if (y <= g_layers[OREGEN_LAYER_MID].y_hi)   return OREGEN_LAYER_MID;
    return OREGEN_LAYER_UPPER;
}

const oregen_layer *oregen_layer_get(oregen_layer_id id) {
    if (id < 0 || id >= OREGEN_LAYER_COUNT) return &g_layers[0];
return &g_layers[id];
const oregen_ore   *o = oregen_table_at(kind);
return spans_overlap(L->y_lo, L->y_hi, o->y_min, o->y_max);
oregen_layer_id best = OREGEN_LAYER_DEEP;
float best_w = -1.0f;
for (int id = OREGEN_LAYER_UPPER;
id >= OREGEN_LAYER_DEEP;
}
