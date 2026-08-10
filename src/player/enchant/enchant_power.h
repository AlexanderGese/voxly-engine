#ifndef PLAYER_ENCHANT_POWER_H
#define PLAYER_ENCHANT_POWER_H
#include "enchant_types.h"
#include "../../world/world.h"
// the table's enchanting power. driven by how many bookshelves surround it,
// the same way it works in the obvious game we're not naming. a bookshelf
// only counts if there's an air gap between it and the table and nothing
// opaque blocking the line, so people can't just wall the table in solid
int enchant_power_count_shelves(world *w, int tx, int ty, int tz);
void enchant_power_slot_levels(int shelves, int out_levels[ENCHANT_TABLE_SLOTS]);
int enchant_power_shelf_offsets(int *dx, int *dy, int *dz, int max);
#endif
