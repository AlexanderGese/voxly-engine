#include "building_target.h"
#include "building_face.h"

#include <math.h>

building_target building_target_resolve(world *w, vec3 origin, vec3 dir,
                                        float reach) {
    building_target t = {0};

    // raycast wants a normalized dir; don't trust the caller.
    vec3 d = vec3_normalize(dir);
    ray_hit rh = raycast_blocks(w, origin, d, reach);
    if (!rh.hit) {
        t.valid = 0;
        return t;
    }

    t.valid = 1;
    t.face  = rh.face;
    t.hit_x = rh.x;
    t.hit_y = rh.y;
    t.hit_z = rh.z;

    building_face_adjacent(rh.x, rh.y, rh.z, rh.face,
                           &t.place_x, &t.place_y, &t.place_z);

    // distance to the hit cell center along the ray. cheap, good enough.
    vec3 center = vec3_new(rh.x + 0.5f, rh.y + 0.5f, rh.z + 0.5f);
    t.dist = vec3_distance(origin, center);
    return t;
}

building_target building_target_from_eye(world *w, vec3 eye, vec3 forward,
                                         float reach) {
    return building_target_resolve(w, eye, forward, reach);
}

int building_target_has_block(const building_target *t) {
    return t && t->valid;
}

float building_target_distance(const building_target *t, vec3 eye) {
    if (!t || !t->valid) return 1e30f;
    vec3 center = vec3_new(t->hit_x + 0.5f, t->hit_y + 0.5f, t->hit_z + 0.5f);
    return vec3_distance(eye, center);
}
