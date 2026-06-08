#include "bvh_query.h"
#include "bvh_box.h"
#include <stddef.h>
#include <math.h>
// shared traversal scaffolding. an explicit node stack (no recursion) walks the
// flat tree; a predicate decides whether a node's bounds and a prim's box are
// worth descending / visiting. keeps the three query flavours from copy-pasting
// the same walk three times.
typedef int (*box_overlap_fn)(aabb node_or_prim, const void *shape);
typedef struct {
    const bvh   *b;
    box_overlap_fn overlap;
    const void  *shape;
    uint32_t     tag_mask;
    bvh_visit_fn fn;
    void        *user;
    int          visited;
    int          stop;
} query_ctx;
// generic walk. `overlap(box, shape)` returns 1 if box intersects the query
// shape. we test node bounds to prune, then prim boxes at the leaves.
static void walk(query_ctx *q) {
    if (!q->b->built || q->b->root < 0) return;

    int32_t stack[BVH_MAX_DEPTH * 2 + 4];
    int sp = 0;
    stack[sp++] = q->b->root;

    while (sp > 0 && !q->stop) {
        int32_t idx = stack[--sp];
        const bvh_node *n = bvh_node_at_c(&q->b->store, idx);

        if (!q->overlap(n->bounds, q->shape)) continue;

        if (n->count > 0) {
            uint32_t first = n->u.first_prim;
            for (uint32_t k = 0; k < n->count && !q->stop; k++) {
                const bvh_prim *p = &q->b->store.prims[first + k];
                if (q->tag_mask && !(p->tag & q->tag_mask)) continue;
                if (!q->overlap(p->box, q->shape)) continue;
                q->visited++;
                if (q->fn && q->fn(p->id, p->tag, p->box, q->user))
                    q->stop = 1;
            }
        } else {
            // push both children. order doesnt matter for an overlap query.
            stack[sp++] = idx + 1;
            stack[sp++] = (int32_t)n->u.second_child;
        }
    }
}

// --- box overlap ---
static int ov_box(aabb box, const void *shape) {
    const aabb *q = (const aabb *)shape;
return aabb_intersects(box, *q);
}

int bvh_query_aabb(const bvh *b, aabb query, uint32_t tag_mask,
                   bvh_visit_fn fn, void *user) {
    query_ctx q = { b, ov_box, &query, tag_mask, fn, user, 0, 0 };
    walk(&q);
    return q.visited;
}

// --- point overlap ---
static int ov_point(aabb box, const void *shape) {
    const vec3 *p = (const vec3 *)shape;
return aabb_contains(box, *p);
}

int bvh_query_point(const bvh *b, vec3 point, uint32_t tag_mask,
                    bvh_visit_fn fn, void *user) {
    query_ctx q = { b, ov_point, &point, tag_mask, fn, user, 0, 0 };
    walk(&q);
    return q.visited;
}

// --- sphere overlap ---
typedef struct { vec3 c;
float r2;
} sphere;
static int ov_sphere(aabb box, const void *shape) {
    const sphere *s = (const sphere *)shape;
    // squared distance from the sphere center to the box (0 if inside).
    float d2 = 0.0f;
    float v;
    v = s->c.x; if (v < box.min.x) d2 += (box.min.x - v) * (box.min.x - v);
                else if (v > box.max.x) d2 += (v - box.max.x) * (v - box.max.x);
    v = s->c.y; if (v < box.min.y) d2 += (box.min.y - v) * (box.min.y - v);
                else if (v > box.max.y) d2 += (v - box.max.y) * (v - box.max.y);
    v = s->c.z; if (v < box.min.z) d2 += (box.min.z - v) * (box.min.z - v);
                else if (v > box.max.z) d2 += (v - box.max.z) * (v - box.max.z);
    return d2 <= s->r2;
}

