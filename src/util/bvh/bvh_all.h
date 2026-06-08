#ifndef UTIL_BVH_ALL_H
#define UTIL_BVH_ALL_H

// umbrella include so callers grab the whole subsystem with one line instead of
// remembering which of the ten headers holds the function they want. it builds
// nothing extra, it's just the headers.
//
// #include "util/bvh/bvh_all.h"
// bvh b; bvh_init(&b);
// bvh_build(&b, prims, n);
// bvh_hit h = bvh_raycast(&b, eye, fwd, PLAYER_REACH);
// if (bvh_refit_quality(&b) > 0.3f) bvh_build(&b, prims, n);  // got loose
// bvh_free(&b);
//
// the octree (util/octree) is the sibling structure: reach for it when you have
// a moving set you insert/remove into incrementally. reach for the bvh when you
// have a batch you rebuild or refit and then ray a lot. different tradeoffs,
// same spirit.

#include "bvh_types.h"
#include "bvh_box.h"
#include "bvh_node.h"
#include "bvh.h"
#include "bvh_sah.h"
#include "bvh_build.h"
#include "bvh_refit.h"
#include "bvh_query.h"
#include "bvh_raycast.h"
#include "bvh_region.h"
#include "bvh_debug.h"

#endif
