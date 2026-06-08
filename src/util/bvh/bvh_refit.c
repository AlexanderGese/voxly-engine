#include "bvh_refit.h"
#include "bvh_build.h"
#include "bvh_box.h"

#include <stddef.h>
#include <stdlib.h>

int bvh_set_prim_box(bvh *b, int32_t ordered_slot, aabb new_box) {
    if (ordered_slot < 0 || ordered_slot >= b->store.prim_count) return -1;
    b->store.prims[ordered_slot].box = new_box;
    return 0;
}

// the flat layout makes the bottom-up pass trivial: nodes were emitted
// depth-first parent-before-children, so a single reverse scan over the node
// array visits every child before its parent. no recursion, no stack.
//
// the catch: an interior node's left child is idx+1 (adjacent) but the right
// child is `u.second_child`, which is *somewhere ahead*. since we sweep in
// reverse, both children are already finalized by the time we reach the parent.
void bvh_refit(bvh *b) {
    if (!b->built || b->root < 0) return;

    bvh_storage *s = &b->store;
    for (int32_t i = s->node_count - 1; i >= 0; i--) {
        bvh_node *n = bvh_node_at(s, i);
        if (n->count > 0) {
            // leaf: union of its prims.
            aabb box = bvh_box_empty();
            uint32_t first = n->u.first_prim;
            for (uint32_t k = 0; k < n->count; k++)
                box = bvh_box_union(box, s->prims[first + k].box);
            n->bounds = box;
        } else {
            // interior: union of the two children. left is i+1, right is stored.
            const bvh_node *l = bvh_node_at_c(s, i + 1);
            const bvh_node *r = bvh_node_at_c(s, (int32_t)n->u.second_child);
            n->bounds = bvh_box_union(l->bounds, r->bounds);
        }
    }
}

float bvh_refit_quality(const bvh *b) {
    if (!b->built || b->root < 0) return 0.0f;

    // sum the surface area of all leaf bounds now vs. the build-time cost proxy.
    // as prims spread, leaf areas balloon and the ratio climbs. it's a heuristic
    // not a law, but it tracks "how much has this tree decayed" well enough to
    // gate a rebuild.
    const bvh_storage *s = &b->store;
    float leaf_area = 0.0f;
    for (int32_t i = 0; i < s->node_count; i++) {
        const bvh_node *n = bvh_node_at_c(s, i);
        if (n->count > 0)
            leaf_area += bvh_box_area(n->bounds) * (float)n->count;
    }
    if (b->last_cost <= 1e-6f) return 0.0f;
    // last_cost folded in traversal terms too; leaf_area is just the leaf side.
    // the ratio drifts above 1 once leaves have grown past their build size.
    float q = leaf_area / b->last_cost - 1.0f;
    return q < 0.0f ? 0.0f : q;
}

int bvh_refit_or_rebuild(bvh *b, float threshold) {
    bvh_refit(b);
    if (!b->built || b->root < 0) return 0;
    if (bvh_refit_quality(b) <= threshold) return 0;

    // gone too loose. rebuild from the internal prim copy. we cant hand
    // bvh_build the live buffer directly - it reallocs that same array mid-call
    // and would read freed memory - so snapshot first. small cost, rare path.
    int32_t n = b->store.prim_count;
    if (n <= 0) return 0;

    bvh_prim *snap = malloc((size_t)n * sizeof(bvh_prim));
    if (!snap) return 0;   // oom: keep the refit result, it's still correct
    for (int32_t i = 0; i < n; i++) snap[i] = b->store.prims[i];

    bvh_build(b, snap, n);
    free(snap);
    return 1;
}
