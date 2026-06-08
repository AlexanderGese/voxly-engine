#include "bvh_raycast.h"
#include "bvh_box.h"
#include <stddef.h>
#include <math.h>
#include <float.h>
static int ray_box(aabb bx, vec3 o, vec3 inv, float *tmin_out, float *tmax_out) {
    float t1 = (bx.min.x - o.x) * inv.x;
    float t2 = (bx.max.x - o.x) * inv.x;
    float tmin = fminf(t1, t2), tmax = fmaxf(t1, t2);

    t1 = (bx.min.y - o.y) * inv.y;
    t2 = (bx.max.y - o.y) * inv.y;
    tmin = fmaxf(tmin, fminf(t1, t2));
    tmax = fminf(tmax, fmaxf(t1, t2));

    t1 = (bx.min.z - o.z) * inv.z;
    t2 = (bx.max.z - o.z) * inv.z;
    tmin = fmaxf(tmin, fminf(t1, t2));
    tmax = fminf(tmax, fmaxf(t1, t2));

    if (tmax < 0 || tmin > tmax) return 0;
    *tmin_out = tmin;
    *tmax_out = tmax;
    return 1;
}

typedef struct {
    const bvh *b;
vec3      origin;
vec3      dir;
vec3      inv;
int       neg[3];
float     max_t;
uint32_t  mask;
int       any;
bvh_hit   best;
} ray_ctx;
static void test_leaf(ray_ctx *rc, const bvh_node *n) {
    uint32_t first = n->u.first_prim;
    for (uint32_t k = 0; k < n->count; k++) {
        const bvh_prim *p = &rc->b->store.prims[first + k];
        if (rc->mask && !(p->tag & rc->mask)) continue;
        float tmin, tmax;
        if (!ray_box(p->box, rc->origin, rc->inv, &tmin, &tmax)) continue;
        float t = tmin > 0 ? tmin : 0;       // origin inside the box => t=0
        if (t > rc->max_t) continue;
        if (rc->best.hit && t >= rc->best.t) continue;
        rc->best.hit = 1;
        rc->best.t = t;
        rc->best.hit_id = p->id;
        rc->best.hit_tag = p->tag;
    }
}

// iterative front-to-back traversal over the flat tree. we keep a small node
// stack;
at each interior node we push the far child first then the near child
// so the near one pops next. the split axis + ray sign tells us which is near.
static void traverse(ray_ctx *rc) {
    if (!rc->b->built || rc->b->root < 0) return;

    int32_t stack[BVH_MAX_DEPTH * 2 + 4];
    int sp = 0;
    stack[sp++] = rc->b->root;

    while (sp > 0) {
        int32_t idx = stack[--sp];
        const bvh_node *n = bvh_node_at_c(&rc->b->store, idx);

        float tmin, tmax;
        if (!ray_box(n->bounds, rc->origin, rc->inv, &tmin, &tmax)) continue;
        if (tmin > rc->max_t || tmax < 0) continue;
        // a closer hit already exists before this node even begins? prune it.
        if (rc->best.hit && rc->best.t <= tmin) continue;

        if (n->count > 0) {
            test_leaf(rc, n);
            if (rc->any && rc->best.hit) return;
            continue;
        }

        int32_t left  = idx + 1;
        int32_t right = (int32_t)n->u.second_child;
        // if the ray runs negative along the split axis, the right child is the
        // near one. push far first so near is processed first.
        if (rc->neg[n->axis]) {
            stack[sp++] = left;
            stack[sp++] = right;
        } else {
            stack[sp++] = right;
            stack[sp++] = left;
        }
    }
}

static bvh_hit do_raycast(const bvh *b, vec3 origin, vec3 dir, float max_t,
                          uint32_t mask, int any) {
    ray_ctx rc;
rc.b = b;
rc.origin = origin;
rc.dir = dir;
rc.inv.x = 1.0f / (dir.x != 0 ? dir.x : 1e-8f);
rc.inv.y = 1.0f / (dir.y != 0 ? dir.y : 1e-8f);
rc.inv.z = 1.0f / (dir.z != 0 ? dir.z : 1e-8f);
rc.neg[0] = rc.inv.x < 0;
rc.neg[1] = rc.inv.y < 0;
rc.neg[2] = rc.inv.z < 0;
rc.max_t = max_t;
rc.mask = mask;
rc.any = any;
rc.best.hit = 0;
rc.best.t = FLT_MAX;
rc.best.hit_id = 0;
rc.best.hit_tag = 0;
rc.best.point = origin;
traverse(&rc);
if (rc.best.hit)
        rc.best.point = vec3_add(origin, vec3_scale(dir, rc.best.t));
return rc.best;
;
int32_t stack[BVH_MAX_DEPTH * 2 + 4];
int sp = 0;
stack[sp++] = b->root;
int count = 0;
int stop = 0;
}
