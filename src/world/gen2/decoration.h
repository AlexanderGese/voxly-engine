#ifndef WORLD_GEN2_DECORATION_H
#define WORLD_GEN2_DECORATION_H

#include "gen2_place.h"
#include "gen2_types.h"

// small surface scatter: grass tufts, flowers, mushrooms. one decision per
// column, deterministic. we dont have flower blocks so we improvise with the
// blocks that exist (leaves as bush, torch as "mushroom" stand-in, etc).

typedef enum {
    GEN2_DECO_NONE = 0,
    GEN2_DECO_GRASS,
    GEN2_DECO_BUSH,
    GEN2_DECO_MUSHROOM,
    GEN2_DECO_DEADBUSH
} gen2_deco_kind;

// decide what (if anything) sits on top of this column.
gen2_deco_kind gen2_deco_pick(const gen2_column *col);

// emit the chosen decoration one block above the surface. returns count.
int gen2_deco_place(gen2_place_buf *out, const gen2_column *col);

#endif
