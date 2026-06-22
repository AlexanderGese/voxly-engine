#ifndef WORLD_OREGEN_LAYER_H
#define WORLD_OREGEN_LAYER_H
#include "oregen_types.h"
// named depth strata. the ore table bands overlap messily on purpose, but
// for tuning and debug overlays it helps to have coarse human-readable
// layers ("deepslate", "stone", "surface") and ask which ores belong to
// each. this is descriptive metadata over the table, it doesnt drive
// placement directly.
typedef enum {
    OREGEN_LAYER_DEEP = 0,   // bedrock-adjacent, the diamond/redstone floor
    OREGEN_LAYER_MID,        // the bulk of the stone column
    OREGEN_LAYER_UPPER,      // near surface, coal/copper territory
    OREGEN_LAYER_COUNT
} oregen_layer_id;
typedef struct {
    const char *name;
    int y_lo, y_hi;          // inclusive world-y span of the stratum
} oregen_layer;
// the stratum a world-y falls into. clamps: anything below DEEP's floor is
// DEEP, anything above UPPER's ceiling is UPPER.
oregen_layer_id oregen_layer_of(int y);
const oregen_layer *oregen_layer_get(oregen_layer_id id);
// 1 if ore kind k has any band overlap with the given stratum.
int oregen_layer_has_ore(oregen_layer_id layer, int kind);
// fill out[] with the ore kind indices that overlap a stratum, in table
// order. returns the count written (<= out_cap).
int oregen_layer_ores(oregen_layer_id layer, int *out, int out_cap);
// the stratum where ore kind k is densest, by sampling its curve at the
// midpoint of each stratum. ties go to the deeper layer.
oregen_layer_id oregen_layer_peak_of(int kind);
#endif
