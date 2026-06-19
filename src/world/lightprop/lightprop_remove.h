#ifndef WORLD_LIGHTPROP_REMOVE_H
#define WORLD_LIGHTPROP_REMOVE_H

#include "lightprop_types.h"
#include "lightprop_queue.h"

// the removal side. when a light source disappears (torch broken) or a cell goes
// opaque (block placed), we can't just flood from neighbours because the world is
// full of now-stale-too-bright values that flood() would never overwrite.
//
// classic two-queue trick:
// 1. seed `rq` with the cell(s) whose light is being removed, carrying the OLD
// level. walk outward: any neighbour dimmer than us was lit BY us -> zero it
// and enqueue it for further removal. any neighbour as bright or brighter is
// lit by some OTHER source -> it becomes a re-light seed (pushed into `aq`).
// 2. caller then runs lp_flood on `aq` to repair the hole from surviving
// sources.
//
// `aq` must be primed/empty by the caller; this only adds to it.
void lp_unflood(world *w, lp_channel ch, lp_queue *rq, lp_queue *aq);

// seed a removal: remember the old level, zero the cell, enqueue it. pass the
// level you want treated as "removed" (usually the cell's current light).
void lp_seed_removal(world *w, lp_channel ch, lp_queue *rq,
                     int wx, int wy, int wz, uint8_t old_level);

// fast path for sky: seed the whole vertical free-light column from (wx,wy,wz)
// downward for removal in one sweep instead of letting the bfs crawl it cell by
// cell. sky channel only.
void lp_seed_sky_column_removal(world *w, lp_queue *rq, int wx, int wy, int wz);

#endif
