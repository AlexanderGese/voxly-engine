#ifndef WORLD_GEN2_ORE_DISTRIBUTION_H
#define WORLD_GEN2_ORE_DISTRIBUTION_H

#include "gen2_place.h"

// height-banded ore distribution. each ore kind likes a y range and a
// rarity. we only have BLOCK_COBBLE as an "ore-ish" block, so all veins
// emit cobble for now but the band logic is the real point.

typedef struct {
    const char *name;
    int   y_min, y_max;   // valid spawn band
    int   per_chunk;      // vein attempts in a 16x16 column region
    int   vein_min, vein_max;
    block_id block;
} gen2_ore_kind;

// number of ore kinds in the table.
int gen2_ore_kind_count(void);
const gen2_ore_kind *gen2_ore_kind_at(int i);

// 1 if an ore of kind k is allowed at world y, else 0.
int gen2_ore_allowed_at_y(int k, int y);

// generate veins for a chunk-sized region rooted at (ox,oy,oz)=(chunk origin).
// emits blob placements into out. region_w/region_d are the column extents.
// returns total blocks emitted.
int gen2_ore_generate(gen2_place_buf *out, int origin_x, int origin_z,
                      int region_w, int region_d, int max_y, uint32_t seed);

#endif
