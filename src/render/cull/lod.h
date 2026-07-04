#ifndef RENDER_CULL_LOD_H
#define RENDER_CULL_LOD_H

#include "../../math/vec3.h"
#include "cull_types.h"

// chunk lod selection. purely distance driven for now, with a little bit
// of hysteresis so chunks dont flicker between levels when you stand on a
// boundary and wiggle the mouse.

typedef struct {
    // squared distance thresholds (chunk units, not blocks) where each lod
    // kicks in. lod[k] applies when dist_sq >= band[k-1] && < band[k].
    float band_sq[CULL_LOD_COUNT];
    float hysteresis;   // fraction of band width, e.g. 0.08 = 8% deadzone
    int   max_lod;      // clamp, lets you disable far lods cheaply
} cull_lod_config;

// fill with sensible defaults derived from RENDER_DISTANCE.
void cull_lod_default(cull_lod_config *cfg);

// pick a lod for a chunk-space distance, ignoring hysteresis. dist is in
// chunk units (distance / CHUNK_SIZE), squared.
int  cull_lod_pick(const cull_lod_config *cfg, float chunk_dist_sq);

// pick with hysteresis given the lod the chunk had last frame. prevents
// thrash at the boundaries. pass prev_lod < 0 if unknown.
int  cull_lod_pick_stable(const cull_lod_config *cfg,
                          float chunk_dist_sq, int prev_lod);

// rough triangle scale factor for a lod, used by the stats estimate.
// lod0 = 1.0, each step roughly quarters the geometry.
float cull_lod_tri_scale(int lod);

#endif
