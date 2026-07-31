#ifndef ENTITY_PATHFIND_PF_NODEPOOL_H
#define ENTITY_PATHFIND_PF_NODEPOOL_H

#include "pf_node.h"
#include "../../util/hashmap.h"

// flat bump pool of nodes plus a coord->index map so we can find the node
// for a visited cell in O(1). i went with a bump pool instead of util/pool
// because we never free individual nodes mid-search, we just reset the lot.

typedef struct {
    pf_node  nodes[PF_MAX_NODES];
    int      count;
    hashmap  index;     // pf_coord_key -> (node index + 1), 0 means absent
} pf_nodepool;

void  pf_nodepool_init(pf_nodepool *p);
void  pf_nodepool_reset(pf_nodepool *p);   // clears for reuse, keeps map cap
void  pf_nodepool_free(pf_nodepool *p);

// grab a fresh node for coord `c`. returns its index, or -1 if pool full.
// also registers it in the index map.
int   pf_nodepool_alloc(pf_nodepool *p, pf_coord c);

// look up the node index for a coord, or -1 if we haven't seen it.
int   pf_nodepool_find(const pf_nodepool *p, pf_coord c);

static inline pf_node *pf_nodepool_at(pf_nodepool *p, int idx) {
    return &p->nodes[idx];
}

#endif
