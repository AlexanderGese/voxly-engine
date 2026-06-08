#ifndef UTIL_BVH_REFIT_H
#define UTIL_BVH_REFIT_H

// refit: recompute every node's bounds from the (possibly moved) primitives
// without changing the tree topology. way cheaper than a rebuild - one bottom-up
// pass, O(N) - but the tree gets looser the further prims drift from where they
// were when the SAH chose the splits. so: refit while things are calm, rebuild
// when the quality metric says it's gone stale.

#include "bvh.h"

// update a single prim's box in place (by slot index in the ordered array) and
// leave the tree topology alone. you'd typically batch several of these then
// call bvh_refit once. returns 0 ok, -1 if slot is out of range.
//
// note: this takes the *ordered* slot index, which the build reshuffled. if you
// only know the prim id, look it up via bvh_query's id helpers or just rebuild.
int   bvh_set_prim_box(bvh *b, int32_t ordered_slot, aabb new_box);

// recompute all node bounds bottom-up from the current prim boxes. leaves take
// the union of their prims, interiors the union of their children. cheap.
void  bvh_refit(bvh *b);

// a 0..1-ish looseness score: refit bounds surface area over the last build's
// SAH cost proxy. climbs as prims drift. when it crosses your threshold (0.3ish
// is a reasonable default) it's time to rebuild. returns 0 for an unbuilt tree.
float bvh_refit_quality(const bvh *b);

// refit, then check the looseness score; if it's past `threshold` do a full SAH
// rebuild from the tree's current (internal, already-reordered) primitive copy.
// returns 1 if it rebuilt, 0 if a refit was enough. the do-the-right-thing knob
// for a per-frame moving set: call this and stop thinking about it.
int   bvh_refit_or_rebuild(bvh *b, float threshold);

#endif
