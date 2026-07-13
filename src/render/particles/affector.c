#include "affector.h"

#include <math.h>

void particles_affectors_init(particles_affector_set *set) {
    set->count = 0;
    set->time  = 0.0f;
}

int particles_affectors_add(particles_affector_set *set,
                            const particles_affector *a) {
    if (set->count >= PARTICLES_MAX_AFFECTORS) return -1;
    int i = set->count++;
    set->items[i] = *a;
    set->items[i].enabled = 1;
    return i;
}

particles_affector particles_affector_wind(vec3 dir, float strength) {
    particles_affector a;
    a.kind     = PARTICLES_AFFECTOR_WIND;
    a.enabled  = 1;
    a.vector   = vec3_normalize(dir);
    a.point    = VEC3_ZERO;
    a.strength = strength;
    a.radius   = 0.0f;     // global
    a.falloff  = 1.0f;
    return a;
}

particles_affector particles_affector_point(vec3 center, float strength,
                                            float radius) {
    particles_affector a;
    a.kind     = PARTICLES_AFFECTOR_POINT;
    a.enabled  = 1;
    a.vector   = VEC3_ZERO;
    a.point    = center;
    a.strength = strength;   // negative repels, positive attracts
    a.radius   = radius;
    a.falloff  = 2.0f;
    return a;
}

particles_affector particles_affector_vortex(vec3 center, vec3 axis,
                                             float strength, float radius) {
    particles_affector a;
    a.kind     = PARTICLES_AFFECTOR_VORTEX;
    a.enabled  = 1;
    a.vector   = vec3_normalize(axis);
    a.point    = center;
    a.strength = strength;
    a.radius   = radius;
    a.falloff  = 1.0f;
    return a;
}

// distance falloff weight in [0,1]. radius<=0 means no falloff (global).
static float falloff_weight(const particles_affector *a, float dist) {
    if (a->radius <= 0.0f) return 1.0f;
    if (dist >= a->radius) return 0.0f;
    float x = 1.0f - dist / a->radius;
    return a->falloff >= 2.0f ? x * x : x;
}

// cheap hashy noise off the particle seed + time. not real turbulence, just
// enough to keep smoke from looking like it's on rails.
static float wobble(float seed, float t, float k) {
    return sinf(seed * 53.13f + t * k) * cosf(seed * 17.71f + t * k * 0.7f);
}

vec3 particles_affector_accel(const particles_affector_set *set,
                              const particles_particle *p) {
    vec3 acc = VEC3_ZERO;

    for (int i = 0; i < set->count; i++) {
        const particles_affector *a = &set->items[i];
        if (!a->enabled || a->strength == 0.0f) continue;

        switch (a->kind) {
        case PARTICLES_AFFECTOR_WIND: {
            acc = vec3_add(acc, vec3_scale(a->vector, a->strength));
            break;
        }
        case PARTICLES_AFFECTOR_POINT: {
            vec3 d = vec3_sub(a->point, p->pos);
            float dist = vec3_length(d);
            if (dist < 1e-4f) break;
            float w = falloff_weight(a, dist);
            if (w <= 0.0f) break;
            vec3 dir = vec3_scale(d, 1.0f / dist);
            acc = vec3_add(acc, vec3_scale(dir, a->strength * w));
            break;
        }
        case PARTICLES_AFFECTOR_VORTEX: {
            // tangential force around the axis: cross(axis, radial).
            vec3 rel = vec3_sub(p->pos, a->point);
            // project rel onto the plane perpendicular to the axis
            float along = vec3_dot(rel, a->vector);
            vec3 radial = vec3_sub(rel, vec3_scale(a->vector, along));
            float dist = vec3_length(radial);
            if (dist < 1e-4f) break;
            float w = falloff_weight(a, dist);
            if (w <= 0.0f) break;
            vec3 tang = vec3_normalize(vec3_cross(a->vector, radial));
            acc = vec3_add(acc, vec3_scale(tang, a->strength * w));
            break;
        }
        case PARTICLES_AFFECTOR_TURBULENCE: {
            float t = set->time;
            vec3 j = vec3_new(
                wobble(p->seed + 0.0f, t, 3.0f),
                wobble(p->seed + 1.0f, t, 2.3f),
                wobble(p->seed + 2.0f, t, 2.7f));
            acc = vec3_add(acc, vec3_scale(j, a->strength));
            break;
        }
        default:
            break;
        }
    }

    return acc;
}
