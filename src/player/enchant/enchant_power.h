#ifndef PLAYER_ENCHANT_POWER_H
#define PLAYER_ENCHANT_POWER_H

#include "enchant_types.h"
#include "../../world/world.h"

// the table's enchanting power. driven by how many bookshelves surround it,
// the same way it works in the obvious game we're not naming. a bookshelf
// only counts if there's an air gap between it and the table and nothing
// opaque blocking the line, so people can't just wall the table in solid
// books and call it a library.

// scan the world around a table at (tx,ty,tz) and count valid bookshelves.
// caps at ENCHANT_MAX_BOOKSHELVES. uses BLOCK_WOOD as the stand-in shelf
// block since the engine has no dedicated bookshelf id (yet).
int enchant_power_count_shelves(world *w, int tx, int ty, int tz);

// turn a shelf count into the three offered enchant "levels" (the numbers
// shown on the slots). these are the cost-in-xp-levels for each slot, with
// the bottom slot scaling hardest with power.
void enchant_power_slot_levels(int shelves, int out_levels[ENCHANT_TABLE_SLOTS]);

// the canonical bookshelf positions checked around a table: 15 spots at the
// outer ring, two height layers. fills xs/ys/zs (each length 15) with the
// offsets and returns how many it wrote. exposed mostly so tests and the
// debug overlay agree with the scanner on geometry.
int enchant_power_shelf_offsets(int *dx, int *dy, int *dz, int max);

#endif
