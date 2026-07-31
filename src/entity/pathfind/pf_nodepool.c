#include "pf_nodepool.h"
#include <stdint.h>

void pf_nodepool_init(pf_nodepool *p) {
    p->count = 0;
    // sized so we rarely rehash mid-search. 0.75 load over the pool.
    hashmap_init(&p->index, PF_MAX_NODES * 2);
}

void pf_nodepool_reset(pf_nodepool *p) {
    p->count = 0;
    // the map has no clear-without-free, so drop and re-init. fast enough,
    // a planner runs maybe a few times a second per mob.
    hashmap_free(&p->index);
    hashmap_init(&p->index, PF_MAX_NODES * 2);
}

void pf_nodepool_free(pf_nodepool *p) {
    hashmap_free(&p->index);
    p->count = 0;
}

int pf_nodepool_alloc(pf_nodepool *p, pf_coord c) {
    if (p->count >= PF_MAX_NODES) return -1;
    int idx = p->count++;
    pf_node *n = &p->nodes[idx];
    n->c        = c;
    n->g        = 0;
    n->f        = 0;
    n->parent   = -1;
    n->heap_pos = -1;
    n->closed   = 0;
    // store idx+1 so the "absent" sentinel (0/NULL) stays distinct from
    // node 0. hashmap stores void*, so we smuggle the int through.
    hashmap_put(&p->index, pf_coord_key(c), (void*)(intptr_t)(idx + 1));
    return idx;
}

int pf_nodepool_find(const pf_nodepool *p, pf_coord c) {
    void *v = hashmap_get(&p->index, pf_coord_key(c));
    if (!v) return -1;
    return (int)(intptr_t)v - 1;
}
