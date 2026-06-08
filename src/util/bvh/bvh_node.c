#include "bvh_node.h"

#include <stdlib.h>
#include <string.h>

void bvh_storage_init(bvh_storage *s, int32_t prim_hint) {
    if (prim_hint < 1) prim_hint = 1;
    // a full binary tree over N leaves with at most LEAF_MAX prims each has at
    // most 2*ceil(N/1)-1 nodes in the pathological 1-prim-per-leaf case. size
    // for that so push_node never reallocs and dangles a pointer.
    int32_t worst_nodes = prim_hint * 2 + 1;

    s->nodes      = malloc((size_t)worst_nodes * sizeof(bvh_node));
    s->node_cap   = s->nodes ? worst_nodes : 0;
    s->node_count = 0;

    s->prims      = malloc((size_t)prim_hint * sizeof(bvh_prim));
    s->prim_count = 0;
    // note: prim cap tracked implicitly via the hint, set_prims reallocs if a
    // later (bigger) build comes through.
    if (!s->prims) { s->prim_count = 0; }
}

void bvh_storage_free(bvh_storage *s) {
    free(s->nodes);
    free(s->prims);
    s->nodes = NULL;
    s->prims = NULL;
    s->node_cap = s->node_count = 0;
    s->prim_count = 0;
}

void bvh_storage_clear(bvh_storage *s) {
    s->node_count = 0;
    s->prim_count = 0;
}

int32_t bvh_storage_push_node(bvh_storage *s) {
    if (s->node_count >= s->node_cap) {
        int32_t newcap = s->node_cap ? s->node_cap * 2 : 16;
        bvh_node *nn = realloc(s->nodes, (size_t)newcap * sizeof(bvh_node));
        if (!nn) return -1;   // oom. caller bails the build.
        s->nodes = nn;
        s->node_cap = newcap;
    }
    int32_t idx = s->node_count++;
    memset(&s->nodes[idx], 0, sizeof(bvh_node));
    return idx;
}

int bvh_storage_set_prims(bvh_storage *s, const bvh_prim *src, int32_t n) {
    if (n < 0) return -1;
    // grow the prim block if this build is bigger than the last. we never shrink
    // - keeping the high-water buffer avoids churn on a yo-yoing entity count.
    bvh_prim *np = realloc(s->prims, (size_t)(n ? n : 1) * sizeof(bvh_prim));
    if (!np) return -1;
    s->prims = np;
    if (n > 0 && src) memcpy(s->prims, src, (size_t)n * sizeof(bvh_prim));
    s->prim_count = n;
    return 0;
}
