#ifndef RENDER_LODTERRAIN_LT_SELECT_H
#define RENDER_LODTERRAIN_LT_SELECT_H

#include "lt_config.h"

// distance -> lod level picker for the terrain meshes. this is the meshing-side
// twin of render/cull/lod: cull picks a label for visibility bookkeeping, we
// pick which coarse grid to actually build. they agree on direction (bigger
// number = coarser) but keep independent bands so you can tune them apart.

typedef struct {
    // ring radii in chunk units (not squared). level k applies when the chunk
    // distance is < band[k]. last band clamps to the coarsest level.
    float band[LT_LEVEL_COUNT];
    float hysteresis;   // fraction of band width, deadzone around boundaries
    int   max_level;    // clamp; lets you cap coarseness cheaply
} lt_select_config;

// fill with defaults derived from RENDER_DISTANCE.
void lt_select_default(lt_select_config *cfg);

// pick a level for a chunk-space distance (in chunk units), no hysteresis.
int  lt_select_pick(const lt_select_config *cfg, float chunk_dist);

// pick with hysteresis, given the level the chunk had last frame. pass
// prev_level < 0 if there's no history yet.
int  lt_select_pick_stable(const lt_select_config *cfg,
                           float chunk_dist, int prev_level);

// chunk-unit distance from a chunk address to a world position. pulled out so
// the manager and the picker agree on exactly the same metric.
float lt_select_chunk_dist(int cx, int cz, float wx, float wz);

#endif
