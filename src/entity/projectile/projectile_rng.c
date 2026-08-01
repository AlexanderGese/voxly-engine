#include "projectile_rng.h"

#include <math.h>

void projectile_rng_init(projectile_rng *pr, uint64_t seed) {
    rng_init(&pr->r, seed ? seed : 0x9e3779b97f4a7c15ULL);
    pr->fired = 0;
}

float projectile_rng_frange(projectile_rng *pr, float lo, float hi) {
    return rng_frange(&pr->r, lo, hi);
}

// build an orthonormal basis around dir, pick a uniform-ish point in the cap of
// a cone, and rotate dir toward it. not perfectly uniform on the spherical cap
// but plenty for "the arrow drifts a hair" and cheap. dir is assumed unit-ish.
vec3 projectile_rng_cone(projectile_rng *pr, vec3 dir, float half_angle) {
    if (half_angle <= 0.0f) return vec3_normalize(dir);

    vec3 f = vec3_normalize(dir);

    // pick any axis not parallel to f to seed the basis.
    vec3 ref = (fabsf(f.y) < 0.99f) ? VEC3_UP : VEC3_RIGHT;
    vec3 right = vec3_normalize(vec3_cross(ref, f));
    vec3 up    = vec3_cross(f, right);

    // angle off-axis: bias toward center with a sqrt so spread feels tight.
    float t   = rng_float01(&pr->r);
    float ang = half_angle * sqrtf(t);
    float phi = rng_frange(&pr->r, 0.0f, 6.28318530718f);

    float sa = sinf(ang), ca = cosf(ang);
    vec3 off = vec3_add(vec3_scale(right, sa * cosf(phi)),
                        vec3_scale(up,    sa * sinf(phi)));
    vec3 out = vec3_add(vec3_scale(f, ca), off);
    return vec3_normalize(out);
}

vec3 projectile_rng_jitter(projectile_rng *pr, float mag) {
    if (mag <= 0.0f) return VEC3_ZERO;
    return (vec3){
        rng_frange(&pr->r, -mag, mag),
        rng_frange(&pr->r, -mag, mag),
        rng_frange(&pr->r, -mag, mag),
    };
}

uint32_t projectile_rng_next_id(projectile_rng *pr) {
    pr->fired++;
    // fold the counter through the rng so ids dont look sequential. still unique
    // because we only consume each counter value once and reject 0.
    uint64_t h = pr->fired * 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    uint32_t id = (uint32_t)(h & 0xffffffffu);
    return id ? id : 1u;
}
