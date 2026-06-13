#ifndef WORLD_BIOME_LOOKUP_H
#define WORLD_BIOME_LOOKUP_H

#include "biome_def.h"
#include "biome_climate.h"

// climate -> biome. we score every registered biome by weighted squared
// distance from its envelope centroid to the sampled climate, and take the
// closest. this is basically nearest-centroid classification over the param
// space. cheap, deterministic, and easy to retune by moving centroids.

biome_kind biome_lookup_pick(const biome_climate *c);

// same but writes the winning score (lower == better fit). handy for blending
// confidence and for the edge detector.
biome_kind biome_lookup_pick_scored(const biome_climate *c, float *out_score);

// the k closest biomes, sorted best-first. fills ids[] and dist2[] up to k,
// returns how many were written. used by the blender to weight neighbors.
int biome_lookup_topk(const biome_climate *c, int k,
                      biome_kind *ids, float *dist2);

// raw weighted squared distance from climate to a specific biome envelope.
float biome_lookup_dist2(const biome_climate *c, const biome_def *def);

#endif
