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
// bank   - any dry cell touching water gets its top block swapped to sand so
// shores read as beaches instead of grass cliffs.
// source - same as river but never deeper than 1, springs are shallow.
//
// edits land in a caller-owned darray of rivers_cell. the worldgen driver stamps
// them; this module never touches a chunk. that separation is the whole reason
// the pipeline is testable.

// bank-flagging pass: mark dry cells adjacent to water as RIVERS_BANK. run this
// after trace + fill, before emit. returns how many were flagged.
int rivers_carve_mark_banks(rivers_field *f, const rivers_params *p);

// channel depth for a given accumulation, clamped to max_depth. exposed so the
// emitter and any debug overlay agree on the number.
int rivers_carve_depth(const rivers_params *p, float accum);

// emit edits for the whole field into *out (a darray of rivers_cell). returns
// the number of cells emitted.
int rivers_carve_emit(rivers_field *f, const rivers_params *p,
                      rivers_cell **out);

#endif
