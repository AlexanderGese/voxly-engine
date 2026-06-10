#include "octree_cull.h"

#include "../darray.h"

// frustum vs node bounds with a tri-state would be nicer (fully-in lets us skip
// the per-item test) but frustum.h only gives a boolean. so we test the node to
// prune, then test each item for real. still cheap.
static int frustum_walk(const octree *t, int32_t idx, const frustum *f,
                        octree_item **out) {
    if (idx < 0) return 0;
    const octree_node *n = &t->pool.nodes[idx];
    if (n->count == 0) return 0;
    if (!frustum_contains_aabb(f, n->bounds)) return 0;

    int added = 0;
    size_t len = darr_len(n->items);
    for (size_t i = 0; i < len; i++) {
        if (frustum_contains_aabb(f, n->items[i].box)) {
            darr_push(*out, n->items[i]);
            added++;
        }
    }
    if (!n->leaf)
        for (int c = 0; c < 8; c++)
            if (n->child[c] >= 0)
                added += frustum_walk(t, n->child[c], f, out);
    return added;
}

int octree_query_frustum(const octree *t, const frustum *f, octree_item **out) {
    return frustum_walk(t, t->root, f, out);
}

static aabb expand_box(aabb b, float m) {
    b.min.x -= m; b.min.y -= m; b.min.z -= m;
    b.max.x += m; b.max.y += m; b.max.z += m;
    return b;
}

// pair finding. the trick with octrees: two boxes can only overlap if they
// share a node somewhere on their root-to-leaf paths. so at each node we pair
// its own items against (a) each other and (b) every item in the subtree below.
// we pass an "ancestors" stack down so a child's items get checked against all
// the straddling items sitting in nodes above it. this is the standard octree
// self-collision sweep.
typedef struct {
    octree_item **stack;   // darray of ancestor items currently in scope
    float         margin;
    octree_pair **out;
    int           count;
} pair_ctx;

static void try_pair(pair_ctx *pc, const octree_item *a, const octree_item *b) {
    if (a->id == b->id) return;
    aabb ea = expand_box(a->box, pc->margin);
    if (!aabb_intersects(ea, b->box)) return;
    octree_pair p = { a->id, b->id };
    darr_push(*pc->out, p);
    pc->count++;
}

static void pair_walk(const octree *t, int32_t idx, pair_ctx *pc) {
    if (idx < 0) return;
    const octree_node *n = &t->pool.nodes[idx];
    if (n->count == 0) return;

    size_t len = darr_len(n->items);
    size_t anc = darr_len(*pc->stack);

    // this node's items against every ancestor item still in scope
    for (size_t i = 0; i < len; i++)
        for (size_t a = 0; a < anc; a++)
            try_pair(pc, &(*pc->stack)[a], &n->items[i]);

    // this node's items against each other (unordered, j>i)
    for (size_t i = 0; i < len; i++)
        for (size_t j = i + 1; j < len; j++)
            try_pair(pc, &n->items[i], &n->items[j]);

    if (n->leaf) return;

    // push our items as ancestors, recurse, pop. darray-as-stack.
    for (size_t i = 0; i < len; i++)
        darr_push(*pc->stack, n->items[i]);

    for (int c = 0; c < 8; c++)
        if (n->child[c] >= 0) pair_walk(t, n->child[c], pc);

    // pop back to the depth we were at. straight truncate, order preserved.
    darr_hdr(*pc->stack)->len = anc;
}

int octree_collect_pairs(const octree *t, float margin, octree_pair **out) {
    octree_item *stack = NULL;
    pair_ctx pc = { &stack, margin, out, 0 };
    pair_walk(t, t->root, &pc);
    darr_free(stack);
    return pc.count;
}
