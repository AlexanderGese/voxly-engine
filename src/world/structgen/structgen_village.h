#ifndef WORLD_STRUCTGEN_VILLAGE_H
#define WORLD_STRUCTGEN_VILLAGE_H

#include "structgen_types.h"
#include "structgen_buffer.h"
#include "structgen_piece.h"

// village assembler. builds a plan around a central well, rings houses and
// farms onto a loose grid of plots, then stamps the whole thing. layout is
// data-driven by the site seed so a given anchor always grows the same village.

// fill `plan` with the chosen pieces. ground_y is the surface the village sits
// on (flat-ish assumed; the per-piece foundations absorb small dips).
void structgen_village_layout(structgen_plan *plan, const structgen_site *site);

// convenience: layout + build in one call. returns voxels emitted.
int  structgen_village_generate(structgen_buffer *out, const structgen_site *site);

#endif
