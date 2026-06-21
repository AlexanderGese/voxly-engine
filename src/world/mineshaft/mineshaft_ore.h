#ifndef WORLD_MINESHAFT_ORE_H
#define WORLD_MINESHAFT_ORE_H

#include "mineshaft_buffer.h"
#include "mineshaft_box.h"
#include "mineshaft_rand.h"
#include "mineshaft_types.h"

// ore rooms: widened chambers at the end of a branch where the miners were
// clearly digging something out. we carve the chamber, expose veins of ore in
// the walls, scatter rubble on the floor, and drop a loot-chest marker. ore /
// chest block ids come from the config palette (cfg->ore_common, ore_rare,
// mat_chest, mat_fill) so the host atlas drives the look.

// carve a room at the cell box, seed ore veins into its walls, pile rubble on
// the floor, and place one chest. returns voxels added.
int mineshaft_ore_room(mineshaft_buffer *b, const mineshaft_config *cfg,
                       mineshaft_box cell_box, int floor_y, int ceil_y,
                       mineshaft_rng *rng, uint32_t seed);

// grow a small connected vein of `ore` from (x,y,z), random-walking up to `size`
// steps. returns ore blocks placed. blobby and connected, not a speck-scatter.
int mineshaft_ore_vein(mineshaft_buffer *b, int x, int y, int z, int size,
                       block_id ore, mineshaft_rng *rng);

#endif
