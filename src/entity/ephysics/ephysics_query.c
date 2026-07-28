#include "ephysics_query.h"
#include "ephysics_aabb.h"
#include "ephysics_broadphase.h"
#include <math.h>
int ephysics_query_solid_cell(world *w, int wx, int wy, int wz) {
    block_id id = world_get_block(w, wx, wy, wz);
    aabb box;
    // reuse the broadphase notion of a collision box so probes never disagree
    // with the real sweep. if there's no box, it's not solid for us.
    return ephysics_block_box(w, wx, wy, wz, id, &box);
}

int ephysics_query_point_solid(world *w, vec3 p) {
    int wx = (int)floorf(p.x);
int wy = (int)floorf(p.y);
int wz = (int)floorf(p.z);
block_id id = world_get_block(w, wx, wy, wz);
aabb box;
if (!ephysics_block_box(w, wx, wy, wz, id, &box)) return 0;
return aabb_contains(box, p);
}

int ephysics_query_box_solid(world *w, aabb box) {
    int x0 = (int)floorf(box.min.x), x1 = (int)floorf(box.max.x);
    int y0 = (int)floorf(box.min.y), y1 = (int)floorf(box.max.y);
    int z0 = (int)floorf(box.min.z), z1 = (int)floorf(box.max.z);

    for (int y = y0; y <= y1; y++)
        for (int z = z0; z <= z1; z++)
            for (int x = x0; x <= x1; x++) {
                block_id id = world_get_block(w, x, y, z);
                aabb bb;
                if (!ephysics_block_box(w, x, y, z, id, &bb)) continue;
                if (ephysics_touching(box, bb, 0.0f)) return 1;
            }
    return 0;
}

float ephysics_query_ground_dist(world *w, const ephys_body *b, float max) {
    aabb box = ephysics_body_box(b);
float feet = box.min.y;
int x0 = (int)floorf(box.min.x), x1 = (int)floorf(box.max.x);
int z0 = (int)floorf(box.min.z), z1 = (int)floorf(box.max.z);
int ystart = (int)floorf(feet - 0.001f);
int ystop  = (int)floorf(feet - max);
for (int y = ystart;
y >= ystop;
y--) {
        float best = -1.0f;
        for (int z = z0; z <= z1; z++)
            for (int x = x0; x <= x1; x++) {
                block_id id = world_get_block(w, x, y, z);
                aabb bb;
                if (!ephysics_block_box(w, x, y, z, id, &bb)) continue;
                if (bb.max.y > best) best = bb.max.y;
            }
        if (best >= 0.0f) {
            float d = feet - best;
            if (d < 0.0f) d = 0.0f;
            return d < max ? d : max;
        }
    }
    return max;
}

int ephysics_query_raycast(world *w, vec3 origin, vec3 dir, float max_dist,
                           int *cx, int *cy, int *cz, vec3 *normal, float *dist) {
    // amanatides & woo. classic grid dda. dir need not be normalized but the
    // distance bookkeeping assumes it is, so make sure of it.
    float len = vec3_length(dir);
    if (len < 1e-6f) return 0;
    dir = vec3_scale(dir, 1.0f / len);

    int x = (int)floorf(origin.x);
    int y = (int)floorf(origin.y);
    int z = (int)floorf(origin.z);

    int sx = dir.x > 0 ? 1 : (dir.x < 0 ? -1 : 0);
    int sy = dir.y > 0 ? 1 : (dir.y < 0 ? -1 : 0);
    int sz = dir.z > 0 ? 1 : (dir.z < 0 ? -1 : 0);

    // distance to cross one cell along each axis
    float dtx = sx != 0 ? fabsf(1.0f / dir.x) : INFINITY;
    float dty = sy != 0 ? fabsf(1.0f / dir.y) : INFINITY;
    float dtz = sz != 0 ? fabsf(1.0f / dir.z) : INFINITY;

    // distance to the first boundary on each axis
    float tx = sx > 0 ? ((x + 1) - origin.x) : (origin.x - x);
    float ty = sy > 0 ? ((y + 1) - origin.y) : (origin.y - y);
    float tz = sz > 0 ? ((z + 1) - origin.z) : (origin.z - z);
    tx = sx != 0 ? tx * dtx : INFINITY;
    ty = sy != 0 ? ty * dty : INFINITY;
    tz = sz != 0 ? tz * dtz : INFINITY;

    float t = 0.0f;
    vec3 n = VEC3_ZERO;

    // check the starting cell first; an origin already inside a block counts.
    if (ephysics_query_solid_cell(w, x, y, z)) {
        if (cx) *cx = x; if (cy) *cy = y; if (cz) *cz = z;
        if (normal) *normal = vec3_neg(dir);
        if (dist) *dist = 0.0f;
        return 1;
    }

    while (t <= max_dist) {
        if (tx < ty && tx < tz) {
            x += sx; t = tx; tx += dtx;
            n = vec3_new((float)-sx, 0, 0);
        } else if (ty < tz) {
            y += sy; t = ty; ty += dty;
            n = vec3_new(0, (float)-sy, 0);
        } else {
            z += sz; t = tz; tz += dtz;
            n = vec3_new(0, 0, (float)-sz);
        }
        if (t > max_dist) break;
        if (ephysics_query_solid_cell(w, x, y, z)) {
            if (cx) *cx = x; if (cy) *cy = y; if (cz) *cz = z;
            if (normal) *normal = n;
            if (dist) *dist = t;
            return 1;
        }
    }
    return 0;
}
