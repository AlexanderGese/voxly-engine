#ifndef WORLD_HEIGHTMAP_COLUMN_H
#define WORLD_HEIGHTMAP_COLUMN_H

#include "heightmap_params.h"
#include "heightmap_sample.h"

// the resolved terrain for one world column. building one runs the whole
// spline pipeline: sample the noise fields, push each through its curve, blend
// continent + erosion-scaled peaks/valleys into a final top y. pure numbers,
// no chunk or block refs, so worldgen can call it from any thread and stamp
// the result however it likes.

typedef struct {
    int   wx, wz;          // world column coords
    int   surface_y;       // final terrain top, world y
    float continental;     // continent spline output (height factor)
    float erosion_factor;  // 0..1 relief multiplier from the erosion spline
    float relief;          // peaks/valleys spline output before scaling
    int   underwater;      // 1 if surface_y < sea level
    float steepness;       // |spline slope| estimate, for surface rules
    heightmap_fields fields; // the raw noise, kept for the biome picker
} heightmap_column;

// resolve a single column. this is the function the rest of worldgen calls.
void heightmap_column_resolve(const heightmap_params *p, int wx, int wz,
                              heightmap_column *out);

// just the height, when the caller doesnt need the breakdown. internally still
// resolves the whole column, its cheap enough.
int  heightmap_column_height(const heightmap_params *p, int wx, int wz);

// classify the column surface block from the resolved data. beach near sea
// level, snow up high, stone on steep faces, grass otherwise. returns a
// block_id-compatible int so this header doesnt have to pull in block.h.
int  heightmap_column_surface_block(const heightmap_params *p,
                                    const heightmap_column *col);

#endif
