#ifndef WORLD_COLORLIGHT_ACCESS_H
#define WORLD_COLORLIGHT_ACCESS_H
#include "../world.h"
#include "colorlight_packed.h"
// world-space rgb light access. mirrors world_get_blocklight / set but for the
// colored grid, doing the chunk lookup + local conversion the flood would
// otherwise hand-roll at every step. reads off a missing/ungenerated chunk
// return 0 (dark), writes silently drop. this is the only place the flood is
// allowed to cross chunk seams.
colorlight_packed colorlight_world_get(world *w, int wx, int wy, int wz);
void              colorlight_world_set(world *w, int wx, int wy, int wz, colorlight_packed p);
uint8_t colorlight_world_get_chan(world *w, int wx, int wy, int wz, int chan);
void    colorlight_world_set_chan(world *w, int wx, int wy, int wz, int chan, uint8_t v);
// does the voxel let light through at all? wraps block_is_opaque so the flood
// and the seeder agree on what counts as a wall. air, glass, water, leaves all
// pass (water/leaves will attenuate later, see colorlight_prop).
int colorlight_world_passable(world *w, int wx, int wy, int wz);
#endif
