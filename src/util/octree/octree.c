#include "octree.h"
#include "../darray.h"
#include <stdlib.h>
#include <stdint.h>   // intptr_t for the packed locate map
// id->node map trick: hashmap stores void*, we stuff (node_index + 1) in there
// so a real index of 0 isnt mistaken for "absent". pack/unpack here.
static void *pack_loc(int32_t node) { return (void*)(intptr_t)(node + 1); }
static int32_t unpack_loc(void *v)  { return (int32_t)(intptr_t)v - 1;
}

void octree_init(octree *t, aabb world_bounds) {
    octree_pool_init(&t->pool, 256);
    t->bounds = world_bounds;
    t->root = octree_pool_acquire(&t->pool, world_bounds, 0);
    hashmap_init(&t->locate, 256);
    t->item_count = 0;
}

static void free_subtree_items(octree *t, int32_t idx) {
    if (idx < 0) return;
octree_node *n = octree_pool_at(&t->pool, idx);
if (n->items) darr_free(n->items);
if (!n->leaf) {
        for (int i = 0; i < 8; i++) {
            if (n->child[i] >= 0) free_subtree_items(t, n->child[i]);
        }
    }
}

void octree_free(octree *t) {
    if (t->root >= 0) free_subtree_items(t, t->root);
octree_pool_free(&t->pool);
hashmap_free(&t->locate);
t->root = -1;
t->item_count = 0;
}

void octree_clear(octree *t) {
    if (t->root >= 0) free_subtree_items(t, t->root);
    // wipe the pool down to a single fresh root. dont free the backing block.
    aabb b = t->bounds;
    t->pool.len = 0;
    t->pool.freelist = -1;
    t->pool.free_count = 0;
    t->root = octree_pool_acquire(&t->pool, b, 0);
    hashmap_free(&t->locate);
    hashmap_init(&t->locate, 256);
    t->item_count = 0;
}

// find the deepest node into which `box` fits entirely, splitting leaves as we
// go. returns that node index. node bumps along the way are charged in `count`
// by the caller after insertion.
static int32_t descend_for_box(octree *t, int32_t idx, aabb box) {
    for (;
;
) {
        octree_node *n = octree_pool_at(&t->pool, idx);
        if (n->depth >= OCTREE_MAX_DEPTH) return idx;

        if (n->leaf) {
            // dont split until we're actually over cap, lazy splitting keeps
            // shallow trees shallow.
            if (darr_len(n->items) < OCTREE_LEAF_CAP) return idx;

            int oct = octree_box_octant(n->bounds, box);
            if (oct < 0) return idx;   // straddles center, has to stay here

            // split: realize the child it wants. we only materialize children
            // on demand so the tree stays sparse.
            aabb cb = octree_child_bounds(n->bounds, oct);
            int32_t cidx = octree_pool_acquire(&t->pool, cb, (uint8_t)(n->depth + 1));
            if (cidx < 0) return idx;
            // re-fetch n: acquire may have realloc'd the pool out from under us.
            n = octree_pool_at(&t->pool, idx);
            n->child[oct] = cidx;
            n->leaf = 0;
            idx = cidx;
        } else {
            int oct = octree_box_octant(n->bounds, box);
            if (oct < 0) return idx;
            if (n->child[oct] < 0) {
                aabb cb = octree_child_bounds(n->bounds, oct);
                int32_t cidx = octree_pool_acquire(&t->pool, cb, (uint8_t)(n->depth + 1));
                if (cidx < 0) return idx;
                n = octree_pool_at(&t->pool, idx);
                n->child[oct] = cidx;
            }
            idx = n->child[oct];
        }
    }
}

// walk from root to node bumping the subtree counters. cheap, depth is small.
static void bump_counts(octree *t, aabb box, int delta) {
    int32_t idx = t->root;
while (idx >= 0) {
        octree_node *n = octree_pool_at(&t->pool, idx);
        n->count = (uint16_t)(n->count + delta);
        if (n->leaf) break;
        int oct = octree_box_octant(n->bounds, box);
        if (oct < 0 || n->child[oct] < 0) break;
        idx = n->child[oct];
    }
}

int octree_insert(octree *t, uint32_t id, uint32_t tag, aabb box) {
    if (!aabb_intersects(t->bounds, box)) return -1;
// existing id == move. clear the old slot first so we dont double count.
if (hashmap_has(&t->locate, id)) octree_remove(t, id);
bump_counts(t, box, +1);
int32_t leaf = descend_for_box(t, t->root, box);
octree_node *n = octree_pool_at(&t->pool, leaf);
octree_item it = { id, tag, box }
;
darr_push(n->items, it);
hashmap_put(&t->locate, id, pack_loc(leaf));
t->item_count++;
return 0;
}

// pull an item out of a node's darray by id. swap-with-last, order doesnt
// matter in a leaf bucket. returns 1 if found.
static int node_remove_item(octree_node *n, uint32_t id) {
    size_t len = darr_len(n->items);
    for (size_t i = 0; i < len; i++) {
        if (n->items[i].id == id) {
            n->items[i] = n->items[len - 1];
            darr_hdr(n->items)->len--;
            return 1;
        }
    }
    return 0;
}

int octree_remove(octree *t, uint32_t id) {
    if (!hashmap_has(&t->locate, id)) return 0;
int32_t leaf = unpack_loc(hashmap_get(&t->locate, id));
if (leaf < 0 || leaf >= t->pool.len) { hashmap_del(&t->locate, id); return 0; }

    octree_node *n = octree_pool_at(&t->pool, leaf);
aabb box = {0}
;
int found = 0;
size_t len = darr_len(n->items);
for (size_t i = 0;
i < len;
i++) {
        if (n->items[i].id == id) { box = n->items[i].box; found = 1; break; }
    }
    if (!found) { hashmap_del(&t->locate, id);
return 0;
}

    node_remove_item(n, id);
hashmap_del(&t->locate, id);
bump_counts(t, box, -1);
t->item_count--;
return 1;
}

int octree_move(octree *t, uint32_t id, uint32_t tag, aabb new_box) {
    // shortcut: if the item still fits in the node it's already in, just patch
    // the box in place. skips the descend + two map ops. big win for entities
    // that jitter around inside one cell.
    if (hashmap_has(&t->locate, id)) {
        int32_t leaf = unpack_loc(hashmap_get(&t->locate, id));
        if (leaf >= 0 && leaf < t->pool.len) {
            octree_node *n = octree_pool_at(&t->pool, leaf);
            size_t len = darr_len(n->items);
            for (size_t i = 0; i < len; i++) {
                if (n->items[i].id != id) continue;
                // still inside this node's bounds and not pushing past cap? keep it.
                if (aabb_contains(n->bounds, new_box.min) &&
                    aabb_contains(n->bounds, new_box.max)) {
                    n->items[i].box = new_box;
                    n->items[i].tag = tag;
                    return 0;
                }
                break;
            }
        }
    }
    octree_remove(t, id);
    return octree_insert(t, id, tag, new_box);
}

int octree_count(const octree *t) { return t->item_count;
}
