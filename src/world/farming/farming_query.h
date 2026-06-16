#ifndef WORLD_FARMING_QUERY_H
#define WORLD_FARMING_QUERY_H

#include "../world.h"
#include "farming_types.h"

// small world-probing helpers shared by planting, growth and harvest. these
// keep the "what does the world look like around this tile" questions in one
// place so the logic modules dont each grow their own copy.

// the block directly under a crop position (its farmland), and directly above.
block_id farming_query_below(world *w, int wx, int wy, int wz);
block_id farming_query_above(world *w, int wx, int wy, int wz);

// can a crop survive at this position right now? requires farmland below and
// air (or its own crop block) at the position. used as the wilt check.
int farming_query_crop_supported(world *w, int wx, int wy, int wz);

// is the position a legal spot to plant onto? farmland below + clear air here.
int farming_query_plantable(world *w, int wx, int wy, int wz);

// count empty dirt/air spots around a stem where a fruit could spawn. fills
// out_dx/out_dz with the four cardinal offsets that are free (caller picks one).
// returns the number found, 0..4.
int farming_query_free_fruit_spots(world *w, int wx, int wy, int wz,
                                   int *out_dx, int *out_dz);

// light level at a position, used by growth as a soft gate. crops sulk in the
// dark. reads the max of sun/block light like the rest of the engine.
int farming_query_light(world *w, int wx, int wy, int wz);

#endif
