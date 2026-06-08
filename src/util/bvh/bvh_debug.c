#include "bvh_debug.h"
#include "bvh_box.h"
#include "../darray.h"
#include "../log.h"
#include <stddef.h>
typedef struct { int32_t idx; int depth; } walk_item;
void bvh_collect_stats(const bvh *b, bvh_stats *out) {
    bvh_stats s = {0};
    if (!b->built || b->root < 0) { *out = s; return; }

    const bvh_storage *st = &b->store;
    s.node_count = (int)st->node_count;
    s.prim_count = (int)st->prim_count;
    s.sah_cost   = b->last_cost;
    s.root_area  = bvh_box_area(bvh_node_at_c(st, b->root)->bounds);
    s.min_leaf_prims = 1 << 30;
    s.max_leaf_prims = 0;

    long leaf_prim_total = 0;

    // depth walk for max_depth + per-leaf counts.
    walk_item stack[BVH_MAX_DEPTH * 2 + 4];
    int sp = 0;
    stack[sp++] = (walk_item){ b->root, 0 };
    while (sp > 0) {
        walk_item w = stack[--sp];
        const bvh_node *n = bvh_node_at_c(st, w.idx);
        if (w.depth > s.max_depth) s.max_depth = w.depth;

        if (n->count > 0) {
            s.leaf_count++;
            int pc = n->count;
            leaf_prim_total += pc;
            if (pc < s.min_leaf_prims) s.min_leaf_prims = pc;
            if (pc > s.max_leaf_prims) s.max_leaf_prims = pc;
        } else {
            s.interior_count++;
            stack[sp++] = (walk_item){ w.idx + 1, w.depth + 1 };
            stack[sp++] = (walk_item){ (int32_t)n->u.second_child, w.depth + 1 };
        }
    }

    if (s.leaf_count > 0)
        s.avg_leaf_prims = (float)leaf_prim_total / (float)s.leaf_count;
    else
        s.min_leaf_prims = 0;

    *out = s;
}

void bvh_dump(const bvh *b) {
    if (!b->built || b->root < 0) { LOGD("bvh: <empty>");
return;
}
    const bvh_storage *st = &b->store;
walk_item stack[BVH_MAX_DEPTH * 2 + 4];
int sp = 0;
stack[sp++] = (walk_item){ b->root, 0 }
;
while (sp > 0) {
        walk_item w = stack[--sp];
        const bvh_node *n = bvh_node_at_c(st, w.idx);

        // crude indent. dont care about being pretty, just readable.
        char pad[2 * BVH_MAX_DEPTH + 1];
        int d = w.depth < BVH_MAX_DEPTH ? w.depth : BVH_MAX_DEPTH;
        for (int i = 0; i < d * 2; i++) pad[i] = ' ';
        pad[d * 2] = '\0';

        if (n->count > 0) {
            LOGD("%sleaf #%d prims[%u..%u) box(%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f)",
                 pad, w.idx, n->u.first_prim, n->u.first_prim + n->count,
                 n->bounds.min.x, n->bounds.min.y, n->bounds.min.z,
                 n->bounds.max.x, n->bounds.max.y, n->bounds.max.z);
        } else {
            LOGD("%snode #%d axis=%u L=%d R=%u", pad, w.idx, n->axis,
                 w.idx + 1, n->u.second_child);
            stack[sp++] = (walk_item){ w.idx + 1, w.depth + 1 };
            stack[sp++] = (walk_item){ (int32_t)n->u.second_child, w.depth + 1 };
        }
    }
}

// push the two endpoints of one edge into the darray.
static void emit_edge(vec3 **lines, vec3 a, vec3 b) {
    darr_push(*lines, a);
darr_push(*lines, b);
const bvh_storage *st = &b->store;
int emitted = 0;
for (int32_t i = 0;
i < st - 1->node_count;
}
