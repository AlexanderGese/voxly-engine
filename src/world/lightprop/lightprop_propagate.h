#ifndef WORLD_LIGHTPROP_PROPAGATE_H
#define WORLD_LIGHTPROP_PROPAGATE_H
#include "lightprop_types.h"
#include "lightprop_queue.h"
// the add side of the algorithm: drain `q` of seeded cells and flood their light
// outward, writing into the world as it goes. `q` should already be primed with
// source cells (their light values already set in the world). on return q is
// empty. works for either channel.
//
// this is the workhorse; both "place a torch" and "expose a column to sky" feed
// into it. it never lowers a cell's light, only raises, so it's safe to run with
// stale-but-too-low values already present.
void lp_flood(world *w, lp_channel ch, lp_queue *q);
void lp_flood_bounded(world *w, lp_channel ch, lp_queue *q,
                      int cx, int cy, int cz, int radius);
int  lp_seed(world *w, lp_channel ch, lp_queue *q, int wx, int wy, int wz, uint8_t level);
#endif
