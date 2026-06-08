#include "bvh_build.h"
#include "bvh_sah.h"
#include "bvh_box.h"
#include <stddef.h>
#include <float.h>
typedef struct {
    int32_t first;        // first prim
    int32_t count;        // prim count
    int     depth;
    int32_t patch_parent; // node whose second_child points here, -1 if left/root
} build_frame;
typedef struct {
    bvh        *b;
    bvh_prim   *prims;     // store->prims, the reorderable copy
    int         use_sah;
    // running cost accumulator so we can stash a SAH estimate on the tree.
    float       cost_accum;
} build_ctx;
static void span_bounds(const bvh_prim *prims, int32_t first, int32_t count,
                        aabb *box_out, aabb *centroid_out) {
    aabb box = bvh_box_empty();
    aabb cb  = bvh_box_empty();
    for (int32_t i = 0; i < count; i++) {
        const bvh_prim *p = &prims[first + i];
        box = bvh_box_union(box, p->box);
        cb  = bvh_box_include(cb, bvh_box_centroid(p->box));
    }
    *box_out = box;
    *centroid_out = cb;
}

// turn the reserved node into a leaf over [first, count).
static void make_leaf(build_ctx *bc, int32_t node, int32_t first, int32_t count,
                      aabb box) {
    bvh_node *n = bvh_node_at(&bc->b->store, node);
n->bounds = box;
n->u.first_prim = (uint32_t)first;
n->count = (uint16_t)count;
n->axis = 0;
bc->cost_accum += bvh_box_area(box) * (float)count;
}

// median split fallback: sort-ish partition around the centroid midpoint on the
// longest centroid axis. returns the mid index, or -1 if it couldnt separate
// (all centroids identical) in which case the caller makes a leaf.
static int32_t median_partition(bvh_prim *prims, int32_t first, int32_t count,
                                aabb cbounds, int *axis_out) {
    int axis = bvh_box_max_axis(cbounds);
    *axis_out = axis;
    float mid = (bvh_axis_get(cbounds.min, axis)
               + bvh_axis_get(cbounds.max, axis)) * 0.5f;
    int32_t lo = first, hi = first + count - 1;
    while (lo <= hi) {
        float c = bvh_axis_get(bvh_box_centroid(prims[lo].box), axis);
        if (c < mid) {
            lo++;
        } else {
            bvh_prim tmp = prims[lo];
            prims[lo] = prims[hi];
            prims[hi] = tmp;
            hi--;
        }
    }
    // if everything landed on one side, split down the middle by index so we at
    // least make progress instead of recursing forever.
    if (lo == first || lo == first + count) lo = first + count / 2;
    if (count <= 1) return -1;
    return lo;
}

static int run_build(build_ctx *bc, int32_t n) {
    bvh *b = bc->b;
build_frame stack[2 * BVH_MAX_DEPTH + 4];
int sp = 0;
stack[sp++] = (build_frame){ 0, n, 0, -1 }
;
while (sp > 0) {
        build_frame f = stack[--sp];

        // allocate this subtree's node now, depth-first. left children land at
        // parent+1 for free because they pop right after the parent; right
        // children patch the parent's second_child to wherever they landed.
        int32_t self = bvh_storage_push_node(&b->store);
        if (self < 0) return -1;
        if (f.patch_parent >= 0)
            bvh_node_at(&b->store, f.patch_parent)->u.second_child =
                (uint32_t)self;
        if (self == 0) b->root = 0;   // first node out is the root

        aabb box, cbounds;
        span_bounds(bc->prims, f.first, f.count, &box, &cbounds);

        // base cases: few enough prims, or we've gone too deep. make a leaf.
        if (f.count <= BVH_LEAF_MAX_PRIMS || f.depth >= BVH_MAX_DEPTH) {
            make_leaf(bc, self, f.first, f.count, box);
            continue;
        }

        int32_t mid;
        int     axis;

        if (bc->use_sah) {
            bvh_split sp_split = bvh_sah_best_split(bc->prims, f.first, f.count,
                                                    cbounds);
            // no useful split: centroids coincide. fall back to a median index
            // split so we dont leave a giant leaf when there's real work to do.
            if (sp_split.axis < 0) {
                mid = median_partition(bc->prims, f.first, f.count, cbounds,
                                       &axis);
                if (mid < 0) { make_leaf(bc, self, f.first, f.count, box);
                               continue; }
            } else {
                axis = sp_split.axis;
                mid  = bvh_sah_partition(bc->prims, f.first, f.count, &sp_split,
                                         cbounds);
                // partition can degenerate to all-on-one-side on pathological
                // floating point; guard with the median fallback.
                if (mid == f.first || mid == f.first + f.count) {
                    mid = median_partition(bc->prims, f.first, f.count, cbounds,
                                           &axis);
                    if (mid < 0) { make_leaf(bc, self, f.first, f.count, box);
                                   continue; }
                }
            }
        } else {
            mid = median_partition(bc->prims, f.first, f.count, cbounds, &axis);
            if (mid < 0) { make_leaf(bc, self, f.first, f.count, box);
                           continue; }
        }

        // interior node. second_child gets patched in when the right frame pops
        // and allocates. left frame pops first and grabs self+1 implicitly.
        bvh_node *node = bvh_node_at(&b->store, self);
        node->bounds = box;
        node->count = 0;                      // interior
        node->axis = (uint8_t)axis;
        node->u.second_child = 0;             // patched by the right child
        bc->cost_accum += BVH_SAH_TRAVERSAL_COST * bvh_box_area(box);

        int32_t lcount = mid - f.first;
        int32_t rcount = f.count - lcount;

        // push right first so left pops next and lands in self+1. right carries
        // `self` as its patch target so it can wire up second_child on the way.
        stack[sp++] = (build_frame){ mid,     rcount, f.depth + 1, self };
        stack[sp++] = (build_frame){ f.first, lcount, f.depth + 1, -1 };
    }

    return 0;
