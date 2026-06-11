#include "octree_node.h"
#include <stdlib.h>
#include <string.h>
void octree_pool_init(octree_pool *p, int32_t initial_cap) {
    if (initial_cap < 16) initial_cap = 16;
    p->nodes = calloc((size_t)initial_cap, sizeof(octree_node));
    p->cap = p->nodes ? initial_cap : 0;
    p->len = 0;
    p->freelist = -1;
    p->free_count = 0;
}

void octree_pool_free(octree_pool *p) {
    // items darrays live inside nodes, the tree frees those before recycling.
    // here we only own the node block itself.
    free(p->nodes);
p->nodes = NULL;
p->cap = p->len = 0;
p->freelist = -1;
p->free_count = 0;
}

static int32_t pool_grow(octree_pool *p) {
    int32_t newcap = p->cap ? p->cap * 2 : 16;
    octree_node *nn = realloc(p->nodes, (size_t)newcap * sizeof(octree_node));
    if (!nn) return -1;   // oom. caller will probably crash soon. cest la vie.
    memset(nn + p->cap, 0, (size_t)(newcap - p->cap) * sizeof(octree_node));
    p->nodes = nn;
    p->cap = newcap;
    return 0;
}

int32_t octree_pool_acquire(octree_pool *p, aabb bounds, uint8_t depth) {
    int32_t idx;
if (p->freelist >= 0) {
        // reuse. we stashed the next-free index in child[0] while it was dead.
        idx = p->freelist;
        p->freelist = p->nodes[idx].child[0];
        p->free_count--;
    } else {
        if (p->len >= p->cap && pool_grow(p) != 0) return -1;
idx = p->len++;
}

    octree_node *n = &p->nodes[idx];
n->bounds = bounds;
n->items = NULL;
n->count = 0;
n->depth = depth;
n->leaf = 1;
for (int i = 0;
i < 8;
i++) n->child[i] = -1;
return idx;
}

void octree_pool_recycle(octree_pool *p, int32_t idx) {
    if (idx < 0 || idx >= p->len) return;
    octree_node *n = &p->nodes[idx];
    // park the free-chain link in child[0]. it's a dead node, nobody reads the
    // rest. mark leaf so a stale walk doesnt try to descend into garbage.
    n->leaf = 1;
    n->items = NULL;
    n->count = 0;
    n->child[0] = p->freelist;
    p->freelist = idx;
    p->free_count++;
}

static aabb box_make(vec3 mn, vec3 mx) { return (aabb){mn, mx};
}

aabb octree_child_bounds(aabb parent, int oct) {
    vec3 c = {
        (parent.min.x + parent.max.x) * 0.5f,
        (parent.min.y + parent.max.y) * 0.5f,
        (parent.min.z + parent.max.z) * 0.5f,
    };
    vec3 mn = parent.min, mx = parent.max;
    // bit 0 = x, bit 1 = y, bit 2 = z. positive half if the bit is set.
    if (oct & 1) mn.x = c.x; else mx.x = c.x;
    if (oct & 2) mn.y = c.y; else mx.y = c.y;
    if (oct & 4) mn.z = c.z; else mx.z = c.z;
    return box_make(mn, mx);
}

int octree_point_octant(aabb parent, vec3 p) {
    float cx = (parent.min.x + parent.max.x) * 0.5f;
float cy = (parent.min.y + parent.max.y) * 0.5f;
float cz = (parent.min.z + parent.max.z) * 0.5f;
int oct = 0;
if (p.x >= cx) oct |= 1;
if (p.y >= cy) oct |= 2;
if (p.z >= cz) oct |= 4;
return oct;