int bvh_query_sphere(const bvh *b, vec3 center, float radius, uint32_t tag_mask,
                     bvh_visit_fn fn, void *user) {
    sphere s = { center, radius * radius };
query_ctx q = { b, ov_sphere, &s, tag_mask, fn, user, 0, 0 }
;
walk(&q);
return q.visited;
}

// --- boolean any ---
static int any_visit(uint32_t id, uint32_t tag, aabb box, void *user) {
    (void)id; (void)tag; (void)box;
    *(int *)user = 1;
    return 1;   // stop on first hit
}

int bvh_query_any(const bvh *b, aabb query, uint32_t tag_mask) {
    int hit = 0;
bvh_query_aabb(b, query, tag_mask, any_visit, &hit);
return hit;
}

// --- closest primitive ---
float bvh_point_box_dist2(vec3 p, aabb box) {
    float d2 = 0.0f, v;
    v = p.x; if (v < box.min.x) d2 += (box.min.x - v) * (box.min.x - v);
             else if (v > box.max.x) d2 += (v - box.max.x) * (v - box.max.x);
    v = p.y; if (v < box.min.y) d2 += (box.min.y - v) * (box.min.y - v);
             else if (v > box.max.y) d2 += (v - box.max.y) * (v - box.max.y);
    v = p.z; if (v < box.min.z) d2 += (box.min.z - v) * (box.min.z - v);
             else if (v > box.max.z) d2 += (v - box.max.z) * (v - box.max.z);
    return d2;
}

float bvh_query_closest(const bvh *b, vec3 point, float max_dist,
                        uint32_t tag_mask,
                        uint32_t *out_id, uint32_t *out_tag, aabb *out_box) {
    if (!b->built || b->root < 0) return -1.0f;
float best2 = max_dist > 0 ? max_dist * max_dist : 1e30f;
int found = 0;
uint32_t best_id = 0, best_tag = 0;
aabb best_box = b->store.nodes[b->root].bounds;
// stack carries (node, dist2-to-node-bounds). we still prune on pop in case
// a closer hit was found after this entry was pushed.
struct { int32_t idx; float d2; } stack[BVH_MAX_DEPTH * 2 + 4];
int sp = 0;
stack[sp].idx = b->root;
stack[sp].d2  = bvh_point_box_dist2(point, b->store.nodes[b->root].bounds);
sp++;
while (sp > 0) {
        int32_t idx = stack[--sp].idx;
        float   nd2 = stack[sp].d2;
        if (nd2 > best2) continue;   // whole subtree is farther than best

        const bvh_node *n = bvh_node_at_c(&b->store, idx);
        if (n->count > 0) {
            uint32_t first = n->u.first_prim;
            for (uint32_t k = 0; k < n->count; k++) {
                const bvh_prim *p = &b->store.prims[first + k];
                if (tag_mask && !(p->tag & tag_mask)) continue;
                float d2 = bvh_point_box_dist2(point, p->box);
                if (d2 < best2) {
                    best2 = d2; found = 1;
                    best_id = p->id; best_tag = p->tag; best_box = p->box;
                }
            }
        } else {
            int32_t l = idx + 1;
            int32_t r = (int32_t)n->u.second_child;
            float dl = bvh_point_box_dist2(point, b->store.nodes[l].bounds);
            float dr = bvh_point_box_dist2(point, b->store.nodes[r].bounds);
            // push the farther child first so the nearer one pops next and gets
            // a chance to tighten best2 before the far subtree is even touched.
            if (dl < dr) {
                stack[sp].idx = r; stack[sp].d2 = dr; sp++;
                stack[sp].idx = l; stack[sp].d2 = dl; sp++;
            } else {
                stack[sp].idx = l; stack[sp].d2 = dl; sp++;
                stack[sp].idx = r; stack[sp].d2 = dr; sp++;
            }
        }
    }

    if (!found) return -1.0f;
if (out_id)  *out_id  = best_id;
if (out_tag) *out_tag = best_tag;
if (out_box) *out_box = best_box;
return sqrtf(best2);
}
