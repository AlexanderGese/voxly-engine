#ifndef WORLD_GEN2_TREE_DISPATCH_H
#define WORLD_GEN2_TREE_DISPATCH_H

#include "gen2_place.h"
#include "gen2_types.h"

// picks which tree species fits a biome and rolls whether a tree grows on a
// given column, then emits it. ties the per-biome tree_density knob to the
// individual tree builders (oak/pine/palm).

typedef enum {
    GEN2_TREE_NONE = 0,
    GEN2_TREE_OAK,
    GEN2_TREE_PINE,
    GEN2_TREE_PALM
} gen2_tree_species;

// which species suits this biome.
gen2_tree_species gen2_tree_species_for(gen2_biome_id biome);

// roll: should a tree spawn on this exact column? uses biome tree_density.
int gen2_tree_should_grow(const gen2_column *col);

// grow the appropriate tree on top of the column. returns blocks emitted
// (0 if none grew or column is unsuitable, e.g. underwater).
int gen2_tree_grow(gen2_place_buf *out, const gen2_column *col);

#endif
