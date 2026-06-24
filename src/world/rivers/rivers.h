#ifndef WORLD_RIVERS_H
#define WORLD_RIVERS_H

#include "rivers_types.h"
#include "rivers_field.h"

// public face of the river + lake carver. the worldgen driver only needs this
// header: hand it a region origin and a way to sample terrain height, get back
// a list of block edits to stamp.
//
// the pipeline, in order:
// 1. fill        depression-fill the surface, pond the closed basins -> lakes
// 2. flow        d8 directions + flow accumulation off the filled surface
// 3. trace       threshold cells become rivers, springs become sources
// 4. levels      resolve a monotone water surface so nothing runs uphill
// 5. banks       flag the shores for sanding
// 6. carve       emit the bed / water / bank edits
//
// each stage is its own translation unit; this just sequences them and owns the
// scratch field. nothing here talks to a chunk — the caller stamps the edits.

// height sampler the driver provides. given a world column it returns the solid
// surface y. the carver calls this to fill the field's surface plane.
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

// set up a context. allocates the scratch field. seed + sea_level seed the
// default params, which you can tweak afterward via ctx.params.
void rivers_ctx_init(rivers_ctx *ctx, uint32_t seed, int sea_level,
                     rivers_height_fn height, void *user);
void rivers_ctx_free(rivers_ctx *ctx);

// run the whole pipeline for one chunk-sized region and append the resulting
// block edits to *out (a darray of rivers_cell, may start NULL). returns the
// number of edits appended. stats land in the ctx.
int rivers_generate_region(rivers_ctx *ctx, int chunk_cx, int chunk_cz,
                           rivers_cell **out);

// query: after a run, was (wx,wz) wet, and at what water surface? returns the
// rivers_wet state and writes water_y if non-null. useful for biome/deco passes
// that want to avoid planting trees in a riverbed.
rivers_wet rivers_query(const rivers_ctx *ctx, int wx, int wz, int *water_y);

#endif
