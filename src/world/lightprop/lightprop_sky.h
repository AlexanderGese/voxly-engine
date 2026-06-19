#ifndef WORLD_LIGHTPROP_SKY_H
#define WORLD_LIGHTPROP_SKY_H

#include "lightprop_types.h"
#include "lightprop_queue.h"

// sky-light seeding. the sky pours MAX_LIGHT straight down each column until it
// hits something opaque; everything above that is full daylight. we set those
// cells directly (the column drop is free) and only enqueue the cells that have
// a non-sky-lit neighbour, i.e. the ones that actually need to flood sideways.
// queuing the entire transparent column would work but wastes a huge amount of
// queue on cells that have nothing to spread to.

// seed every column in chunk `c`, writing the vertical sky shafts and priming
// `q` with the boundary cells. clears existing sky light in the chunk first.
void lp_sky_seed_chunk(world *w, chunk *c, lp_queue *q);

// seed a single world column (used on block edits). re-derives the opaque height
// at (wx,wz) and pushes the affected cells. returns the y of the highest opaque
// block, or -1 if the column is fully open.
int  lp_sky_seed_column(world *w, lp_queue *q, int wx, int wz);

// height of the topmost opaque block in a column, local coords. -1 if none.
int  lp_sky_opaque_top(chunk *c, int lx, int lz);

#endif
