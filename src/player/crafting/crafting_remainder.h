#ifndef PLAYER_CRAFTING_REMAINDER_H
#define PLAYER_CRAFTING_REMAINDER_H

#include "crafting_types.h"

// "remainder" / byproduct handling. some ingredients aren't consumed when you
// craft, they leave something behind in the cell instead. classic example is a
// bucket: craft a cake and the bucket comes back empty (well, we dont have
// cakes, but glass-from-sand-with-a-mould reads the same). the table calls
// into here after consuming so the leftover stays in the grid.
//
// keyed flat by block_id, 256 entries, BLOCK_AIR meaning "fully consumed".

// install the default remainder table (idempotent). registered at init.
void craft_remainder_init(void);

// register: when `ingredient` is consumed in a craft, leave `leftover` behind
// instead of clearing the cell. pass leftover==BLOCK_AIR to clear a mapping.
void craft_remainder_set(block_id ingredient, block_id leftover);

// what does consuming `ingredient` leave behind? BLOCK_AIR == nothing.
block_id craft_remainder_of(block_id ingredient);

// does this id leave anything when crafted with? cheap predicate.
int craft_remainder_has(block_id ingredient);

// apply remainders to a grid after a single craft. for each occupied cell,
// if its item has a remainder, the cell is rewritten to hold the leftover
// (count preserved) instead of being decremented to nothing. returns the
// number of cells that got a leftover. the table uses this in place of the
// plain consume when any grid item has a remainder.
struct craft_grid;
int craft_remainder_apply(struct craft_grid *g);

// count how many distinct mappings are live. mostly for tests / the book ui.
int craft_remainder_count(void);

#endif
