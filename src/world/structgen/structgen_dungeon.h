#ifndef WORLD_STRUCTGEN_DUNGEON_H
#define WORLD_STRUCTGEN_DUNGEON_H

#include "structgen_types.h"
#include "structgen_buffer.h"
#include "structgen_piece.h"

// dungeon assembler. grows a small graph of brick rooms underground, connecting
// each new room back to an existing one with a corridor. placement is a guided
// random walk: pick an open room, pick a free side, project a corridor + room,
// reject on overlap. bounded by a room budget so it never runs away.

// fill plan with rooms+corridors. ground_y is the surface; the dungeon buries
// itself a fixed depth below it.
void structgen_dungeon_layout(structgen_plan *plan, const structgen_site *site);

// layout + build. returns voxels emitted.
int  structgen_dungeon_generate(structgen_buffer *out, const structgen_site *site);

#endif
