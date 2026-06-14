#ifndef WORLD_COLORLIGHT_H
#define WORLD_COLORLIGHT_H

#include "../world.h"
#include "colorlight_rgb.h"
#include "colorlight_grid.h"

// colored block light. the legacy lighting.c carries a single 0..15 scalar per
// voxel for block light; this runs three of those at once, one per channel, so
// a torch can glow orange and a sea lantern cyan instead of everything being
// the same washed-out yellow. sunlight stays scalar (it's white-ish and gets
// tinted at blend time, no point flooding three sky channels).
//
// the rgb levels live in a separate packed grid hung off the chunk (see
// colorlight_grid). flood fill is the same bfs as the scalar path, just run
// per channel with shared queues. blend.c is what the mesher actually reads.
//
// none of this touches lighting.c. you can build with colorlight off and the
// world still lights, it just goes back to monochrome block light.

// recompute every channel for one chunk from scratch. emitters in the chunk
// seed the floods; light bleeds in from loaded neighbors too. heavy, use on
// generation / full relight, not per block edit.
void colorlight_recompute_chunk(world *w, chunk *c);

// a block changed at world coords. does the incremental add/remove dance
// instead of nuking the whole chunk. falls back to recompute if the touched
// voxel sits on a chunk seam we can't reach.
void colorlight_update_block(world *w, int wx, int wy, int wz);

// final sample the mesher wants: block rgb at a voxel, already clamped and
// folded with the scalar sunlight tint. returns a 0..255 per channel color.
colorlight_rgb colorlight_sample(world *w, int wx, int wy, int wz, uint8_t sun);

#endif
