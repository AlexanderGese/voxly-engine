#ifndef WORLD_HEIGHTMAP_H
#define WORLD_HEIGHTMAP_H

#include <stdint.h>
#include "heightmap_params.h"
#include "heightmap_column.h"
#include "heightmap_field.h"
#include "heightmap_fill.h"

// public face of the spline-terrain heightmap generator. continentalness +
// erosion + peaks/valleys splines, the lot. you build one generator from the
// world seed, then ask it for column heights or hand it a chunk-sized scratch
// buffer to fill. the generator owns the params and the curve tables; columns
// and fields are scratch the caller provides.
//
// none of this touches a chunk struct directly. the worldgen driver pulls a
// height (or a filled column) out of here and stamps blocks itself. keeps the
// terrain math from depending on the chunk layout, which has changed twice now.

typedef struct {
    heightmap_params params;
} heightmap_gen;

// build a generator from the world seed and the engine sea level. wires up the
// default params and makes sure the spline tables are built.
void heightmap_gen_init(heightmap_gen *g, uint32_t seed, int sea_level);

// build a generator from explicit params, for tests and the worldgen tuning
// screen. copies the params in; safe to discard the source after.
void heightmap_gen_init_params(heightmap_gen *g, const heightmap_params *p);

// surface y for a single world column. the common query.
int  heightmap_gen_height(const heightmap_gen *g, int wx, int wz);

// resolve the full column breakdown.
void heightmap_gen_column(const heightmap_gen *g, int wx, int wz,
                          heightmap_column *out);

// build the whole chunk-footprint height field rooted at a chunk origin.
void heightmap_gen_field(const heightmap_gen *g, int cx_world, int cz_world,
                         heightmap_field *out);

// fill one column's worth of blocks into out[y0..y1] for the chunk at this
// world xz. convenience wrapper that resolves the column then runs the fill.
// returns solid block count.
int  heightmap_gen_fill_column(const heightmap_gen *g, int wx, int wz,
                               int y0, int y1, uint8_t *out);

#endif
