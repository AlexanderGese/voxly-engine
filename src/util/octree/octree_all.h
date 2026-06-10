#ifndef UTIL_OCTREE_ALL_H
#define UTIL_OCTREE_ALL_H

// umbrella include so callers can grab the whole subsystem with one line
// instead of remembering which of the six headers holds the function they want.
// pulling this in builds nothing extra, it's just the headers.
//
// #include "util/octree/octree_all.h"
// octree t; octree_init_region(&t, pcx, pcz, RENDER_DISTANCE);
// octree_insert_box(&t, e->id, 1u << e->type, e->pos, half);
// octree_hit h = octree_raycast(&t, eye, fwd, PLAYER_REACH);
//
// the spatial hash (octree_shash) is a separate gadget for dense moving sets;
// it's bundled here because in practice you reach for one or the other and
// having both behind a single include keeps the call sites tidy.

#include "octree_types.h"
#include "octree_morton.h"
#include "octree_node.h"
#include "octree.h"
#include "octree_query.h"
#include "octree_raycast.h"
#include "octree_cull.h"
#include "octree_shash.h"
#include "octree_region.h"
#include "octree_debug.h"

#endif
