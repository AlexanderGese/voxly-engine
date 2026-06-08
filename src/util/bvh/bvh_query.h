#ifndef UTIL_BVH_QUERY_H
#define UTIL_BVH_QUERY_H

// box / point / sphere overlap queries against the tree. these are the
// broadphase workhorses: "what's near here?". results come back as prim ids via
// a callback so the caller decides what to do with each hit without us
// allocating a result array.

#include "bvh.h"

// callback per overlapping prim. return 0 to keep going, non-zero to stop the
// whole traversal early (e.g. you only wanted to know "is there anything?").
// `user` is passed straight through.
typedef int (*bvh_visit_fn)(uint32_t id, uint32_t tag, aabb box, void *user);

// visit every prim whose box overlaps `query`. tag_mask of 0 means "any tag",
// otherwise a prim is skipped unless (prim.tag & tag_mask) is non-zero. returns
// the number of prims visited (before any early-out).
int bvh_query_aabb(const bvh *b, aabb query, uint32_t tag_mask,
                   bvh_visit_fn fn, void *user);

// visit every prim whose box contains `point`. same tag/callback contract.
int bvh_query_point(const bvh *b, vec3 point, uint32_t tag_mask,
                    bvh_visit_fn fn, void *user);

// visit every prim whose box overlaps the sphere (center, radius). uses the
// cheap aabb-vs-sphere closest-point test, so a prim grazing the corner counts.
int bvh_query_sphere(const bvh *b, vec3 center, float radius, uint32_t tag_mask,
                     bvh_visit_fn fn, void *user);

// quick boolean: does anything (matching tag_mask) overlap `query`? short-
// circuits on the first hit. handy for collision "can i place here" checks.
int bvh_query_any(const bvh *b, aabb query, uint32_t tag_mask);

// nearest-primitive search. finds the prim whose box is closest to `point`
// (distance 0 if point is inside a box) within max_dist. distance-pruned
// best-first-ish walk: subtrees whose bounds are already farther than the
// current best are skipped wholesale. writes the winner id/tag/box into the
// out params (any may be NULL) and returns the distance, or a negative value if
// nothing within max_dist matched.
float bvh_query_closest(const bvh *b, vec3 point, float max_dist,
                        uint32_t tag_mask,
                        uint32_t *out_id, uint32_t *out_tag, aabb *out_box);

// squared distance from a point to the nearest surface of a box (0 if inside).
// exposed because the closest search leans on it and callers occasionally want
// the same metric for their own ranking.
float bvh_point_box_dist2(vec3 p, aabb box);

#endif
