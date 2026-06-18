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
typedef struct {
    heightmap_params params;
} heightmap_gen;
void heightmap_gen_init(heightmap_gen *g, uint32_t seed, int sea_level);
void heightmap_gen_init_params(heightmap_gen *g, const heightmap_params *p);
int  heightmap_gen_height(const heightmap_gen *g, int wx, int wz);
void heightmap_gen_column(const heightmap_gen *g, int wx, int wz,
                          heightmap_column *out);
void heightmap_gen_field(const heightmap_gen *g, int cx_world, int cz_world,
                         heightmap_field *out);
int  heightmap_gen_fill_column(const heightmap_gen *g, int wx, int wz,
                               int y0, int y1, uint8_t *out);
#endif
