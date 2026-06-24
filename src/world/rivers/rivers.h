#ifndef WORLD_RIVERS_H
#define WORLD_RIVERS_H
#include "rivers_types.h"
#include "rivers_field.h"
typedef int (*rivers_height_fn)(int wx, int wz, void *user);
typedef struct {
    rivers_field   *field;     // owned scratch, reused across regions
    rivers_params   params;
    rivers_height_fn height;   // surface sampler
    void           *user;      // passed back to height

    // last-run stats, handy for debug overlays and the worldgen log line.
    int lake_cells;
    int river_cells;
    int bank_cells;
    int edits;
    float peak_accum;
} rivers_ctx;
void rivers_ctx_init(rivers_ctx *ctx, uint32_t seed, int sea_level,
                     rivers_height_fn height, void *user);
#endif
