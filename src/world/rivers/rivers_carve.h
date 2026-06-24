#ifndef WORLD_RIVERS_CARVE_H
#define WORLD_RIVERS_CARVE_H
#include "rivers_field.h"
#include "rivers_types.h"
// the carve pass is the only stage that produces world edits. it walks the wet
// plane and, per cell, decides what blocks to place:
//
// river  - cut a bed below the water surface (depth scales with accumulation),
// fill the channel with water up to water_y, drop a sand bed under it.
// lake   - fill from the basin floor up to water_y with water, sand the floor.
int rivers_carve_mark_banks(rivers_field *f, const rivers_params *p);
int rivers_carve_depth(const rivers_params *p, float accum);
int rivers_carve_emit(rivers_field *f, const rivers_params *p,
                      rivers_cell **out);
#endif
