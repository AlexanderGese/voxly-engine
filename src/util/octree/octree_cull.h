#ifndef UTIL_OCTREE_CULL_H
#define UTIL_OCTREE_CULL_H

// frustum visibility + pair broadphase. these grew out of the basic queries
// once entity_render wanted "only the mobs the camera can see" and the physics
// pass wanted "every pair of entities close enough to maybe collide". both are
// just tree walks with a different acceptance test, so they live together.

#include "octree.h"
#include "../../math/frustum.h"

// gather every item whose box is at least partially inside the frustum. prunes
// whole subtrees whose bounds are fully outside, so for a tight frustum this
// touches very few nodes. appends to *out (darray), returns count added.
int octree_query_frustum(const octree *t, const frustum *f, octree_item **out);

// a candidate collision pair. ids only, caller resolves to whatever it stores.
typedef struct {
    uint32_t a;
    uint32_t b;
} octree_pair;

// find all pairs of items whose boxes overlap, expanded by `margin` on each
// side (so "about to touch" counts). appends to *out (darray of octree_pair).
// returns pair count. this is the broadphase the physics step consumes - it
// never reports (a,a) and reports each unordered pair once.
int octree_collect_pairs(const octree *t, float margin, octree_pair **out);

#endif
