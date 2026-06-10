#ifndef UTIL_OCTREE_QUERY_H
#define UTIL_OCTREE_QUERY_H

// region queries against the tree. all of these are "gather everything that
// might overlap" - they're a broadphase, so a hit here means "check this pair
// for real", not "definitely overlapping". the box test is exact though, point
// and sphere are exact too.
//
// results go into a caller-owned darray of octree_item (pass the address of a
// `octree_item *results` that's either NULL or a live darray). we append, we
// dont clear - caller decides when to reset. count returned is how many we
// added this call.

#include "octree.h"

// everything whose box overlaps `region`.
int octree_query_aabb(const octree *t, aabb region, octree_item **out);

// everything whose box contains `p`.
int octree_query_point(const octree *t, vec3 p, octree_item **out);

// everything whose box is within `radius` of `center` (sphere vs aabb).
int octree_query_sphere(const octree *t, vec3 center, float radius, octree_item **out);

// tag-filtered aabb query. only items with (item.tag & tag_mask) != 0 pass.
int octree_query_tagged(const octree *t, aabb region, uint32_t tag_mask,
                        octree_item **out);

// nearest item to `p` (by box surface distance). returns 1 and fills *out if
// the tree is non-empty, else 0. linear-ish but prunes whole subtrees by their
// bounds so it stays sane.
int octree_query_nearest(const octree *t, vec3 p, octree_item *out);

#endif
