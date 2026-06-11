#include "octree_query.h"

#include "../darray.h"
#include <math.h>
#include <float.h>

// squared distance from a point to an aabb. 0 if inside. avoids the sqrt until
// the caller actually needs a real distance.
static float box_dist_sq(aabb b, vec3 p) {
    float dx = 0, dy = 0, dz = 0;
    if (p.x < b.min.x) dx = b.min.x - p.x; else if (p.x > b.max.x) dx = p.x - b.max.x;
    if (p.y < b.min.y) dy = b.min.y - p.y; else if (p.y > b.max.y) dy = p.y - b.max.y;
    if (p.z < b.min.z) dz = b.min.z - p.z; else if (p.z > b.max.z) dz = p.z - b.max.z;
    return dx*dx + dy*dy + dz*dz;
}

// recursive gather. `mask` 0 means no tag filter. we test the node bounds first
// so empty branches of space cost us nothing.
static int gather_aabb(const octree *t, int32_t idx, aabb region,
                       uint32_t mask, octree_item **out) {
    if (idx < 0) return 0;
    const octree_node *n = &t->pool.nodes[idx];
    if (n->count == 0) return 0;
    if (!aabb_intersects(n->bounds, region)) return 0;

    int added = 0;
    size_t len = darr_len(n->items);
    for (size_t i = 0; i < len; i++) {
        const octree_item *it = &n->items[i];
        if (mask && !(it->tag & mask)) continue;
        if (aabb_intersects(it->box, region)) {
            darr_push(*out, *it);
            added++;
        }
    }
    if (!n->leaf) {
        for (int c = 0; c < 8; c++) {
            if (n->child[c] >= 0)
                added += gather_aabb(t, n->child[c], region, mask, out);
        }
    }
    return added;
}

int octree_query_aabb(const octree *t, aabb region, octree_item **out) {
    return gather_aabb(t, t->root, region, 0, out);
}

int octree_query_tagged(const octree *t, aabb region, uint32_t tag_mask,
                        octree_item **out) {
    return gather_aabb(t, t->root, region, tag_mask, out);
}

static int gather_point(const octree *t, int32_t idx, vec3 p, octree_item **out) {
    if (idx < 0) return 0;
    const octree_node *n = &t->pool.nodes[idx];
    if (n->count == 0) return 0;
    if (!aabb_contains(n->bounds, p)) return 0;

    int added = 0;
    size_t len = darr_len(n->items);
    for (size_t i = 0; i < len; i++) {
        if (aabb_contains(n->items[i].box, p)) {
            darr_push(*out, n->items[i]);
            added++;
        }
    }
    if (!n->leaf) {
        // a point lives in exactly one child, but items straddle, so still
        // have to consider any child whose bounds contain p.
        for (int c = 0; c < 8; c++) {
            if (n->child[c] >= 0)
                added += gather_point(t, n->child[c], p, out);
        }
    }
    return added;
}

int octree_query_point(const octree *t, vec3 p, octree_item **out) {
    return gather_point(t, t->root, p, out);
}

static int gather_sphere(const octree *t, int32_t idx, vec3 c, float r2,
                         octree_item **out) {
    if (idx < 0) return 0;
    const octree_node *n = &t->pool.nodes[idx];
    if (n->count == 0) return 0;
    if (box_dist_sq(n->bounds, c) > r2) return 0;

    int added = 0;
    size_t len = darr_len(n->items);
    for (size_t i = 0; i < len; i++) {
        if (box_dist_sq(n->items[i].box, c) <= r2) {
            darr_push(*out, n->items[i]);
            added++;
        }
    }
    if (!n->leaf) {
        for (int ci = 0; ci < 8; ci++) {
            if (n->child[ci] >= 0)
                added += gather_sphere(t, n->child[ci], c, r2, out);
        }
    }
    return added;
}

int octree_query_sphere(const octree *t, vec3 center, float radius,
                        octree_item **out) {
    return gather_sphere(t, t->root, center, radius * radius, out);
}

// nearest: branch-and-bound. carry the best distance so far and skip any node
// whose bounds cant possibly beat it. order doesnt matter much for correctness
// but visiting the near child first prunes harder, so we sort the 8 children by
// their bound distance before recursing.
static void nearest_walk(const octree *t, int32_t idx, vec3 p,
                         float *best, octree_item *out) {
    if (idx < 0) return;
    const octree_node *n = &t->pool.nodes[idx];
    if (n->count == 0) return;
    if (box_dist_sq(n->bounds, p) > *best) return;

    size_t len = darr_len(n->items);
    for (size_t i = 0; i < len; i++) {
        float d = box_dist_sq(n->items[i].box, p);
        if (d < *best) { *best = d; *out = n->items[i]; }
    }
    if (n->leaf) return;

    // cheap insertion sort of child indices by bound distance. only 8 of them.
    int32_t order[8]; float od[8]; int m = 0;
    for (int c = 0; c < 8; c++) {
        if (n->child[c] < 0) continue;
        float d = box_dist_sq(t->pool.nodes[n->child[c]].bounds, p);
        int j = m++;
        order[j] = n->child[c]; od[j] = d;
        while (j > 0 && od[j-1] > od[j]) {
            float td = od[j]; od[j] = od[j-1]; od[j-1] = td;
            int32_t ti = order[j]; order[j] = order[j-1]; order[j-1] = ti;
            j--;
        }
    }
    for (int k = 0; k < m; k++) {
        if (od[k] > *best) break;   // rest are even farther, sorted, so done.
        nearest_walk(t, order[k], p, best, out);
    }
}

int octree_query_nearest(const octree *t, vec3 p, octree_item *out) {
    if (t->item_count == 0) return 0;
    float best = FLT_MAX;
    octree_item found = {0};
    nearest_walk(t, t->root, p, &best, &found);
    if (best == FLT_MAX) return 0;
    *out = found;
    return 1;
}
