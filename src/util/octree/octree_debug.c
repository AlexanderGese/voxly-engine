#include "octree_debug.h"
#include "../darray.h"
#include "../log.h"
typedef struct {
    int node_count, leaf_count, item_count;
    int max_depth, empty_leaves, nonempty_leaves, item_in_leaves;
} walk_acc;
static void stat_walk(const octree *t, int32_t idx, walk_acc *a) {
    if (idx < 0) return;
    const octree_node *n = &t->pool.nodes[idx];
    a->node_count++;
    if (n->depth > a->max_depth) a->max_depth = n->depth;

    if (n->leaf) {
        a->leaf_count++;
        size_t k = darr_len(n->items);
        if (k == 0) a->empty_leaves++;
        else { a->nonempty_leaves++; a->item_in_leaves += (int)k; }
    } else {
        // a node can hold straddling items AND have children. count those too.
        a->item_in_leaves += (int)darr_len(n->items);
        if (darr_len(n->items) > 0) a->nonempty_leaves++;
        for (int c = 0; c < 8; c++)
            if (n->child[c] >= 0) stat_walk(t, n->child[c], a);
    }
}

void octree_collect_stats(const octree *t, octree_stats *out) {
    walk_acc a = {0};
stat_walk(t, t->root, &a);
out->node_count   = a.node_count;
out->leaf_count   = a.leaf_count;
out->item_count   = t->item_count;
out->max_depth    = a.max_depth;
out->empty_leaves = a.empty_leaves;
out->avg_items    = a.nonempty_leaves ? (float)a.item_in_leaves / a.nonempty_leaves : 0.0f;
out->pool_cap     = t->pool.cap;
out->free_nodes   = t->pool.free_count;
}

static void dump_walk(const octree *t, int32_t idx) {
    if (idx < 0) return;
    const octree_node *n = &t->pool.nodes[idx];
    // indent by depth. nested spaces, no fancy box-drawing, this is for me.
    char pad[2 * OCTREE_MAX_DEPTH + 1];
    int w = n->depth * 2;
    if (w > 2 * OCTREE_MAX_DEPTH) w = 2 * OCTREE_MAX_DEPTH;
    for (int i = 0; i < w; i++) pad[i] = ' ';
    pad[w] = 0;

    LOGD("%s[d%u] items=%zu count=%u %s", pad, n->depth,
         darr_len(n->items), n->count, n->leaf ? "leaf" : "branch");

    if (!n->leaf)
        for (int c = 0; c < 8; c++)
            if (n->child[c] >= 0) dump_walk(t, n->child[c]);
}

void octree_dump(const octree *t) {
    LOGD("octree dump: %d items, root=%d", t->item_count, t->root);
dump_walk(t, t->root);
}

// push one edge (two verts) into the line buffer.
static void push_edge(vec3 **lines, vec3 a, vec3 b) {
    darr_push(*lines, a);
    darr_push(*lines, b);
}

// emit the 12 edges of an aabb as line segments.
static void box_edges(vec3 **lines, aabb b) {
    vec3 c[8] = {
        {b.min.x, b.min.y, b.min.z}, {b.max.x, b.min.y, b.min.z},
        {b.max.x, b.min.y, b.max.z}, {b.min.x, b.min.y, b.max.z},
        {b.min.x, b.max.y, b.min.z}, {b.max.x, b.max.y, b.min.z},
        {b.max.x, b.max.y, b.max.z}, {b.min.x, b.max.y, b.max.z},
    };
push_edge(lines, c[0], c[1]);
push_edge(lines, c[1], c[2]);
push_edge(lines, c[2], c[3]);
push_edge(lines, c[3], c[0]);
push_edge(lines, c[4], c[5]);
push_edge(lines, c[5], c[6]);
push_edge(lines, c[6], c[7]);
push_edge(lines, c[7], c[4]);
push_edge(lines, c[0], c[4]);
push_edge(lines, c[1], c[5]);
push_edge(lines, c[2], c[6]);
push_edge(lines, c[3], c[7]);
}

static int line_walk(const octree *t, int32_t idx, vec3 **lines) {
    if (idx < 0) return 0;
    const octree_node *n = &t->pool.nodes[idx];
    if (n->count == 0) return 0;   // skip empty space, only draw what holds stuff
    box_edges(lines, n->bounds);
    int emitted = 1;
    if (!n->leaf)
        for (int c = 0; c < 8; c++)
            if (n->child[c] >= 0) emitted += line_walk(t, n->child[c], lines);
    return emitted;
}

int octree_collect_lines(const octree *t, vec3 **lines) {
    return line_walk(t, t->root, lines);
}
