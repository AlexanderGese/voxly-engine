#ifndef WORLD_OREGEN_TABLE_H
#define WORLD_OREGEN_TABLE_H

#include "oregen_types.h"

// the static ore table. classic mc-ish layering: coal everywhere, iron a
// bit lower, gold/redstone deep, diamond hugging bedrock. we lack ore
// textures so every kind still emits cobble for now, but the bands, curves
// and shapes are the real content and swap to real blocks the day we have
// the atlas tiles.

int               oregen_table_count(void);
const oregen_ore *oregen_table_at(int i);

// look an ore up by name. returns NULL if not found. linear scan, the
// table is tiny.
const oregen_ore *oregen_table_find(const char *name);

// the deepest y any ore in the table wants. handy for clamping the driver's
// scan range so we dont iterate dead air.
int oregen_table_max_y(void);
int oregen_table_min_y(void);

#endif
