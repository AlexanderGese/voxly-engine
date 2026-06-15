#ifndef WORLD_EROSION_FIELD_H
#define WORLD_EROSION_FIELD_H
#include <stdint.h>
#include "erosion_types.h"
#include "../../math/vec2.h"
// accessors and sampling for the float heightmap. the droplet sim lives in
// continuous space so everything here is bilinear: you ask for the height at
// (3.7, 9.2) and get a smooth surface, not a staircase. the gradient comes
// from the same bilinear patch so the marble rolls where the surface tips.
// zero a field and stamp its world origin. hardness/sediment cleared too.
void erosion_field_clear(erosion_field *f, int ox, int oz);
// seed per-cell rock hardness from value noise so erosion isnt uniform.
// base is the floor hardness, amp the noise contribution. both in [0,1].
void erosion_field_seed_hardness(erosion_field *f, const erosion_params *p,
                                 float base, float amp);
// raw cell read/write, clamped to the field so the skirt absorbs overruns.
float erosion_height_at(const erosion_field *f, int x, int z);
void  erosion_height_set(erosion_field *f, int x, int z, float h);
void  erosion_height_add(erosion_field *f, int x, int z, float d);
// bilinear height at a continuous position. clamps to the valid interior.
float erosion_sample_height(const erosion_field *f, vec2 p);
// downhill gradient of the bilinear patch at p. points uphill (df/dx, df/dz);
// the droplet moves along -gradient. returns {0,0} dead flat.
vec2 erosion_sample_gradient(const erosion_field *f, vec2 p);
// bilinear hardness, used to scale how much a droplet may carve here.
float erosion_sample_hardness(const erosion_field *f, vec2 p);
// largest absolute height difference to any 8-neighbour. used by the thermal
// pass to find unstable cells and by stats for the steepest delta.
float erosion_cell_relief(const erosion_field *f, int x, int z);
#endif
